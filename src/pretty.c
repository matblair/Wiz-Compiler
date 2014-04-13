/* pretty.c */

/*-----------------------------------------------------------------------
    A stub for a pretty-printer for Iz programs.
    For use in the COMP90045 project 2014.
 
     Edited by team #undef TEAMNAME for full language Wiz
-----------------------------------------------------------------------*/

//includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "std.h"

#define INDENT 4

#define BINOP_OR_PREC 1
#define BINOP_AND_PREC 2
#define UNOP_NOT_PREC 3
#define BINOP_EQ_PREC 4
#define BINOP_NOTEQ_PREC 4
#define BINOP_LT_PREC 4
#define BINOP_GT_PREC 4
#define BINOP_LTEQ_PREC 4
#define BINOP_GTEQ_PREC 4
#define BINOP_ADD_PREC 5
#define BINOP_SUB_PREC 5
#define BINOP_MUL_PREC 6
#define BINOP_DIV_PREC 6
#define UNOP_MINUS_PREC 7

//TYPES
typedef enum {
    RIGHT, LEFT, UNARY
} SubExprKind;

//DECLS
extern void report_error_and_exit(const char *msg);


void sort_procs(Procs *);
int proc_compar(const void *, const void *);
void print_program(FILE *, Program *);
void print_proc(FILE *, Proc *);
void print_header(FILE *, Proc *);
void print_pp(FILE *, ParamMode);
void print_type(FILE *, Type);
void print_declarations(FILE *, Decls *);
void print_indent(FILE *, int);
void print_statements(FILE *, Stmts *, int);

void print_statement(FILE *, Stmt *, int);
void print_stmt_assign(FILE *, Stmt *, int);
void print_stmt_cond(FILE *, Stmt *, int);
void print_stmt_read(FILE *, Stmt *, int);
void print_stmt_while(FILE *, Stmt *, int);
void print_stmt_write(FILE *, Stmt *, int);
void print_stmt_proc(FILE *, Stmt *, int);

void print_expression(FILE *, Expr *);
void print_expr_const(FILE *, Expr *);
void print_expr_binop(FILE *, Expr *);
void print_expr_unop(FILE *, Expr *);
void print_expr_array(FILE *, Expr *);

BOOL check_parens(Expr *, Expr *, SubExprKind);
int get_precedence(Expr *);

void *smalloc(size_t);

/**
 * main driver function of module
 * prints program to given FILE*
 */
void pretty_prog(FILE *fp, Program *prog) {

    print_program(fp, prog);
    //report_error_and_exit("Pretty-print has not been implemented yet");
}

/**
 * sorts the provided Procs into alphabetical order, given
 * by the name of each member Proc
 */
void sort_procs(Procs *proc_list) {
    int num_procs = 0;
    int i = 0;
    Procs *p;
    Proc **proc_array;
    //first get number of procs
    p = proc_list;
    while (TRUE)
    {
        if (p == NULL) break;
        num_procs++;
        p = p->rest;
    }
    //create array for storing Proc structs
    proc_array = (Proc **)smalloc(sizeof(Proc *) * num_procs);
    p = proc_list;
    for (i=0 ; i<num_procs ; i++)
    {
        proc_array[i] = p->first;
        p = p->rest;
    }
    qsort(proc_array, num_procs, sizeof(Proc*), proc_compar);
    //now sort the Proc elements in proc_list
    p = proc_list;
    for (i=0 ; i<num_procs ; i++)
    {
        p->first = proc_array[i];
        p = p->rest;
    }
    //free memory used for array
    free(proc_array);
}

/**
 * comparison function to compare pointers to Proc* objects, which
 * have been cast to void* (needed by qsort)
 */
int proc_compar(const void *p1, const void *p2) {
    Proc *proc1 = * (Proc **) p1;
    Proc *proc2 = * (Proc **) p2;
    return strcmp(proc1->proc_name, proc2->proc_name);
}

/**
 * prints entire program (given by Program object)
 */
void print_program(FILE *fp, Program *prog) {
    Procs *p = prog->proc_list;
    Proc *next_proc;
    //sort the procs 
    sort_procs(p);
    while (TRUE) {
        next_proc = p->first;
        print_proc(fp, next_proc);
        p = p->rest;
        if (p == NULL) break;
        fprintf(fp, "\n");
    }
}

/**
 * prints a single procedure, given by proc
 */
void print_proc(FILE *fp, Proc *proc) {
    //print function header
    print_header(fp, proc);
    //print declarations
    print_declarations(fp, proc->decls);
    fprintf(fp, "\n");
    //print statements with one space gap
    print_statements(fp, proc->body, 1);
    //print "end" keyword
    fprintf(fp, "end\n");
}

