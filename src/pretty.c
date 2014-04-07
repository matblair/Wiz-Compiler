/*                             pretty.c                                */
/*-----------------------------------------------------------------------
    Developed by: #undef teamname
    Based on template code provided by Harald Sondergard for COMP90045.
    Provides a pretty printer for the Wiz programming language. Takes
    an abstract syntax tree as defined in ast.h and prints according
    to the provided rules.

    Original message included as follows:

    " A stub for a pretty-printer for Iz programs.
    For use in the COMP90045 project 2014."
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "pretty.h"

/*-----------------------------------------------------------------------
    Defines our start precedence and indentation. Increasing precedence
    will increase the number of brackets used, increasing the indent
    level will shift the output to the right be 4*n spaces.
-----------------------------------------------------------------------*/
#define START_PREC 0
#define START_INDENT 1


/*----------------------------------------------------------------------
    Functions that organise the printing process and
    sort the procs to print.  
-----------------------------------------------------------------------*/
void
pretty_prog(FILE *fp, Program *prog) {
    //Print all procedures
    print_procedures(fp, prog->procedures);
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

int proc_comparison(const void *a, const void *b){
    //Cast to procs
    Proc *proc_a = *((Proc **) a);
    Proc *proc_b = *((Proc **) b);
    char *id_a = proc_a->header->id;
    char *id_b = proc_b->header->id;
    //Sort by string name
    return (strcmp(id_a, id_b));
}


/*----------------------------------------------------------------------
    Functions that are responsibile for managing the 
    printing of a procedure
-----------------------------------------------------------------------*/

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
    //Print declarations and statements with their respective functions
    print_declarations(fp, body->decls, START_INDENT);
    
    //Add the one line break required
    fprintf(fp,"\n");
    print_statements(fp, body->statements, START_INDENT);
}

/*----------------------------------------------------------------------
    Functions that are responsible for printing 
    declarations
-----------------------------------------------------------------------*/

