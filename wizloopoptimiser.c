/* wizloopoptimiser.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides a series of optimisation functions that allow for the
    reduction and optimisation of wiz loop
-----------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include "symbol.h"
#include "bbst.h"
#include "helper.h"
#include "wizloopoptimiser.h"


/*----------------------------------------------------------------------
    Internal function definitions.  
-----------------------------------------------------------------------*/

Stmts* generate_statements(Exprs *es);
Stmts* reduce_while(While *loop);
Stmts* reduce_if(Cond *cond);
Exprs* find_inactive_exprs(Stmts *ss);

/*----------------------------------------------------------------------
    Internal function.  
-----------------------------------------------------------------------*/

Program* reduce_loops(Program *prog){
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

        //Optimised statments to add in
        Stmts *new_stmts = NULL;
        
         // Switch on kind of statement and print appropriately
        switch(kind) {
            case STMT_COND:
                (new_stmts = reduce_if(&info->cond));
                break;

            case STMT_WHILE:
                (new_stmts = reduce_while(&info->loop));
                break;

            default:
            	break;
        }

        if(new_stmts!=NULL){
            //We need to add these to the loop.
            //Get rid of the simple one
            statments->first = statments->rest->first;
            Stmts *temp = statments->rest->rest;
            statements->rest = new_stmts;
            while(new_stmts!=null){
                new_stmts = new_stmts->rest;
            }
            new_stmts = temp;
        }
        statements = statements->rest;  
    }
}

void reduce_while(While *loop){
    //Declare variables
    Stmts *ss;

    //Now generate a list of symbols that are used within the if statement.
    //We want to find a table of all symbols that exist either in assignment
    //or as parameters for this expression.
    void *active_variables = generate_loop_table(loop);

    //Now we want to go through the statements and return all inactive 
    //expressions, that is, any assignment where the right hand side is not
    //active and the left hand side if not active.
    Exprs *es = find_inactive_functions(active_variables, loop->body);

    //Generate any statements required.
    if(es != NULL){
        Stmts *ss = generate_statements(es);
    }

    return ss; 
}

void reduce_if(Cond *cond){
    //Declare variables
    Stmts *ss;

    //Now generate a list of symbols that are used within the if statement.
    //We want to find a table of all symbols that exist either in assignment
    //or as parameters for this expression.
    void *active_variables = generate_if_table(cond);

    //Now we want to go through the statements and return all inactive 
    //expressions, that is, any assignment where the right hand side is not
    //active and the left hand side if not active.
    Exprs *es = find_inactive_exprs(active_variables, cond->then_branch);
    if(es == NULL){
    	es =  find_inactive_exprs(active_variables, cond->else_branch);
    } else {
    	//Find the end
    	Exprs *temp = es;
    	while(temp != NULL){
    		temp = temp->rest;
    	}
    	temp = find_inactive_exprs(active_variables, cond->else_branch);
    }

    //Generate any statements required.
    if(es != NULL){
        Stmts *ss = generate_statements(es);
    }

    return ss; 
}

Exprs* find_inactive_exprs()

Stmts* generate_statements(Exprs *es){

}