/**
 * prints the header of  a procedure (line containing procedure
 * name and input specifications)
 */
void print_header(FILE *fp, Proc *proc) {
    Args *a = proc->args;
    fprintf(fp, "proc %s (", proc->proc_name);
    //print out string of arguments
    while (a != NULL) {
        print_pp(fp, a->first->pp);
        print_type(fp, a->first->type);
        fprintf(fp, "%s", a->first->arg_name);
        //if not the last arg print a comma
        a = a->rest;
        if (a != NULL) {
            fprintf(fp, ", ");
        }
    }
    //print final paren
    fprintf(fp, ")\n");
}

/**
 * prints a paramter-passing mode (i.e. val or ref); used as part of
 * printing a header
 */
void print_pp(FILE *fp, ParamMode mode) {
    switch (mode) {
        case PP_VAL:
            fprintf(fp, "val ");
            break;
        case PP_REF:
            fprintf(fp, "ref ");
            break;
    }
}

/**
 * prints a given variable type (bool, int or float), given by Type object
 */
void print_type(FILE *fp, Type type) {
    switch (type) {
        case BOOL_TYPE:
            fprintf(fp, "bool ");
            break;
        case INT_TYPE:
            fprintf(fp, "int ");
            break;
        case FLOAT_TYPE:
            fprintf(fp, "float ");
            break;
    }
}

/**
 * prints the declarations of a procedure (done at procedure start)
 */
void print_declarations(FILE *fp, Decls *declarations) {

    if (declarations == NULL) {
        return;
    }

    Decl *decl = declarations->first;
    print_indent(fp, 1);
    print_type(fp, decl->type);
    fprintf(fp, "%s", decl->id);
    //if it is an array, need to print array-bounds intervals
    if (decl->array_bounds != NULL) {
        Bounds *b = decl->array_bounds;
        fprintf(fp, "[");
        while (b != NULL) {
            fprintf(fp, "%d..%d", b->interval_start, b->interval_end);
            b = b->rest;
            //if there are more to print, separate with comma
            if (b != NULL) {
                fprintf(fp, ",");
            }
        }
        fprintf(fp, "]");
    }
    //end with semi-colon and nl
    fprintf(fp, ";\n");

    //print the rest of declarations
    print_declarations(fp, declarations->rest);
}

/**
 * prints indentation, using the current indentation level and the
 * module-defined indentation spacing given by INDENT
 */
void print_indent(FILE *fp, int indent_level) {
    int i;
    for (i=0 ; i<indent_level*INDENT ; i++) {
        fprintf(fp, " ");
    }
}

/**
 * prints a series of statements given by Stmts object
 */
void print_statements(FILE *fp, Stmts *statements, int indent) {
    if (statements == NULL) {
        return;
    }

    print_statement(fp, statements->first, indent);
    print_statements(fp, statements->rest, indent);
}


/**
 * prints a single statement given by Stmt object
 */
void print_statement(FILE *fp, Stmt *statement, int indent) {
    //switch based on statement type
    switch (statement->kind) {
        case STMT_ASSIGN:
            print_stmt_assign(fp, statement, indent);
            break;
        case STMT_COND:
            print_stmt_cond(fp, statement, indent);
            break;
        case STMT_READ:
            print_stmt_read(fp, statement, indent);
            break;
        case STMT_WHILE:
            print_stmt_while(fp, statement, indent);
            break;
        case STMT_WRITE:
            print_stmt_write(fp, statement, indent);
            break;
        case STMT_PROC:
            print_stmt_proc(fp, statement, indent);
            break;
    }
}

/**
 * prints a single assign statement, given by "statement"
 */
void print_stmt_assign(FILE *fp, Stmt *statement, int indent) {
    print_indent(fp, indent);
    //LHS val of assignment stored as an expression
    print_expression(fp, statement->info.assign.asg_id);
    fprintf(fp, " := ");
    //print the RHS val of assignment
    print_expression(fp, statement->info.assign.asg_expr);
    fprintf(fp, ";\n");
}

/**
 * prints a single cond statement, given by "statement"
 */
void print_stmt_cond(FILE *fp, Stmt *statement, int indent) {
    print_indent(fp, indent);
    fprintf(fp, "if ");
    //print the condition expression
    print_expression(fp, statement->info.cond.cond);
    fprintf(fp, " then\n");
    //recuresively print the then-branch statements at higher
    //indentation level
    print_statements(fp, statement->info.cond.then_branch, indent+1);
    //if else-branch non-null print this too
    if (statement->info.cond.else_branch != NULL) {
        print_indent(fp, indent);
        fprintf(fp, "else\n");
        print_statements(fp, statement->info.cond.else_branch, indent+1);
    }
    //print the obligatory "fi"
    print_indent(fp, indent);
    fprintf(fp, "fi\n");
}

