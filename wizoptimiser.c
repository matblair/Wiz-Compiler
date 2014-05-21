/* wizoptimiser.h.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides a series of optimisation functions that allow for the
    reduction and optimisation of wiz syntax trees and opcode trees
-----------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "symbol.h"
#include "bbst.h"
#include "helper.h"
#include "wizoptimiser.h"

/*----------------------------------------------------------------------
    Internal structures.  
-----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
    Internal function definitions.  
-----------------------------------------------------------------------*/
void reduce_statements(Stmts *statements);

void reduce_assigment(Assign *a);
void reduce_if(Cond *c);
Expr* reduce_binop(Expr *e);
Expr* reduce_commutative_multiop(Expr *e);
Expr* generate_binop_node(BinOp op, Expr* e1, Expr* e2, int lineno);
BOOL is_identity(Expr* e, BinOp op);
Expr* fold_expression_list(Exprs* elist, BinOp op);
Exprs* linearize_expression(Expr* e, BinOp std_op, BinOp inv_op, int num_inv);
Expr* reduce_unop(Expr *e, BOOL recursive);
Expr* generate_unop_node(UnOp op, Expr* e1, int lineno);

/*----------------------------------------------------------------------
FUNCTIONS!!!! cOMMENT THIS LATER
-----------------------------------------------------------------------*/

Program* reduce_ast(Program *prog){
    //We want to go through each expression and remove any constant expressions
    //recursively.
    Program *optimised = prog;
    Procs*procs = optimised->procedures;
    while(procs!=NULL){
        Proc *p = procs->first;
        reduce_statements(p->body->statements);
        procs = procs->rest;
    }

    return optimised;
}

void reduce_statements(Stmts *statements){
    while(statements!=NULL){
        Stmt *statement = statements->first;

        StmtKind kind = statement->kind;
        SInfo *info = &(statement->info);
        
         // Switch on kind of statement and print appropriately
        switch(kind) {
            case STMT_ASSIGN: 
                reduce_assigment(&info->assign);
                break;

            case STMT_COND:
                reduce_if(&info->cond);
                break;

            case STMT_READ:
                info->read = reduce_expression(info->read);
                break;

            case STMT_WHILE:
                info->loop.cond = reduce_expression(info->loop.cond);
                reduce_statements(info->loop.body);
                break;

            case STMT_WRITE: 
                info->write = reduce_expression(info->write);
                break;

            case STMT_FUNC:
                //We need to reduce the statments in the arguments not the id.
                if(info){
                    Exprs *e = info->func->args;
                    while(e != NULL){
                        e->first = reduce_expression(e->first);
                        e = e->rest;
                    }
                }
                break;
        }
        statements = statements->rest;  
    }
}

void reduce_assigment(Assign *a){
    a->asg_ident = reduce_expression(a->asg_ident);
    a->asg_expr = reduce_expression(a->asg_expr);
}

void reduce_if(Cond *c){
    c->cond = reduce_expression(c->cond);
    reduce_statements(c->then_branch);
    reduce_statements(c->else_branch);
}

Expr* reduce_expression(Expr *e){
    //Find the type of an expression
    ExprKind kind = e->kind;
    // Switch on kind to print
    switch(kind) {
        case EXPR_ID:
            return e;
            break;
        case EXPR_CONST:
            return e;
            break;
        case EXPR_BINOP:
            return reduce_binop(e);
            break;
        case EXPR_UNOP:
            //for general expression reduction, want the unop reduction
            //to be fully recursive
            return reduce_unop(e, TRUE);
            break;
        case EXPR_ARRAY:
            //We need some expression here not assignment (stupid c);
            if(e){
                Exprs *es = e->indices;
                while(es != NULL){
                    es->first = reduce_expression(es->first);
                    es = es->rest;
                }

                return e;
                break;
            }
    } 
    return e;
}

