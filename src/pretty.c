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

// Simple wrapper responsible for printing all procedures, could be 
// extended if the language required it for global variables etc.
void pretty_prog(FILE *fp, Program *prog) {
    //Print all procedures
    print_procedures(fp, prog->procedures);
}

// Simple function to use with qsort to sort procedures. Simply uses
// strcmp after extracting the procedure ids and returns the value
int proc_comparison(const void *a, const void *b){
    // Cast to procs
    Proc *proc_a = *((Proc **) a);
    Proc *proc_b = *((Proc **) b);
    char *id_a = proc_a->header->id;
    char *id_b = proc_b->header->id;
    // Sort by string name
    return (strcmp(id_a, id_b));
}

// Simple function responsible for sorting all the procedures and 
// then calling the appropriate printing function on each procedure
void print_procedures(FILE *fp, Procs *procs){

    // Go through and count the number of procedures
    int num_procs=1;
    Procs *rest = procs->rest;
    while(rest!=NULL){
        num_procs++;
        rest = rest->rest;
    }

    // Create the array of procedures to then sort
    Proc **proc_ptrs = (Proc **) malloc(num_procs * sizeof(Proc *));
    Procs *c = procs;
    int i=0;
    for(i=0; i<num_procs; i++){
        proc_ptrs[i] = c->first;
        c = c->rest; 
    }

    // Sort the array using qsort and the proc_comparision function
    // which compares by proc id lexicographically
    qsort(proc_ptrs, num_procs, sizeof(Proc *), proc_comparison);

    // Go through the array and print each procedure
    for(i=0; i<num_procs; i++){
        // Get the current process
        Proc *current = proc_ptrs[i];

        // Print the header
        print_header(fp, current->header);
        // Print the body
        print_body(fp, current->body);
        // Print a new line
        fprintf(fp, "end\n\n");
    }

    // Free all the pointers like a responsible c programmer.
    free(proc_ptrs);
}

/*----------------------------------------------------------------------
    Functions that are responsibile for managing the 
    printing of a procedure
-----------------------------------------------------------------------*/
// Prints a header with the appropriate wrapping (i.e. proc id(params))
void print_header(FILE *fp, Header *header){
    // Print opening line
    fprintf(fp, "proc %s",header->id);
    // Print parameters 
    if(header->params!=NULL){
        fprintf(fp, " ("); // White space before params
        print_params(fp, header->params);
    } else {
        // If there are no parameters, print empty brackets
        fprintf(fp, " ()\n");
    }
}

// Parameters are a linked list, this function goes through the linked list
// and prints the parameter as required. It is recursive by definiton.
void print_params(FILE *fp, Params *params){
    
    // Print the current param
    Param *current = params->first;

    // Check the type of the value and print appropriately
    ParamsInd ind = current->ind;
    Type type = current->type;
    char *id = current->id;
    fprintf(fp, "%s %s %s", valnames[ind], typenames[type], id);

    // If we have more parameters then we can add the comma and 
    // print the rest recursively.
    if(params->rest != NULL){
        fprintf(fp, ", ");
        print_params(fp, params->rest);
    } else {
        // Otherwise we have reached the end of the list so close
        // the brackets
        fprintf(fp,")\n");
    }
}

// Simple wrapper function that delegates the printing of the appropriate
// part of a declaration and statemetns.
void print_body(FILE *fp, Body *body){
    // Print declarations
    print_declarations(fp, body->decls, START_INDENT);
    // Add the one line break required
    fprintf(fp,"\n");
    // Print the statements
    print_statements(fp, body->statements, START_INDENT);
}

/*----------------------------------------------------------------------
    Functions that are responsible for printing 
    declarations
-----------------------------------------------------------------------*/