/**
 * prints a single read statement, given by "statement"
 */
void print_stmt_read(FILE *fp, Stmt *statement, int indent) {
    print_indent(fp, indent);
    fprintf(fp, "read ");
    //print the LHS val to be read into (either an ident or array expr)
    print_expression(fp, statement->info.read);
    fprintf(fp, ";\n");
}

/**
 * prints a single while statement, given by "statement"
 */
void print_stmt_while(FILE *fp, Stmt *statement, int indent) {
    print_indent(fp, indent);
    fprintf(fp, "while ");
    //print the while conditional expression
    print_expression(fp, statement->info.loop.cond);
    fprintf(fp, " do\n");
    //print while body at higher indent
    print_statements(fp, statement->info.loop.body, indent+1);
    //print the obligatory "od"
    print_indent(fp, indent);
    fprintf(fp, "od\n");
}

/**
 * prints a single write statement, given by "statement"
 */
void print_stmt_write(FILE *fp, Stmt *statement, int indent) {
    print_indent(fp, indent);
    //print the write expression
    fprintf(fp, "write ");
    print_expression(fp, statement->info.write);
    fprintf(fp, ";\n");
}

/**
 * prints a single proc statement, given by "statement"
 */
void print_stmt_proc(FILE *fp, Stmt *statement, int indent) {
    print_indent(fp, indent);
    //print the identifier and open paren
    fprintf(fp, "%s(", statement->info.proc.proc_name);
    //recursively print arguments
    ExprList *e = statement->info.proc.proc_args;
    while (e != NULL) {
        print_expression(fp, e->first);
        e = e->rest;
        //if not at end print comma to separate args
        if (e != NULL) {
            fprintf(fp, ", ");
        }
    }
    fprintf(fp, ");\n");
}

/**
 * prints the given expression at current position (no newlines used)
 */
void print_expression(FILE *fp, Expr *expr) {
    //switch on expression type
    switch (expr->kind) {
        case EXPR_ID:
            fprintf(fp, "%s", expr->id);
            break;
        case EXPR_CONST:
            print_expr_const(fp, expr);
            break;
        case EXPR_BINOP:
            print_expr_binop(fp, expr);
            break;
        case EXPR_UNOP:
            print_expr_unop(fp, expr);
            break;
        case EXPR_ARRAY:
            print_expr_array(fp, expr);
            break;
        case EXPR_STR:
            //print string contents surrounded by quotes
            fprintf(fp, "\"%s\"", expr->id);
            break;
    }
}

/**
 * print constant expression
 */
void print_expr_const(FILE *fp, Expr *expr) {
    //need to switch on the type of the constant
    switch (expr->constant.type) {
        case BOOL_TYPE:
            if (expr->constant.val.bool_val == TRUE) {
                fprintf(fp, "true");
            } else {
                fprintf(fp, "false");
            }
            break;
        case INT_TYPE:
            fprintf(fp, "%d", expr->constant.val.int_val);
            break;
        case FLOAT_TYPE:
            fprintf(fp, "%lf", expr->constant.val.float_val);
            break;
    }
}

/**
 * print binary operation
 */
void print_expr_binop(FILE *fp, Expr *expr) {
    //decide whether left sub-expression needs parens
    if (check_parens(expr->e1, expr, LEFT)) {
        fprintf(fp, "(");
        print_expression(fp, expr->e1);
        fprintf(fp, ")");
    } else {
        print_expression(fp, expr->e1);
    }
    //print the binary operation
    switch (expr->binop) {
        case BINOP_ADD:
            fprintf(fp, " + ");
            break;
        case BINOP_SUB:
            fprintf(fp, " - ");
            break;
        case BINOP_MUL:
            fprintf(fp, " * ");
            break;
        case BINOP_DIV:
            fprintf(fp, " / ");
            break;
        case BINOP_LT:
            fprintf(fp, " < ");
            break;
        case BINOP_GT:
            fprintf(fp, " > ");
            break;
        case BINOP_LTEQ:
            fprintf(fp, " <= ");
            break;
        case BINOP_GTEQ:
            fprintf(fp, " >= ");
            break;
        case BINOP_NOTEQ:
            fprintf(fp, " != ");
            break;
        case BINOP_EQ:
            fprintf(fp, " = ");
            break;
        case BINOP_OR:
            fprintf(fp, " or ");
            break;
        case BINOP_AND:
            fprintf(fp, " and ");
            break;
    }
    //decide whether right sub-expression needs parens
    if (check_parens(expr->e2, expr, RIGHT)) {
        fprintf(fp, "(");
        print_expression(fp, expr->e2);
        fprintf(fp, ")");
    } else {
        print_expression(fp, expr->e2);
    }
}