Expr* reduce_binop(Expr *e){
    //Get the type of the binary operation
    BinOp b = e->binop;

    //cover commutative cases first
    switch(b){
        case BINOP_OR:
        case BINOP_AND:
        case BINOP_ADD:
        case BINOP_SUB:
        case BINOP_MUL:
            // all of these cases involve commutative operator(s), so advanced
            // constant folding is possible (or in case of sub, can view as
            // commutative since x - y  =  x + (-y)
            return reduce_commutative_multiop(e);
            break;
        default:
            //do nothing for now in default case
            break;
    }

    //now reduce sub expressions recursively for non-commutative cases,
    //and check if they are both constants
    e->e1 = reduce_expression(e->e1);
    e->e2 = reduce_expression(e->e2);           
    //Get their kinds
    ExprKind e1k = e->e1->kind;
    ExprKind e2k = e->e2->kind;
    BOOL both_const = (e1k == EXPR_CONST && e2k == EXPR_CONST);
    //only continue if both sub-expressions are constant and of same type
    if (!(both_const && e->e1->constant.type == e->e2->constant.type)){
        return e;
    }
    Type t = e->e1->constant.type;
    Expr* new_expr;
    Constant new_constant;

    //switch again for the other cases
    switch(b){
        case BINOP_DIV:
            //in this case just reduce if sub-expressions are constant
            if (t == INT_TYPE){
                //don't divide by zero
                if (e->e2->constant.val.int_val == 0) {
                    return e;
                }
                new_constant.type = INT_TYPE;
                new_constant.val.int_val = e->e1->constant.val.int_val
                    / e->e2->constant.val.int_val;
            }
            else{
                //for non-int don't want to reduce
                return e;
            }
            break;
        case BINOP_EQ:
            if (t == BOOL_TYPE) {
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.bool_val
                        == e->e2->constant.val.bool_val);
            } else if (t == INT_TYPE){
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.int_val
                        == e->e2->constant.val.int_val);
            } else{
                //do nothing in error case
                return e;
            }
            break;
        case BINOP_NTEQ:
            if (t == BOOL_TYPE) {
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.bool_val
                        != e->e2->constant.val.bool_val);
            } else if (t == INT_TYPE){
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.int_val
                        != e->e2->constant.val.int_val);
            } else{
                //do nothing in error case
                return e;
            }
            break;
        case BINOP_LT:
            if (t == INT_TYPE){
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.int_val
                        < e->e2->constant.val.int_val);
            } else if (t == FLOAT_TYPE){
                //this is a safe reduction for floats (unambiguous result)
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.float_val
                        < e->e2->constant.val.float_val);
            } else{
                //do nothing in error case
                return e;
            }
            break;
        case BINOP_GT:
            if (t == INT_TYPE){
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.int_val
                        < e->e2->constant.val.int_val);
            } else if (t == FLOAT_TYPE){
                //this is a safe reduction for floats (unambiguous result)
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.float_val
                        < e->e2->constant.val.float_val);
            } else{
                //do nothing in error case
                return e;
            }
            break;
        case BINOP_LTEQ:
            if (t == INT_TYPE){
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.int_val
                        < e->e2->constant.val.int_val);
            } else if (t == FLOAT_TYPE){
                //this is a safe reduction for floats (unambiguous result)
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.float_val
                        < e->e2->constant.val.float_val);
            } else{
                //do nothing in error case
                return e;
            }
            break;
        case BINOP_GTEQ:
            if (t == INT_TYPE){
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.int_val
                        < e->e2->constant.val.int_val);
            } else if (t == FLOAT_TYPE){
                //this is a safe reduction for floats (unambiguous result)
                new_constant.type = BOOL_TYPE;
                new_constant.val.bool_val = (e->e1->constant.val.float_val
                        < e->e2->constant.val.float_val);
            } else{
                //do nothing in error case
                return e;
            }
            break;
        default:
            //do nothing in error case
            return e;
    }
    //now construct new node to hold constant result
    new_expr = checked_malloc(sizeof(Expr));
    new_expr->constant = new_constant;
    new_expr->kind = EXPR_CONST;
    new_expr->lineno = e->lineno;

    //free the sub-expressions and previous expression, and return new
    free(e->e1);
    free(e->e2);
    free(e); 
    return new_expr;
}


