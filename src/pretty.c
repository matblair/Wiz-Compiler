/* pretty.c */

/*-----------------------------------------------------------------------
    A stub for a pretty-printer for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "pretty.h"

extern void report_error_and_exit(const char *msg);

int indent_level;

void
pretty_prog(FILE *fp, Program *prog) {
    indent_level=0;
    print_procedures(fp, prog->procedures);
}

int proc_comparison(const void *a, const void *b){
    //Cast to procs
    Proc *proc_a = *((Proc **) a);
    Proc *proc_b = *((Proc **) b);
    char *id_a = proc_a->header->id;
    char *id_b = proc_b->header->id;
    //Sort by string name
    return (strcmp(id_a, id_b));
}


void print_procedures(FILE *fp, Procs *procs){

    //Go through and count the array
    int num_procs=0;
    Procs *rest = procs;
    while(rest!=NULL){
        num_procs++;
        rest = rest->rest;
    }

    //Create the array
    Proc **proc_ptrs = (Proc **) malloc(num_procs * sizeof(Proc *));
    Procs *c = procs;
    int i=0;
    for(i=0; i<num_procs; i++){
        proc_ptrs[i] = c->first;
        c = c->rest; 
    }

    // Sort the array 
    qsort(proc_ptrs, num_procs, sizeof(Proc *), proc_comparison);

    // Go through the array and print
    for(i=0; i<num_procs; i++){
        //Get the current process
        Proc *current = proc_ptrs[i];

        //Print the header
        print_header(fp, current->header);
        //Print the body
        print_body(fp, current->body);
        //Print a new line
        fprintf(fp, "end\n\n");
    }

    free(proc_ptrs);
}

void print_header(FILE *fp, Header *header){
    // Print opening line
    fprintf(fp, "proc %s",header->id);
    //Print parameters 
    if(header->params!=NULL){
        fprintf(fp, " ("); // White space before params
        print_params(fp, header->params);
    } else {
        fprintf(fp, " ()\n");
    }
}

void print_params(FILE *fp, Params *params){
    //Print the current param
    Param *current = params->first;

    //Check the type of value and print appropriately
    ParamsInd ind = current->ind;
    Type type = current->type;
    char *id = current->id;
    fprintf(fp, "%s %s %s", valnames[ind], typenames[type], id);

    if(params->rest != NULL){
        //Add the comma and whitespace, then continue printing
        fprintf(fp, ", ");
        print_params(fp, params->rest);
    } else {
        // Close the parenthesis
        fprintf(fp,")\n");
    }
}

void print_body(FILE *fp, Body *body){

    //Indent one level in from the main function call
    indent_level++;
    //Print declarations and statements with their respective functions
    print_declarations(fp, body->decls);
    
    //Add the one line break required
    fprintf(fp,"\n");
    print_statements(fp, body->statements);
    indent_level--;
}


void print_declarations(FILE *fp, Decls *declarations) {
    //Check non null conditions
    if (declarations == NULL) {
        return;
    }
    //Print the current declaration       
    Decl *decl = declarations->first;
    print_indents(fp);

    //Check if array declaration or standard variable declaration
    if(decl->array!=NULL){
        fprintf(fp, "%s %s[", typenames[decl->type], decl->id);
        //Print expression list
        print_array_decl(fp, decl->array);
        fprintf(fp, "];\n");

    } else {
        fprintf(fp, "%s %s;\n", typenames[decl->type], decl->id);
    }

    // Continue along our weird data structure
    print_declarations(fp, declarations->rest);
}

void print_array_decl(FILE *fp, Intervals *intervals) {
    Interval *i = intervals->first;
    fprintf(fp,"%d..%d", i->lower, i->upper);
    
    //Check we need to print the next one
    if(intervals->rest != NULL){
        fprintf(fp,", ");
        print_array_decl(fp, intervals->rest);
    } 
}

void print_statements(FILE *fp, Stmts *statements) {
    if (statements == NULL) {
        return;
    }
    // Assign current value
    Stmt *statement = statements->first;
    print_indents(fp);

    // Print the current statement
    print_statement(fp, statement);

    //Continue along the datastructure
    print_statements(fp, statements->rest);
}

void print_statement(FILE *fp, Stmt *statement) {
    //Find the type of statement and call the appropriate function.
    StmtKind kind = statement->kind;
    SInfo *info = &(statement->info);
    //Switch on kind of statement and print appropriately
    switch(kind) {
        case STMT_ASSIGN : 
            
            fprintf(fp,"%s := ", info->assign.asg_id);
            Expr *expression = info->assign.asg_expr;
            print_expression(fp, expression);
            fprintf(fp,";\n");
            break;

        case STMT_COND:
            
            // Print if branch
            fprintf(fp,"if ");
            Expr *if_expr = info->cond.cond;
            indent_level++;
            print_expression(fp, if_expr);
            indent_level--;
           
            //Print then branch
            fprintf(fp," then\n");
            Stmts *then_stmt = info->cond.then_branch;
            indent_level++;
            print_statements(fp, then_stmt);
            indent_level--;

            //Possibly print else brach
            if(info->cond.else_branch != NULL){
                print_indents(fp);
                fprintf(fp,"else\n");
                Stmts *else_stmt = info->cond.else_branch;
                indent_level++;
                print_statements(fp, else_stmt);
                indent_level--;
            }

            //Print the fi
            print_indents(fp);
            fprintf(fp, "fi\n");
            break;

        case STMT_READ:
            
            fprintf(fp,"read %s;\n", info->read);
            break;

        case STMT_WHILE:
            
            //Print the while
            fprintf(fp,"while ");
            //Print the expression
            Expr *while_cond = info->loop.cond;
            print_expression(fp, while_cond);
            //Print the do
            fprintf(fp, " do\n");
            //Print the statements
            indent_level++;
            print_statements(fp, info->loop.body);
            indent_level--;

            //Print the closing do
            print_indents(fp);
            fprintf(fp, "od\n");
            break;

        case STMT_WRITE:
            
            //Write the command
            fprintf(fp,"write ");
            //Print the expression 
            print_expression(fp, info->write);
            fprintf(fp,";\n");
            break;

        case STMT_FUNC:
            //Write the funciton name
            fprintf(fp,"%s(",info->func->id);
            //Print the arguments 
            print_exprs(fp, info->func->args);
            //Close the braces 
            fprintf(fp,");\n");
            break;

        case STMT_ARRAY_ASSIGN:  
            //Print the ident and index
            fprintf(fp,"%s[", info->array_id);
        
            //Print array list
            print_exprs(fp, info->arrayinds);
            //Print the end and expression sign
            fprintf(fp,"] := ");
            //Now print the expression
            print_expression(fp, info->assign.asg_expr);
            //Close the expression 
            fprintf(fp,";\n");
            break;

        case STMT_ARRAY_READ:
            fprintf(fp,"read %s[", info->array_id);
            print_exprs(fp, info->arrayinds);
            //Print the end and expression sign
            fprintf(fp,"];\n");
            break;

    }
}

void print_exprs(FILE *fp, Exprs *args){
    
    //Given that expression lists may be empty
    if(args == NULL){
        return;
    }

    //Print the current param
    Expr *current = args->first;

    print_expression(fp, current);
    if(args->rest != NULL){
        //Add the comma and whitespace, then continue printing
        fprintf(fp, ", ");
        print_exprs(fp, args->rest);
    }
}


void print_expression(FILE *fp, Expr *expr){
    //Find the type of expression and print it
    ExprKind kind = expr->kind;
    // Switch on kind to print
    switch(kind) {
        case EXPR_ID:
            fprintf(fp, "%s", expr->id);
            break;
        case EXPR_CONST:
            print_constant(fp, &(expr->constant));
            break;
        case EXPR_BINOP:
            print_expression(fp, expr->e1);
            fprintf(fp," %s ", binopname[expr->binop]);
            print_expression(fp, expr->e2);
            break;
        case EXPR_UNOP:
            print_expression(fp, expr->e1);
            fprintf(fp," %s ", unopname[expr->unop]);
            print_expression(fp, expr->e2);
            break;
        case EXPR_ARRAY:
            fprintf(fp,"%s[", expr->id);
            print_exprs(fp,expr->indices);
            fprintf(fp,"]");
            break;
    }
}

void print_constant(FILE *fp, Constant *cons){
    //Find the type of constant.
    Type type = cons->type;
    switch(type){
        case BOOL_TYPE:
            fprintf(fp, "%s", cons->val.bool_val ? "true" : "false");
            break;
        case INT_TYPE: 
            fprintf(fp, "%d", cons->val.int_val);
            break;
        case FLOAT_TYPE:
            fprintf(fp, "%f", cons->val.float_val);
            break;
        case STRING_CONST:
            fprintf(fp, "%s", cons->val.string);
            break;
        default:
            break;
    }
}

void print_indents(FILE *fp){
    int i;
    for(i=0; i<indent_level; i++){
        fprintf(fp,"    ");
    }
}