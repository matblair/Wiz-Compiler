/* array_access.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME

    implements functions for constructing ArrayAccess objects, which
    mediate safe and efficient array access
    semantic analysis should already have been performed, so that
    aray dimension matches number of index expressions, etc.
-----------------------------------------------------------------------*/

#include "helper.h"
#include "wizoptimiser.h"
#include "ast.h"
#include "symbol.h"
#include "array_access.h"


Intervals *create_dbounds_node(int lower, int upper, int offset_coefficient);
Exprs *create_doffsets_node(Expr *index_e, int offset_coefficient);

ArrayAccess *get_array_access(Expr *expr, Bounds *array_bounds) {
    //get array index expressions
    Exprs *index_list = expr->indices;

    //first traverse through the bounds list once to get total
    //size, which will assist in linear scan
    Bounds *tmp_b = array_bounds;
    int size = 1;
    while (tmp_b != NULL) {
        size *= (tmp_b->first->upper - tmp_b->first->lower + 1);
        tmp_b = tmp_b->rest;
    }

    //now scan through again, appending each index to either the dynamic
    //lists or joining to static_offset / is_in_static_bounds
    //recall, the flat array index is given by
    //
    // (high_n - low_n + 1) * ... * (high_2 - low_2 + 1) * (i_1 - low_1)
    // + ...
    // + (high_n - low_n + 1) * (high_n-1 - low_n-1 + 1) * (i_n-2 - low_n-2)
    // + (high_n - low_n + 1) * (i_n-1 - low_n-1)
    // + (i_n - low_n)

    Exprs *dynamic_offsets = NULL;
    Exprs *tmp_e;
    Intervals *dynamic_bounds = NULL;
    Intervals *tmp_i;
    int static_offset = 0;
    BOOL is_in_static_bounds = TRUE;

    int offset_coefficient = size;
    while (index_list != NULL) {
        Expr *next_index = index_list->first;
        //update the offset coefficient
        int upper = array_bounds->first->upper;
        int lower = array_bounds->first->lower;
        offset_coefficient /= (upper - lower + 1);

        //static expression case
        if (next_index->kind == EXPR_CONST) {
            //if constant expression, add to the static_offset
            int index_val = next_index->constant.val.int_val;
            static_offset += (offset_coefficient * index_val); 
            if (index_val < lower || index_val > upper) {
                //in this case we have failed static bounds check
                is_in_static_bounds = FALSE;
            }
        }

        //dynamic expression case
        else {
            //calculate bounds for the dynamic offset (multiply bounds
            //by the offset_coefficient)
            Intervals *dynamic_bounds_node = create_dbounds_node(lower,
                    upper, offset_coefficient);
            //append node at end of list
            if (dynamic_bounds == NULL) {
                //this means it is first node of the list
                dynamic_bounds = dynamic_bounds_node;
                tmp_i = dynamic_bounds; //pointer we keep to end of list for
                                        //appending
            } else {
                tmp_i->rest = dynamic_bounds_node;
                tmp_i = tmp_i->rest;
            }

            //calculate and reduce the expression for calculating the
            //dynamic offset
            Exprs *dynamic_offsets_node = create_doffsets_node(next_index,
                    offset_coefficient);
            //append node at end of list
            if (dynamic_offsets == NULL) {
                //this means it is first node of list
                dynamic_offsets = dynamic_offsets_node;
                tmp_e = dynamic_offsets;
            } else {
                tmp_e->rest = dynamic_offsets_node;
                tmp_e = tmp_e->rest;
            }
        }
        //advance the list pointers to iterate through list
        array_bounds = array_bounds->rest;
        index_list = index_list->rest;
    }

    //finally put the pieces together and return the ArrayAccess pointer
    ArrayAccess *array_access
        = (ArrayAccess*) checked_malloc(sizeof(ArrayAccess));
    array_access->static_offset = static_offset;
    array_access->is_in_static_bounds = is_in_static_bounds;
    array_access->dynamic_offsets = dynamic_offsets;
    array_access->dynamic_bounds = dynamic_bounds;
    return array_access;
}
                    


    
/*-----------------------------------------------------------------------
    creates a new node for the dynamic bounds list, given the argument
    lower and upper bounds, and the given offset coefficient
    returns pointer to node
-----------------------------------------------------------------------*/
Intervals *create_dbounds_node(int lower, int upper, int offset_coefficient) {
    Interval *offset_bounds
        = (Interval*) checked_malloc(sizeof(Interval));
    Intervals *dynamic_bounds_node 
        = (Intervals*) checked_malloc(sizeof(Intervals));
    offset_bounds->lower = offset_coefficient * lower;
    offset_bounds->upper = offset_coefficient * upper;
    dynamic_bounds_node->first = offset_bounds;
    dynamic_bounds_node->rest = NULL;
    return dynamic_bounds_node;
}


/*-----------------------------------------------------------------------
    creates a new node for the dynamic offsets list, given the argument
    index expression, and offset_coefficient, and returns a pointer
    to this node
-----------------------------------------------------------------------*/
Exprs *create_doffsets_node(Expr *index_e, int offset_coefficient) {
    Expr *dynamic_offset_expr = (Expr*) checked_malloc(sizeof(Expr));
    Exprs *dynamic_offset_node = (Exprs*) checked_malloc(sizeof(Exprs));

    //create a BINOP expression, multiplying by the offset
    dynamic_offset_expr->kind = EXPR_BINOP;
    dynamic_offset_expr->lineno = index_e->lineno;
    dynamic_offset_expr->binop = BINOP_MUL;
    //link the index expression for e1
    dynamic_offset_expr->e1 = index_e;
    //create a constant node for e2
    dynamic_offset_expr->e2 = (Expr*) checked_malloc(sizeof(Expr));
    dynamic_offset_expr->e2->kind = EXPR_CONST;
    dynamic_offset_expr->e2->lineno = index_e->lineno;
    dynamic_offset_expr->e2->constant.type = INT_TYPE;
    dynamic_offset_expr->e2->constant.val.int_val = offset_coefficient;

    //finally attempt to reduce the expression we have created, and link
    //from the new node
    dynamic_offset_node->first = reduce_expression(dynamic_offset_expr);
    dynamic_offset_node->rest = NULL;
    return dynamic_offset_node;
}