/*-----------------------------------------------------------------------
    reduces a multi-op expression involving some commutative operator
    (in case of - expressions view these as +)
-----------------------------------------------------------------------*/
Expr* reduce_commutative_multiop(Expr* e){
    BinOp b = e->binop;
    BinOp std_op, inv_op;
    Exprs* term_list;
    //determine standard and inverse operations for analysis
    if (b == BINOP_ADD || b == BINOP_SUB){
        std_op = BINOP_ADD;
        inv_op = BINOP_SUB;
    }
    else{
        std_op = b;
        inv_op = -1;    //in any other case there is no inverse operation
    }
    //linearize expression, converting to list of terms at same precedence
    //level
    term_list = linearize_expression(e, std_op, inv_op, 0);

    //now scan through the list, folding constant terms onto the RHS
    //expression, and appending others to the left
    //set up expressions needed for scanning
    Exprs* pos_list = NULL;
    Exprs* pos_list_start = NULL;
    Exprs* neg_list = NULL;
    Exprs* neg_list_start = NULL;
    UnOp neg_op;
    Expr* const_node = checked_malloc(sizeof(Expr));
    const_node->inferred_type = -1;

    const_node->kind = EXPR_CONST;
    const_node->lineno = e->lineno;
    //obtain identity constant node, and the negative operator
    if (std_op == BINOP_ADD){
        const_node->constant.type = INT_TYPE;
        const_node->constant.val.int_val = 0; //additive identity
        neg_op = UNOP_MINUS;
    } else if (b == BINOP_MUL){
        const_node->constant.type = INT_TYPE;
        const_node->constant.val.int_val = 1; //mult identity
        neg_op = UNOP_MINUS;
    } else if (b == BINOP_OR){
        const_node->constant.type = BOOL_TYPE;
        const_node->constant.val.bool_val = FALSE; //or identity
        neg_op = UNOP_NOT;
    } else if (b == BINOP_AND){
        const_node->constant.type = BOOL_TYPE;
        const_node->constant.val.bool_val = TRUE; //and identity
        neg_op = UNOP_NOT;
    } else{
        //should never occur
        return e;
    }

    //now perform the scan
    while (term_list != NULL){
        Expr* next_e = term_list->first;
        Exprs* old;
        //only fold expression if it is constant, and of correct type
        if(next_e->kind == EXPR_CONST && next_e->constant.type
                == const_node->constant.type){
            //this is a constant expr that we can fold
            Constant c = next_e->constant;
            if (std_op == BINOP_ADD){
                int next = c.val.int_val;
                const_node->constant.val.int_val += next;
            } else if (std_op == BINOP_MUL){
                int next = c.val.int_val;
                const_node->constant.val.int_val *= next;
            } else if (std_op == BINOP_OR){
                BOOL prev, next;
                prev = const_node->constant.val.bool_val;
                next = c.val.bool_val;
                const_node->constant.val.bool_val = prev || next;
            } else{
                BOOL prev, next;
                prev = const_node->constant.val.bool_val;
                next = c.val.bool_val;
                const_node->constant.val.bool_val = prev && next;
            }
            //advance term_list
            old = term_list;
            term_list = term_list->rest;
            free(old);
        }
        else{
            //otherwise we can't reduce this expression, so append it to
            //either positive or negative list 
            //check if it is a negative operand first
                if (next_e->kind == EXPR_UNOP && next_e->unop == neg_op){
                    //append list node to the negative list
                    if (neg_list == NULL){
                        //if this is first node in neg list set neg_list_start
                        neg_list = term_list;
                        neg_list_start = term_list;
                    } else{
                        neg_list->rest = term_list;
                        neg_list = neg_list->rest;
                    }
                    //advance term_list, and cut off pointer from end of neg_list
                    term_list = term_list->rest;
                    neg_list->rest = NULL;
                    //prune the negative unary node from the neg_list entry
                    neg_list->first = next_e->e1;
                    free(next_e);
                }
                else{
                    //otherwise append list node to the positive list
                    if (pos_list == NULL){
                        //if this is first node in pos list set pos_list_start
                        pos_list = term_list;
                        pos_list_start = term_list;
                    } else{
                        pos_list->rest = term_list;
                        pos_list = pos_list->rest;
                    }
                    //advance term_list, and cut off pointer from end of pos_list
                    term_list = term_list->rest;
                    pos_list->rest = NULL;
                }
            }
        }

        //now iterate through positive and negative lists recursively to create
        //positive and negative expressions, then join these along with the
        //constant expression

        //work out which binary operator we want to use in negative list
        //(for ADD and MUL this is regular binop, for AND/OR have to swap,
        //according to De Morgan's Law)
        BinOp neglist_binop;
        BinOp combine_binop;
        if (std_op == BINOP_ADD){
            neglist_binop = BINOP_ADD;
        } else if (std_op == BINOP_MUL){
            neglist_binop = BINOP_MUL;
        } else if (std_op == BINOP_AND){
        neglist_binop = BINOP_OR;
    } else{
        neglist_binop = BINOP_AND;
    }
    Expr* pos_expr = fold_expression_list(pos_list_start, std_op);
    Expr* neg_expr = fold_expression_list(neg_list_start, neglist_binop);

    //form reduced_expr from pos_expr and neg_expr
    Expr* reduced_expr = NULL;
    if (pos_expr == NULL && neg_expr != NULL){
        //make sure to enclose the neg_expr in negative node
        reduced_expr = generate_unop_node(neg_op, neg_expr, e->lineno);
    } else if (neg_expr == NULL && pos_expr != NULL){
        reduced_expr = pos_expr;
    } else if (pos_expr != NULL && neg_expr != NULL){
        //in this case have to decide how to combine them
        //by default, the combining operation is std_op
        if (std_op == BINOP_ADD){
            //for add, just turn node into a SUB node
            combine_binop = BINOP_SUB;
        } else{
            //for others, need to explicitly negate the negative terms
            combine_binop = std_op;
            neg_expr = generate_unop_node(neg_op, neg_expr, e->lineno);
        }
        //now combine them
        reduced_expr = generate_binop_node(combine_binop, pos_expr,
                neg_expr, e->lineno);
    }

    //remove possible redundency in result
    if (reduced_expr == NULL){
        //this means the constant node is the entire expression
        reduced_expr = const_node;
    }
    //otherwise check if constant node is identity, with respect to operation
    else if (!is_identity(const_node, std_op)){
        //if not identity need to combine with our reduced expr from
        //positive and negative lists

        //check for case where reduced_expr is negated and op is ADD (can
        //optimize and change to SUB)
        if (reduced_expr->kind == EXPR_UNOP
                && reduced_expr->unop == UNOP_MINUS && std_op == BINOP_ADD){
            Expr* tmp = reduced_expr;
            reduced_expr = generate_binop_node(BINOP_SUB, const_node,
                    reduced_expr->e1, e->lineno);
            //free the now redundant unop node
            free(tmp);
        }
        else{
            reduced_expr = generate_binop_node(std_op,
                    reduced_expr, const_node, e->lineno);
        }
    }

    return reduced_expr;
}