void print_declarations(FILE *fp, Decls *declarations, int indents) {
    //Check non null conditions
    if (declarations == NULL) {
        return;
    }
    //Print the current declaration       
    Decl *decl = declarations->first;

    print_indents(fp, indents);

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
    print_declarations(fp, declarations->rest, indents);
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

/*----------------------------------------------------------------------
    Functions that are responsible for printing statements 
-----------------------------------------------------------------------*/

void print_statements(FILE *fp, Stmts *statements, int indents) {
    if (statements == NULL) {
        return;
    }
    // Assign current value
    Stmt *statement = statements->first;
    print_indents(fp, indents);

    // Print the current statement
    print_statement(fp, statement, indents);

    //Continue along the datastructure
    print_statements(fp, statements->rest, indents);
}

void print_statement(FILE *fp, Stmt *statement, int indents) {
    //Find the type of statement and call the appropriate function.
    StmtKind kind = statement->kind;
    SInfo *info = &(statement->info);
    
    //Switch on kind of statement and print appropriately
    switch(kind) {
        case STMT_ASSIGN: 
            //Print the left expression
            print_expression(fp, info->assign.asg_ident,START_PREC);
            fprintf(fp, " := ");
            print_expression(fp, info->assign.asg_expr,START_PREC);
            fprintf(fp,";\n");
            break;

        case STMT_COND:
            //Hand off to seperate function to print if statements
            print_conds(fp, &info->cond, indents);
            break;

        case STMT_READ:
            fprintf(fp,"read ");
            print_expression(fp, info->read,START_PREC);
            fprintf(fp,";\n");
            break;

        case STMT_WHILE:
            //Print while
            print_while(fp, &info->loop, indents);
            break;

        case STMT_WRITE:   
            //Write the command
            fprintf(fp,"write ");
            //Print the expression 
            print_expression(fp, info->write,START_PREC);
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

void print_while(FILE *fp, While *loop, int indents){
    //Print the while
    fprintf(fp,"while ");
    //Print the expression
    Expr *while_cond = loop->cond;
    print_expression(fp, while_cond,START_PREC);
    //Print the do
    fprintf(fp, " do\n");
    //Print the statements
    print_statements(fp, loop->body, indents+1);
    //Print the closing do
    print_indents(fp, indents);
    fprintf(fp, "od\n");
    return;
}

void print_conds(FILE *fp, Cond *info, int indents){
    // Print if branch
    fprintf(fp,"if ");
    print_expression(fp, info->cond, START_PREC);
   
    //Print then branch
    fprintf(fp," then\n");
    print_statements(fp, info->then_branch, indents+1);

    //Possibly print else brach
    if(info->else_branch != NULL){
        print_indents(fp, indents);
        fprintf(fp,"else\n");
        print_statements(fp, info->else_branch,indents+1);
    }
    //Print the fi
    print_indents(fp, indents);
    fprintf(fp, "fi\n");
}

/*----------------------------------------------------------------------
    Functions that are responsible for printing 
    individual expressions and constants 
-----------------------------------------------------------------------*/

void print_exprs(FILE *fp, Exprs *args){  
    //Given that expression lists may be empty
    if(args == NULL){
        return;
    }
    //Print the current param
    Expr *current = args->first;
    print_expression(fp, current,START_PREC);
    if(args->rest != NULL){
        //Add the comma and whitespace, then continue printing
        fprintf(fp, ", ");
        print_exprs(fp, args->rest);
    }
}



void print_expression(FILE *fp, Expr *expr, int prec){
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
           print_binop(fp, expr, prec);
            break;
        case EXPR_UNOP:
            print_unop(fp, expr, prec);
            break;
        case EXPR_ARRAY:
            fprintf(fp,"%s[", expr->id);
            print_exprs(fp,expr->indices);
            fprintf(fp,"]");
            break;
    }
}

void print_binop(FILE *fp, Expr *bin_expr, int prec){

    BOOL brackets = prec>binopprec[bin_expr->binop];

    //Check if we need brackets
    if(brackets) fprintf(fp, "(");
    else prec = binopprec[bin_expr->binop];

    if(!is_commutative(bin_expr->e1)){
        print_expression(fp, bin_expr->e1, binopprec[bin_expr->binop]+1);
    } else {
        print_expression(fp, bin_expr->e1, binopprec[bin_expr->binop]);
    }
    fprintf(fp," %s ", binopname[bin_expr->binop]);

    //If the binop expresion is left associative, then we need to 
    //print brackets. I.e. if it's 24/(6/2) then we need that.
    //Likewise for 6 - (2 - 1) we must print the brackets
    if(!is_commutative(bin_expr)){
        //Then we need to temporarily increase precedence
        print_expression(fp, bin_expr->e2, prec+1);
    } else {
        //Just print
        print_expression(fp, bin_expr->e2, prec);
    }

    if(brackets) fprintf(fp, ")");

    return;
}

void print_unop(FILE *fp, Expr *unop_expr, int prec){
    BOOL brackets = prec>unopprec[unop_expr->unop];

    //Check if we need brackets
    if(brackets) fprintf(fp, "(");
    else prec = unopprec[unop_expr->unop];

    //Print the expression regardless of brackets
    fprintf(fp,"%s ", unopname[unop_expr->unop]);
    print_expression(fp, unop_expr->e1,prec);

    //Print brackets as required
    if(brackets) fprintf(fp, ")");

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

/*----------------------------------------------------------------------
    Functions to help with printing minimal
    brackets and printing correct indentation
-----------------------------------------------------------------------*/
BOOL is_commutative(Expr *expr){
    if(expr->binop == BINOP_ADD || expr->binop == BINOP_MUL){
        return TRUE;
    } else {
        return FALSE;
    }
}

void print_indents(FILE *fp, int indents){
    int i;
    for(i=0; i<indents; i++){
        fprintf(fp,"    ");
    }
}