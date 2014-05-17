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

/*----------------------------------------------------------------------
    Internal structures.  
-----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
    Internal function definitions.  
-----------------------------------------------------------------------*/
void reduce_statements(Stmts *statements);
Expr* reduce_expression(Expr *e);
void reduce_assigment(Assign *a);
void reduce_if(Cond *c);
Expr* reduce_binop(Expr *e);
Expr* reduce_unop(Expr *e);

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
                (info->read = reduce_expression(info->read));
                break;

            case STMT_WHILE:
                (info->loop.cond = reduce_expression(info->loop.cond));
                reduce_statements(info->loop.body);
                break;

            case STMT_WRITE: 
                (info->write = reduce_expression(info->write));
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
            return reduce_unop(e);
            break;
        case EXPR_ARRAY:
            //We need some expression here not assigmnet (stupid c);
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
    //Reduce two sub expressions
    e->e1 = reduce_expression(e->e1);
    e->e2 = reduce_expression(e->e2);           
    //Get the type of the binary operation
    BinOp b = e->binop;
    //Get their kinds
    ExprKind e1k = e->e1->kind;
    ExprKind e2k = e->e2->kind;
    int both_const = (e1k == EXPR_CONST && e2k == EXPR_CONST);
    Expr* new_expr;

    //cover non-boolean cases first
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
        case BINOP_DIV:
            //in this case just reduce if sub-expressions are constant
            if (both_const && e1t == INT_TYPE && e1t == INT_TYPE){
                new_expr = check_malloc(sizeof(Expr));
                new_expr->constant.type = INT_TYPE;
                new_expr->constant.val.int_val = e->e1->constant.val.int_val
                    / e->e2->constant.val.int_val;
                new_expr->kind = EXPR_CONST;
                new_expr->lineno = e->lineno;
                //free unused expressions and return
                free(e->e1);
                free(e->e2);
                free(e);
                return new_expr;
            }
            else{
                return e;
            }
    }

    //now cover boolean cases, only if both sub expressions are constant
    //and of same type
    if (both_const && e1t == e2t){
        int result;
        switch(b){
            //for EQ and NTEQ could have int or bool arguments
            case BINOP_EQ:
                if (e1t == BOOL_TYPE) {
                    result = (e->e1->constant.val.bool_val
                            == e->e2->constant.val.bool_val);
                } else if (e1t == INT_TYPE){
                    result = (e->e1->constant.val.int_val
                            == e->e2->constant.val.int_val);
                } else{
                    //do nothing in error case
                    return e;
                }
                break;
            case BINOP_NTEQ:
                if (e1t == BOOL_TYPE) {
                    result = (e->e1->constant.val.bool_val
                            != e->e2->constant.val.bool_val);
                } else if (e1t == INT_TYPE){
                    result = (e->e1->constant.val.int_val
                            != e->e2->constant.val.int_val);
                } else{
                    //do nothing in error case
                    return e;
                }
                break;
            case BINOP_LT:
                if (e1t == INT_TYPE){
                    result = (e->e1->constant.val.int_val
                            < e->e2->constant.val.int_val);
                } else{
                    //do nothing in error case
                    return e;
                }
                break;
            case BINOP_GT:
                if (e1t == INT_TYPE){
                    result = (e->e1->constant.val.int_val
                            < e->e2->constant.val.int_val);
                } else{
                    //do nothing in error case
                    return e;
                }
                break;
            case BINOP_LTEQ:
                if (e1t == INT_TYPE){
                    result = (e->e1->constant.val.int_val
                            < e->e2->constant.val.int_val);
                } else{
                    //do nothing in error case
                    return e;
                }
                break;
            case BINOP_GTEQ:
                if (e1t == INT_TYPE){
                    result = (e->e1->constant.val.int_val
                            < e->e2->constant.val.int_val);
                } else{
                    //do nothing in error case
                    return e;
                }
                break;
            default:
                //do nothing in error case
                return e;
        }
        //now construct new node to hold boolean result
        new_expr = checked_malloc(sizeof(Expr));
        new_expr->constant.type = BOOL_TYPE;
        new_expr->constant.val.bool_val = result;
        new_expr->kind = EXPR_CONST;
        new_expr->lineno = e->lineno;
    }
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
Expr* reduce_commutative_multiop(e){
    BinOp b = e->binop;
    BinOp std_op, inv_op;
    Exprs* term_list;
    //determine standard and inverse operations for analysis
    if (b == BINOP_ADD || b == BINOP_SUB){
        std_op = BINOP_ADD;
        inv_op = BIONP_SUB;
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
    Expr* reduced_expr;
    Exprs* pos_list = NULL;
    Exprs* pos_list_start;
    Exprs* neg_list = NULL;
    Exprs* neg_list_start;
    UnOp neg_op;
    Expr* const_node = checked_malloc(sizeof(Expr));

    const_node->kind = EXPR_CONST;
    const_node->lineno = e->lineo;
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
                == constant_node->constant.type){
            //this is a constant expr that we can fold
            if (std_op == BINOP_ADD){
                int next = constant.val.int_val;
                const_node->constant.val.int_val += next;
            } else if (std_op == BINOP_MUL){
                int next = constant.val.int_val;
                const_node->constant.val.int_val *= next;
            } else if (std_op == BINOP_OR){
                bool prev, next;
                prev = const_node->constant.val.bool_val;
                next = next_e->constant.val.bool_val;
                const_node->constant.val.bool_val = prev || next;
            } else{
                bool prev, next;
                prev = const_node->constant.val.bool_val;
                next = next_e->constant.val.bool_val;
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
    BinOp combine_binop
    if (std_op == BINOP_ADD){
        neglist_binop = BINOP_ADD;
    } else if (std_op == BINOP_MUL){
        neglist_binop = BINOP_MUL;
    } else if (std_op == BINOP_AND){
        neglist_binop = BINOP_OR;
    } else{
        neglist_binop = BINOP_AND;
    }
    Expr* pos_expr = fold_expression_list(start_pos_list, std_op);
    Expr* neg_expr = fold_expression_list(start_neg_list, neglist_binop);

    //form reduced_expr from pos_expr and neg_expr
    if (pos_expr == NULL){
        reduced_expr = neg_expr;
    } else if (neg_expr == NULL){
        reduced_expr = pos_expr;
    } else{
        //in this case have to decide how to combine them
        //by default, the combining operation is std_op
        BinOp combine_op = std_op;
        if (std_op == BINOP_ADD){
            //for add, just turn node into a SUB node
            combine_op = BINOP_SUB;
        } else if (std_op == BINOP_MUL){
            //for mul, need to explicitely negative the negative terms
            neg_expr = generate_unop_node(UNOP_MINUS, neg_expr, e->lineno);
        } else{
            //for AND and OR, need to negate negative expression
            neg_expr = generate_unop_node(UNOP_NOT, neg_expr, e->lineno);
        }
        //now combine them
        reduced_expr = generate_binop_node(combine_op, pos_expr,
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
        reduced_expr = generate_binop_node(std_op,
                reduced_expr, const_node, e->lineno);
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
    Exprs* e_list;
    if (e->kind == EXPR_BINOP && e->binop == std_op) {
        //linearise LHS sub-expression
        e_list = linearize_expression(e->e1, std_op, inv_op, num_inv);
        //traverse to end of list
        Exprs* tmp_list = e_list;
        while (tmp_list->rest != NULL){
            tmp_list = tmp_list->rest;
        }
        //linearise RHS sub-expression and append
        tmp_list->rest = linearize_expression(e->e2, std_op. inv_op, num_inv);
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
        tmp_list->rest = linearize_expression(e->e2, std_op. inv_op, num_inv+1);
        free(e);
    }
    else {
        //in any other case the expression is a single term, so reduce
        //recursively, then continue linearizing if possible (the reduction
        //might create further scope), or terminate by returning a singleton
        //list node
        Expr* reduced_expr;
        //if odd number of inversions, invert the expression
        if (num_inv % 2 == 1){
            //invert expression (add unary minus node)
            Expr* inv_node = (Expr*) checked_malloc(sizeof(Expr));
            inv_node->kind = EXPR_UNOP;
            inv_node->lineno = e->lineno;
            inv_node->unop = UNOP_MINUS;
            inv_node->e1 = e;
            reduced_expr = reduce_expression(inv_node);
        }
        else{
            reduced_expr = reduce_expression(e);
        }
        //if there is scope to continue linearising reduced expression,
        //then call recursively to continue
        if (reduced_expr->kind == EXPR_BINOP && (reduced_expr->binop == std_op
                    || reduced_expr->binop == inv_op)){
            e_list = linearize_expression(reduced_expr,
                    std_op, inv_op, num_inv);
        }
        else{
            //otherwise we are at base case of the recursive search, so
            //create a list node, and populate
            e_list = (Exprs*) checked_malloc(sizeof(Exprs));
            e_list->first = reduced_expr;
            e_list->rest = NULL;
        }
    }
    return e_list;
}


Expr* reduce_unop(Expr *e){
    UnOp u = e->unop;
    Expr *e1 = e->e1;
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
                return reduce_expression(e1);
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_SUB){
                //reduce -(a-b) to (-a)+b
                e1->binop = BINOP_ADD;
                free(e);
                //place minus around left sub expression
                e1->e1 = generate_unop_node(UNOP_MINUS, e1->e1, e1->lineno);
                return reduce_expression(e1);
            } else if (e1->kind == EXPR_BINOP && e1->binop == BINOP_MUL){
                //reduce -(a*b) to (-a)*(-b)
                free(e);
                //place minus around each sub expression
                e1->e1 = generate_unop_node(UNOP_MINUS, e1->e1, e1->lineno);
                e1->e2 = generate_unop_node(UNOP_MINUS, e1->e2, e1->lineno);
                return reduce_expression(e1);
            }
            //do not do above for divide, as we do not try to re-arrange
            //integer division expressions
            else if (e1->kind == EXPR_UNOP && e1->unop == UNOP_MINUS){
                //double negation, remove
                Expr* e1e1 = e1->e1;
                free(e);
                free(e1);
                return reduce_expression(e1e1);
            }

            //if we fail to find reduction above, reduce sub-expression now
            e1 = reduce_expression(e1);
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
                e1->binop = BIONP_OR;
                free(e);
                //place NOT around sub-expressions
                e1->e1 = generate_unop_node(UNOP_NOT, e1->e1, e1->lineno);
                e1->e2 = generate_unop_node(UNOP_NOT, e1->e2, e1->lineno);
                return reduce_expression(e1);
            } else if (e1->kind == EXPR_BINOP && e1->binop == EXPR_OR){
                //reduces sub expression to an AND (De Morgan's Law)
                e1->binop = BIONP_AND;
                free(e);
                //place NOT around sub-expressions
                Expr* e1e1 = generate_unop_node(UNOP_NOT, e1->e1, e1->lineno);
                e1->e1 = e1e1;
                Expr* e1e2 = generate_unop_node(UNOP_NOT, e1->e2, e1->lineno);
                e1->e2 = e1e2;
                return reduce_expression(e1);
            } else if (e1->kind == EXPR_BINOP && e1->binop == EXPR_GT){
                //reduces sub expression to a LTEQ
                e1->binop = BIONP_LTEQ;
                free(e);
                return reduce_expression(e1);
            } else if (e1->kind == EXPR_BINOP && e1->binop == EXPR_LT){
                //reduces sub expression to a GTEQ
                e1->binop = BIONP_GTEQ;
                free(e);
                return reduce_expression(e1);
            } else if (e1->kind == EXPR_BINOP && e1->binop == EXPR_GTEQ){
                //reduces sub expression to a LT
                e1->binop = BIONP_LT;
                free(e);
                return reduce_expression(e1);
            } else if (e1->kind == EXPR_BINOP && e1->binop == EXPR_LTEQ){
                //reduces sub expression to a GT
                e1->binop = BIONP_GT;
                free(e);
                return reduce_expression(e1);
            } else if (e1->kind == EXPR_UNOP && e1->unop == UNOP_NOT){
                //remove double negative
                Expr* e1e1 = e1->e1;
                free(e);
                free(e1);
                return reduce_expression(e1e1);
            }
            //otherwise reduce e1, and check if it is constant
            Expr *e1 = reduce_expression(e->e1);
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
    return node;
}