/*-----------------------------------------------------------------------
    creates a new binary op expression, with operation op, sub-expressions
    e1 and e2, and given lineno
    - returns pointer to this new expression
-----------------------------------------------------------------------*/
Expr* generate_binop_node(BinOp op, Expr* e1, Expr* e2, int lineno){
    Expr* node = (Expr*) checked_malloc(sizeof(Expr));
    node->lineno = lineno;
    node->kind = EXPR_BINOP;
    node->binop = op;
    node->e1 = e1;
    node->e2 = e2;
    //make sure node does not initially match any type
    node->inferred_type = -1;
    return node;
}

/*-----------------------------------------------------------------------
    decides whether the provided expression is identity with respect
    to the given boolean operator (works for ADD, MUL, OR and AND)
-----------------------------------------------------------------------*/
BOOL is_identity(Expr* e, BinOp op){
    if (e->kind == EXPR_CONST){
        //for ADD, require int type with value 0
        if (op == BINOP_ADD && e->constant.type == INT_TYPE
                && e->constant.val.int_val == 0){
            return TRUE;
        }
        //for MUL, require int type with value 1
        else if (op == BINOP_MUL && e->constant.type == INT_TYPE
                && e->constant.val.int_val == 1){
            return TRUE;
        }
        //for OR, require bool type with value FALSE
        else if (op == BINOP_OR && e->constant.type == BOOL_TYPE
                && e->constant.val.bool_val == FALSE){
            return TRUE;
        }
        //for AND, require bool type with value TRUE 
        else if (op == BINOP_AND && e->constant.type == BOOL_TYPE
                && e->constant.val.bool_val == TRUE){
            return TRUE;
        }
        //in any other case, constant is not identity
        else{
            return FALSE;
        }
    }
    else{
        //if expression not constant, it is not in general identity
        return FALSE;
    }
}