// Progress through the linked list of declarations and print each one
// as required according to its type. This function is recursive by nature
void print_declarations(FILE *fp, Decls *declarations, int indents) {
    
    // Check non null conditions due to recursive nature
    if (declarations == NULL) {
        return;
    }

    // Get the current declaration       
    Decl *decl = declarations->first;
    // Print the appropriate level of indents on this line
    print_indents(fp, indents);

    // Check if array declaration or standard variable declaration and then
    // print as appropriate
    if(decl->array!=NULL){
        fprintf(fp, "%s %s[", typenames[decl->type], decl->id);
        print_array_decl(fp, decl->array);
        fprintf(fp, "];\n");
    } else {
        fprintf(fp, "%s %s;\n", typenames[decl->type], decl->id);
    }

    // Continue along our data structure
    print_declarations(fp, declarations->rest, indents);
}

// Responsible for traversing the array declaration linked list and printing
// each interval.
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
// A simple function that recursively walks through the linked list of 
// statements and calls the appropriate function to print each one.
void print_statements(FILE *fp, Stmts *statements, int indents) {

    // Check for null condition before printing
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

// A rather beefy function that is responsible for detmerining the type of 
// the statement and then either printing (for simple statements) or calling
// helper functions on (for more complex statements) each statement
void print_statement(FILE *fp, Stmt *statement, int indents) {
    
    // Find the type of statement and the info associated
    StmtKind kind = statement->kind;
    SInfo *info = &(statement->info);
    
    // Switch on kind of statement and print appropriately
    switch(kind) {
        case STMT_ASSIGN: 
            // Print the left expression
            print_expression(fp, info->assign.asg_ident,START_PREC);
            fprintf(fp, " := ");
            // Print the right expression
            print_expression(fp, info->assign.asg_expr,START_PREC);
            fprintf(fp,";\n");
            break;

        case STMT_COND:
            // Hand off to seperate function to print if statements
            print_conds(fp, &info->cond, indents);
            break;

        case STMT_READ:
            fprintf(fp,"read ");
            // Print the read expressions using helper function
            print_expression(fp, info->read,START_PREC);
            fprintf(fp,";\n");
            break;

        case STMT_WHILE:
            // Print while statements using helper functions
            print_while(fp, &info->loop, indents);
            break;

        case STMT_WRITE:   
            // Write the command
            fprintf(fp,"write ");
            // Print the expression 
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

// Helper function to print the while statements, which consist of 
// a condition followed by a series of statements.
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
    //Print the closing do at appropriate indentation
    print_indents(fp, indents);
    fprintf(fp, "od\n");
    return;
}

// Helper function to print the if statements which consist of a condition
// followed by a series of statements and possibly an else statement. 
void print_conds(FILE *fp, Cond *info, int indents){
    // Print the if branch with condition
    fprintf(fp,"if ");
    print_expression(fp, info->cond, START_PREC);
   
    // Print then branch with statements
    fprintf(fp," then\n");
    print_statements(fp, info->then_branch, indents+1);

    // Possibly print else brach if it exists at the appropriate indent level
    if(info->else_branch != NULL){
        print_indents(fp, indents);
        fprintf(fp,"else\n");
        print_statements(fp, info->else_branch,indents+1);
    }
    // Print the fi at the appropriate indentation level
    print_indents(fp, indents);
    fprintf(fp, "fi\n");
}

/*----------------------------------------------------------------------
    Functions that are responsible for printing 
    individual expressions and constants 
-----------------------------------------------------------------------*/
// Simple function to recursively walkt through an expression list printing 
// the commas if the expression list contains multiple expressions. This 
// function is recursive.
void print_exprs(FILE *fp, Exprs *args){  
    // Given that expression lists may be empty
    if(args == NULL){
        return;
    }
    // Get and print the current expression
    Expr *current = args->first;
    print_expression(fp, current,START_PREC);
    
    // Check if we have more expressions and if so, continue printing
    // after adding the comma and white space.
    if(args->rest != NULL){
        fprintf(fp, ", ");
        print_exprs(fp, args->rest);
    }
}


// A function to identify the kind of expression and then if appropriate 
// print it (for simple expressions) or call a helper function if required. 
void print_expression(FILE *fp, Expr *expr, int prec){
    //Find the type of expression and print it
    ExprKind kind = expr->kind;
    
    // Switch on kind to print
    switch(kind) {
        case EXPR_ID:
            //Simply print the identifier
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
            // Print the array identifier and opening bracket.
            fprintf(fp,"%s[", expr->id);
            // Then print all the indices using the helper function
            print_exprs(fp,expr->indices);
            // Now close the brackets
            fprintf(fp,"]");
            break;
    }
}

// A helper function to print a binary operation. Will print to the minimum
// number of brackets if precedence is passed in correctly. 
void print_binop(FILE *fp, Expr *bin_expr, int prec){
    // Check if our precedence dictates the need for brackets
    BOOL brackets = prec>binopprec[bin_expr->binop];

    // If we need brackets then add them, otherwise we are the highest 
    // level of precedence accounted and should set the precedence
    // thusly. 
    if(brackets) fprintf(fp, "(");
    else prec = binopprec[bin_expr->binop];

    // If the left expression is commutative, then we fake the precedence to 
    // ensure minimum bracketing otherwise print nomrally.
    if(!is_commutative(bin_expr->e1)){
        print_expression(fp, bin_expr->e1, binopprec[bin_expr->binop]+1);
    } else {
        print_expression(fp, bin_expr->e1, binopprec[bin_expr->binop]);
    }

    // Print the binary operation name
    fprintf(fp," %s ", binopname[bin_expr->binop]);

    // If the binop expresion is left associative, then we need to 
    // print brackets to handle cases like 24/(6/2). Therefore we spoof
    // the precedence level to force the expression to bracket itself.
    // otherwise print as normal
    if(!is_commutative(bin_expr)){
        print_expression(fp, bin_expr->e2, prec+1);
    } else {
        print_expression(fp, bin_expr->e2, prec);
    }

    // Close bracket if required
    if(brackets) fprintf(fp, ")");

    return;
}

// A helper function to print a unary operation. Will print to the minimum
// number of brackets if precedence is passed in correctly. 
void print_unop(FILE *fp, Expr *unop_expr, int prec){
    // Check if our precedence dictates the need for brackets
    BOOL brackets = prec>unopprec[unop_expr->unop];

    // If we need brackets then add them, otherwise we are the highest 
    // level of precedence accounted and should set the precedence
    // thusly. 
    if(brackets) fprintf(fp, "(");
    else prec = unopprec[unop_expr->unop];

    // Print the expression and pass on precedence. 
    fprintf(fp,"%s ", unopname[unop_expr->unop]);
    print_expression(fp, unop_expr->e1,prec);

    // Close bracket if required
    if(brackets) fprintf(fp, ")");
}

// A helper function to print constants as required.
void print_constant(FILE *fp, Constant *cons){
    //Find the type of constant.
    Type type = cons->type;

    //Switch on the type of constant
    switch(type){
        case BOOL_TYPE:
            // Conditionally print either true or false based on the boolean
            // value stored in the constant.
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
            // Should not get her as only remaining types are int_array_type
            // and float array type. Therefore return
            return; 
    }
}

/*----------------------------------------------------------------------
    Functions to help with printing minimal
    brackets and printing correct indentation
-----------------------------------------------------------------------*/
// A function to help return whether or not a function is comutative. For 
// the purpose of this language, we are only concerned with multiplication
// and addition.
BOOL is_commutative(Expr *expr){
    if(expr->binop == BINOP_ADD || expr->binop == BINOP_MUL){
        return TRUE;
    } else {
        return FALSE;
    }
}

// A simple function to print the requisite four spaces per indent level
// on the current line.
void print_indents(FILE *fp, int indents){
    int i;
    for(i=0; i<indents; i++){
        fprintf(fp,"    ");
    }
}