/**
 * print unary operation
 */
void print_expr_unop(FILE *fp, Expr *expr) {
    //switch based on unary expression type
    if (expr->unop == UNOP_MINUS) {
        fprintf(fp, "-");
    } else {
        fprintf(fp, "not ");
    }
    if (check_parens(expr->e1, expr, UNARY)) {
        fprintf(fp, "(");
        print_expression(fp, expr->e1);
        fprintf(fp, ")");
    } else {
        print_expression(fp, expr->e1);
    }
}

/**
 * print an array expression (i.e. identifier, followed by square brackets
 * containing comma-separated list of sub-expressions)
 */
void print_expr_array(FILE *fp, Expr *expr) {
    //print the array identifier
    fprintf(fp, "%s[", expr->id);
    //fill in the arguments
    ExprList *e = expr->el;
    while (e != NULL) {
        print_expression(fp, e->first);
        //if not at end of list print separating comma
        e = e->rest;
        if (e != NULL) {
            fprintf(fp, ", ");
        }
    }
    //print ending ']'
    fprintf(fp, "]");
}

/**
 * given inputs:
 *  a) the inside expression of an operation 
 *  b) the outside expression of the operation (this is either a binary or
 *    unary operation)
 *  c) the kind of subexpression of inner one (either RIGHT or LEFT if
 *    outer is binary, or UNARY if outer is unary)
 *  function determines whether parentheses are required around inner
 *  expression (for unambiguous pretty-printing)
 *  returns boolean value TRUE or FALSE giving result (TRUE meaning 
 *  parentheses required)
 */
BOOL check_parens(Expr *inside_expr, Expr *outside_expr,
        SubExprKind sub_expr_kind) {
    //if inside_expr is not an operation (binary or unary), definitely do not
    //need parentheses
    if (inside_expr->kind != EXPR_BINOP && inside_expr->kind != EXPR_UNOP) {
        return FALSE;
    }
    int outside_prec = get_precedence(outside_expr);
    int inside_prec = get_precedence(inside_expr);
    //special treatment if we are looking at right subexpression of - or /
    if (sub_expr_kind == RIGHT && (outside_expr->binop == BINOP_DIV 
                || outside_expr->binop == BINOP_SUB)) {
        //artificially boost inside prec level by 1 to force parentheses
        //around right sub-expression if equal level
        inside_prec -= 1;
    }
    //need parens if the inside operation has lower precedence
    //than the outside one
    if (inside_prec < outside_prec) {
        return TRUE;
    } else {
        return FALSE;
    }
}


/**
 * calculates the precedence level of a given operation, using values
 * in #define's at top of this file
 *
 * input: op: either a UnOp or BinOp cast to void *
 *        kind: the kind of expression (EXPR_BINOP or EXPR_UNOP)
 * output: the precedence level as an int
 * (-1 cases should never occur)
 */
int get_precedence(Expr *expr) {
    if (expr->kind == EXPR_UNOP) {
        switch (expr->unop) {
            case UNOP_MINUS:
                return UNOP_MINUS_PREC;
                break;
            case UNOP_NOT:
                return UNOP_NOT_PREC;
                break;
            default:
                return -1; //should never occur
                break;
        }
    } else if (expr->kind == EXPR_BINOP) {
        switch (expr->binop) {
            case BINOP_MUL:
                return BINOP_MUL_PREC;
                break;
            case BINOP_DIV:
                return BINOP_DIV_PREC;
                break;
            case BINOP_ADD:
                return BINOP_ADD_PREC;
                break;
            case BINOP_SUB:
                return BINOP_SUB_PREC;
                break;
            case BINOP_EQ:
                return BINOP_EQ_PREC;
                break;
            case BINOP_NOTEQ:
                return BINOP_NOTEQ_PREC;
                break;
            case BINOP_LT:
                return BINOP_LT_PREC;
                break;
            case BINOP_LTEQ:
                return BINOP_LTEQ_PREC;
                break;
            case BINOP_GT:
                return BINOP_GT_PREC;
                break;
            case BINOP_GTEQ:
                return BINOP_GTEQ_PREC;
                break;
            case BINOP_AND:
                return BINOP_AND_PREC;
                break;
            case BINOP_OR:
                return BINOP_OR_PREC;
                break;
            default:
                return -1; //should never occur
                break;
        }
    } else {
        return -1; //should never occur
    }
}

/**
 * secure malloc function
 */
void *smalloc(size_t variable_size) {
    void *location = malloc(variable_size);
    if (location == NULL) {
        //means error occured with memory allocation - crash
        fprintf(stderr, "malloc call failed; aborting program execution!\n");
        exit(EXIT_FAILURE);
    }
    return location;
}

