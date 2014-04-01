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
int cur_precedence;

void
pretty_prog(FILE *fp, Program *prog) {
    //Set our initial indent level
    indent_level=0;
    //Print all procedures
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
    int num_procs=1;
    Procs *rest = procs->rest;
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
    
    // Define current precedence
    cur_precedence=0;

    //Switch on kind of statement and print appropriately
    switch(kind) {
        case STMT_ASSIGN: 
            //Print the left expression
            print_expression(fp, info->assign.asg_ident);
            fprintf(fp, " := ");
            //Redefine current precedence
            cur_precedence=0;
            print_expression(fp, info->assign.asg_expr);
            fprintf(fp,";\n");
            break;

        case STMT_COND:
            //Hand off to seperate function to print if statements
            print_conds(fp, &info->cond);
            break;

        case STMT_READ:
            fprintf(fp,"read ");
            print_expression(fp, info->read);
            fprintf(fp,";\n");
            break;

        case STMT_WHILE:
            //Print while
            print_while(fp, &info->loop);
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
    }
}

void print_while(FILE *fp, While *loop){
    //Print the while
    fprintf(fp,"while ");
    //Print the expression
    Expr *while_cond = loop->cond;
    print_expression(fp, while_cond);
    //Print the do
    fprintf(fp, " do\n");
    //Print the statements
    indent_level++;
    print_statements(fp, loop->body);
    indent_level--;

    //Print the closing do
    print_indents(fp);
    fprintf(fp, "od\n");
    return;
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
           print_binop(fp, expr);
            break;
        case EXPR_UNOP:
            print_unop(fp, expr);
            break;
        case EXPR_ARRAY:
            fprintf(fp,"%s[", expr->id);
            print_exprs(fp,expr->indices);
            fprintf(fp,"]");
            break;
    }
}

void print_binop(FILE *fp, Expr *bin_expr){

    BOOL brackets = cur_precedence>binopprec[bin_expr->binop];

    //Check if we need brackets
    if(brackets) fprintf(fp, "(");
    else cur_precedence = binopprec[bin_expr->binop];

    //Print the expression regardless of brackets
    print_expression(fp, bin_expr->e1);
    fprintf(fp," %s ", binopname[bin_expr->binop]);

    //If the binop expresion is left associative, then we need to 
    //print brackets. I.e. if it's 24/(6/2) then we need that.
    //Likewise for 6 - (2 - 1) we must print the brackets

    if(!is_commutative(bin_expr)){
        //Then we need to temporarily increase precedence
        cur_precedence++;
        print_expression(fp, bin_expr->e2);
        cur_precedence--;
    } else {
        //Just print
        print_expression(fp, bin_expr->e2);

    }

    if(brackets) fprintf(fp, ")");

    return;
}


BOOL is_commutative(Expr *expr){
    if(expr->binop == BINOP_ADD || expr->binop == BINOP_MUL){
        return TRUE;
    } else {
        return FALSE;
    }

}

void print_unop(FILE *fp, Expr *unop_expr){
    BOOL brackets = cur_precedence>unopprec[unop_expr->unop];

    //Check if we need brackets
    if(brackets) fprintf(fp, "(");
    else cur_precedence = unopprec[unop_expr->unop];

    //Print the expression regardless of brackets
    fprintf(fp,"%s ", unopname[unop_expr->unop]);
    print_expression(fp, unop_expr->e1);

    //Print brackets as required
    if(brackets) fprintf(fp, ")");

}

void print_conds(FILE *fp, Cond *info){
    // Print if branch
    fprintf(fp,"if ");
    indent_level++;
    print_expression(fp, info->cond);
    indent_level--;
   
    //Print then branch
    fprintf(fp," then\n");
    indent_level++;
    print_statements(fp, info->then_branch);
    indent_level--;

    //Possibly print else brach
    if(info->else_branch != NULL){
        print_indents(fp);
        fprintf(fp,"else\n");
        indent_level++;
        print_statements(fp, info->else_branch);
        indent_level--;
    }

    //Print the fi
    print_indents(fp);
    fprintf(fp, "fi\n");
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