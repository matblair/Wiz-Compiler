/* wizoptimiser.h.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides a series of optimisation functions that allow for the
    reduction and optimisation of wiz syntax trees and opcode trees
-----------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "symbol.h"
#include "balanced_bst.h"
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
	//Get the type of binary operation
    BinOp b = e->binop;
   	//Reduce two sub expressions
    e->e1 = reduce_expression(e->e1);
	e->e2 = reduce_expression(e->e2);           
	
	//Get their kinds, we only need to reduce if constants
	ExprKind e1k = e->e1->kind;
	ExprKind e2k = e->e2->kind;

	//If both sub expressions are constant, then this should be a constant, so
	//we reduce this to constant.
	if(e1k == EXPR_CONST && e2k == EXPR_CONST){
    // Return appropriate type fo each class of function based on the two types
		Expr *new_expr = checked_malloc(sizeof(Expr));
   		new_expr->kind = EXPR_CONST;
		new_expr->lineno = e->lineno;

	    switch(b){
	        case BINOP_OR:
				if(e->e1->constant.type == BOOL_TYPE &&
	        		 e->e2->constant.type == BOOL_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.bool_val ||
	        			 e->e2->constant.val.bool_val);
	        		return new_expr;
	        	}
	            return e;
	        case BINOP_AND:
				if(e->e1->constant.type == BOOL_TYPE &&
	        		 e->e2->constant.type == BOOL_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.bool_val &&
	        			 e->e2->constant.val.bool_val);
	        		return new_expr;
	        	}
	            return e;

	        case BINOP_EQ:
				if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val ==
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){
	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val ==
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			( e->e1->constant.val.int_val ==
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.int_val ==
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	} else if(e->e1->constant.type == BOOL_TYPE &&
	        		 e->e2->constant.type == BOOL_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.bool_val ==
	        			 e->e2->constant.val.bool_val);
	        		return new_expr;
	        	}
	            return e;

	        case BINOP_NTEQ:
				if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val !=
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){
	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val !=
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			( e->e1->constant.val.int_val !=
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.int_val !=
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	} else if(e->e1->constant.type == BOOL_TYPE &&
	        		 e->e2->constant.type == BOOL_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.bool_val !=
	        			 e->e2->constant.val.bool_val);
	        		return new_expr;
	        	}
	            return e;

	        case BINOP_LT:
				if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val <
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){
	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val <
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			( e->e1->constant.val.int_val <
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.int_val <
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	}
	            return e;
	            
	        case BINOP_LTEQ:
				if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val <= 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){
	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val <=
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			( e->e1->constant.val.int_val <=
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.int_val <=
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	}
	            return e;
	            
	        case BINOP_GT:
				if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val > 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){
	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val >
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			( e->e1->constant.val.int_val >
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.int_val >
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	}
	            return e;

	        case BINOP_GTEQ:
	        	if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val >= 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){
	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.float_val >= 
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			( e->e1->constant.val.int_val >=
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = BOOL_TYPE;
	        		new_expr->constant.val.bool_val = 
	        			(e->e1->constant.val.int_val >=
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	}
	            return e;

	        case BINOP_ADD:
	        	if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.float_val + 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){
	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.float_val + 
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			((float) e->e1->constant.val.int_val + 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = INT_TYPE;
	        		new_expr->constant.val.int_val = 
	        			(e->e1->constant.val.int_val + 
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	}
	        	break;

	        case BINOP_MUL:
				if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.float_val * 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.float_val *
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.int_val * 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = INT_TYPE;
	        		new_expr->constant.val.int_val = 
	        			(e->e1->constant.val.int_val * 
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	}
	        	break;

	        case BINOP_SUB:
				if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.float_val - 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.float_val - 
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.int_val - 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = INT_TYPE;
	        		new_expr->constant.val.int_val = 
	        			(e->e1->constant.val.int_val - 
	        			 e->e2->constant.val.int_val);
	        		return new_expr;

	        	}
	        	break;
	        	return e;
	        	break; 

	        case BINOP_DIV:
				if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.float_val / 
	        			 e->e2->constant.val.float_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == FLOAT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are floats, so we can optimise
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			(e->e1->constant.val.float_val /
	        			 (float)e->e2->constant.val.int_val);
	        		return new_expr;

	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == FLOAT_TYPE){

	        		//Then we cast the int to a float and the whole expression 
	        		//is a float
	        		new_expr->constant.type = FLOAT_TYPE;
	        		new_expr->constant.val.float_val = 
	        			((float)e->e1->constant.val.int_val /
	        			 e->e2->constant.val.float_val);
	        		return new_expr;
	        	} else if(e->e1->constant.type == INT_TYPE &&
	        		 e->e2->constant.type == INT_TYPE){

	        		//Then both are ints, so we can optimise
	        		new_expr->constant.type = INT_TYPE;
	        		new_expr->constant.val.int_val = 
	        			(e->e1->constant.val.int_val / 
	        			 e->e2->constant.val.int_val);
	        		return new_expr;
	        	}	
	        	break;           
    	}
	}
	return e;
}

Expr* reduce_unop(Expr *e){
    UnOp u = e->unop;
    Expr *e1 = reduce_expression(e->e1);

    switch(u){
        case UNOP_MINUS:
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
			//We have to change it into a negative value
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