/*-----------------------------------------------------------------------
    folds the expression list given by elist, using the commutative
    operator op, into a left-heavy binary expression tree 
-----------------------------------------------------------------------*/
Expr* fold_expression_list(Exprs* elist, BinOp op){
    //trivial cases
    if (elist == NULL){
        return NULL;
    } else if (elist->rest == NULL){
        return elist->first;
    }
    //main, recursive case
    else{
        //call recursively on rest of list, and put the recursively
        //built expression on left (left growing tree)
        Expr* e_left = fold_expression_list(elist->rest, op);
        return generate_binop_node(op, e_left, elist->first, e_left->lineno);
    }
}


/*-----------------------------------------------------------------------
    linearizes the expression, converting it to list of operands of the
    given commutative operator (std_op)
    in the case of addition, we also take into account inversions from
    the subtaction operator (expressions that are inverted an odd number
    of times have unary minus node attached)
    also performs recursive reductions of the individual terms
-----------------------------------------------------------------------*/
Exprs* linearize_expression(Expr* e, BinOp std_op, BinOp inv_op, int num_inv){
    Exprs* e_list = NULL;
    if (e->kind == EXPR_BINOP && e->binop == std_op) {
        //linearise LHS sub-expression
        e_list = linearize_expression(e->e1, std_op, inv_op, num_inv);
        //traverse to end of list
        Exprs* tmp_list = e_list;
        while (tmp_list->rest != NULL){
            tmp_list = tmp_list->rest;
        }
        //linearise RHS sub-expression and append
        tmp_list->rest = linearize_expression(e->e2, std_op, inv_op, num_inv);
        //free memory that was used for holding this (now redundant) node
        free(e);
    }
    else if (e->kind == EXPR_BINOP && e->binop == inv_op) {
        //same as previous case, but number of inversions is increased by
        //one for RHS sub-expression
        e_list = linearize_expression(e->e1, std_op, inv_op, num_inv);
        Exprs* tmp_list = e_list;
        while (tmp_list->rest != NULL){
            tmp_list = tmp_list->rest;
        }
        tmp_list->rest = linearize_expression(e->e2, std_op, inv_op, num_inv+1);
        free(e);
    }
    else {
        //in any other case the expression is a single term, so reduce
        //and continue linearizing if possible (the reduction
        //might create further scope), or terminate by returning a singleton
        //list node and reducing recursively

        //first, invert the expression if necessary
        Expr* e1;
        //if odd number of inversions, invert the expression
        if (num_inv % 2 == 1){
            //invert expression (add unary minus node)
            Expr* inv_node = (Expr*) checked_malloc(sizeof(Expr));
            inv_node->kind = EXPR_UNOP;
            inv_node->lineno = e->lineno;
            inv_node->unop = UNOP_MINUS;
            inv_node->e1 = e;
            e1 = inv_node;
        }
        else{
            e1 = e;
        }
        //if e1 is unary, there could be scope for further
        //linearization, so try a non-recursive reduction
        if (e1->kind == EXPR_UNOP){
            e1 = reduce_unop(e1, FALSE);
            //if there is scope to continue linearising reduced expression,
            //then call recursively to continue
            if (e1->kind == EXPR_BINOP && (e1->binop == std_op
                        || e1->binop == inv_op)){
                e_list = linearize_expression(e1, std_op, inv_op, num_inv);
            }
        }
        if (e_list == NULL){
            //otherwise we are at base case of the recursive search, so
            //create a list node, and populate, after performing
            //a fully recursive reduction
            e_list = (Exprs*) checked_malloc(sizeof(Exprs));
            e_list->first = reduce_expression(e1);
            e_list->rest = NULL;
        }
    }
    return e_list;
}


Expr* reduce_unop(Expr *e, BOOL recursive){
    UnOp u = e->unop;
    Expr *e1 = e->e1;
    Expr *e_shallow_reduced = NULL;
    switch(u){
        case UNOP_MINUS:
            //before we reduce sub-expression recursively, check for
            //numeric expressions that we can reduce by algebra
            if (e1->kind == EXPR_BINOP && e1->binop == BINOP_ADD){
                //reduce -(a+b) to (-a)-b
                e1->binop = BINOP_SUB;
                free(e);
                //place minus around left sub expression
                e1->e1 = generate_unop_node(UNOP_MINUS, e1->e1, e1->lineno);
                e_shallow_reduced = e1;
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_SUB){
                //reduce -(a-b) to (-a)+b
                e1->binop = BINOP_ADD;
                free(e);
                //place minus around left sub expression
                e1->e1 = generate_unop_node(UNOP_MINUS, e1->e1, e1->lineno);
                e_shallow_reduced = e1;
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_MUL){
                //reduce -(a*b) to (-a)*(-b)
                free(e);
                //place minus around each sub expression
                e1->e1 = generate_unop_node(UNOP_MINUS, e1->e1, e1->lineno);
                e1->e2 = generate_unop_node(UNOP_MINUS, e1->e2, e1->lineno);
                e_shallow_reduced = e1;
            }
            //do not do above for divide, as we do not try to re-arrange
            //integer division expressions
            else if (e1->kind == EXPR_UNOP && e1->unop == UNOP_MINUS){
                //double negation, remove
                Expr* e1e1 = e1->e1;
                free(e);
                free(e1);
                //greedily continue reducing extra unary nodes with same
                //recursivity
                if (e1e1->kind == EXPR_UNOP && e1e1->unop == UNOP_MINUS){
                    return reduce_unop(e1e1, recursive);
                }
                else{
                    e_shallow_reduced = e1e1;
                }
            }

            //decide whether we reduce the shallow-reduced expression (if
            //generated), based on whether unop reduction is recursive
            if (e_shallow_reduced != NULL){
                if (recursive){
                    return reduce_expression(e_shallow_reduced);
                } else{
                    return e_shallow_reduced;
                }
            }

            //if we fail to find reduction above, reduce sub-expression now
            if (recursive){
                e1 = reduce_expression(e1);
            } 
            //We have to change it into a negative value
            if(e1->kind == EXPR_CONST && !(e1->constant.type == BOOL_TYPE)){
                Expr *new_expr = checked_malloc(sizeof(Expr));
                new_expr->kind = EXPR_CONST;
                new_expr->lineno = e->lineno;
                Constant *c = &e1->constant;

                if(c->type == FLOAT_TYPE){
                    //Then return new constant.
                    new_expr->constant.type = FLOAT_TYPE;
                    new_expr->constant.val.float_val = -(c->val.float_val);
                } else if (c->type == INT_TYPE){
                    //Then return new constant.
                    new_expr->constant.type = INT_TYPE;
                    new_expr->constant.val.int_val = -(c->val.int_val);
                }

                free(e);
                return new_expr;
            }
            return e;

        case UNOP_NOT:
            //before we reduce sub-expression recursively, check for boolean
            //expression that we can reduce logically
            if (e1->kind == EXPR_BINOP && e1->binop == BINOP_AND){
                //reduces sub expression to an OR (De Morgan's Law)
                e1->binop = BINOP_OR;
                free(e);
                //place NOT around sub-expressions
                e1->e1 = generate_unop_node(UNOP_NOT, e1->e1, e1->lineno);
                e1->e2 = generate_unop_node(UNOP_NOT, e1->e2, e1->lineno);
                e_shallow_reduced = e1;
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_OR){
                //reduces sub expression to an AND (De Morgan's Law)
                e1->binop = BINOP_AND;
                free(e);
                //place NOT around sub-expressions
                Expr* e1e1 = generate_unop_node(UNOP_NOT, e1->e1, e1->lineno);
                e1->e1 = e1e1;
                Expr* e1e2 = generate_unop_node(UNOP_NOT, e1->e2, e1->lineno);
                e1->e2 = e1e2;
                e_shallow_reduced = e1;
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_GT){
                //reduces sub expression to a LTEQ
                e1->binop = BINOP_LTEQ;
                free(e);
                e_shallow_reduced = e1;
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_LT){
                //reduces sub expression to a GTEQ
                e1->binop = BINOP_GTEQ;
                free(e);
                e_shallow_reduced = e1;
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_GTEQ){
                //reduces sub expression to a LT
                e1->binop = BINOP_LT;
                free(e);
                e_shallow_reduced = e1;
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_LTEQ){
                //reduces sub expression to a GT
                e1->binop = BINOP_GT;
                free(e);
                e_shallow_reduced = e1;
            } else if (e1->kind == EXPR_UNOP && e1->unop == UNOP_NOT){
                //remove double negative, and continue if any remain
                Expr* e1e1 = e1->e1;
                free(e);
                free(e1);
                //greedily continue reducing extra unary nodes with same
                //recursivity
                if (e1e1->kind == EXPR_UNOP && e1e1->unop == UNOP_NOT){
                    return reduce_unop(e1e1, recursive);
                }
                else{
                    e_shallow_reduced = e1e1;
                }
            }

            //decide whether we reduce the shallow-reduced expression (if
            //generated), based on whether unop reduction is recursive
            if (e_shallow_reduced != NULL){
                if (recursive){
                    return reduce_expression(e_shallow_reduced);
                } else{
                    return e_shallow_reduced;
                }
            }

            //otherwise reduce e1, and check if it is constant
            if (recursive){
                e1 = reduce_expression(e1);
            }
            if(e1->kind == EXPR_CONST && e1->constant.type == BOOL_TYPE){
                Expr *new_expr = checked_malloc(sizeof(Expr));
                new_expr->kind = EXPR_CONST;
                new_expr->lineno = e->lineno;
                Constant *c = &e1->constant;

                //Then return new constant.
                new_expr->constant.type = BOOL_TYPE;
                new_expr->constant.val.bool_val = !(c->val.bool_val);
                free(e);
                return new_expr;
            }
            return e;
    }
    return e;
}

Expr* generate_unop_node(UnOp op, Expr* e1, int lineno){
    Expr* node = (Expr*) checked_malloc(sizeof(Expr));
    node->kind = EXPR_UNOP;
    node->unop = op;
    node->e1 = e1;
    node->lineno = lineno;
    //make sure node does not automatically match any type
    node->inferred_type = -1;
    return node;
}

