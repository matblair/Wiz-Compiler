/* pretty.c */

/*-----------------------------------------------------------------------
    A stub for a pretty-printer for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

//includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "std.h"

#define INDENT 4

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
void print_expression(FILE *, Expr *);

BOOL check_parens(Expr *, void *, ExprKind);
int get_precedence(void *, ExprKind);

void
pretty_prog(FILE *fp, Program *prog) {

    print_program(fp, prog);
    //report_error_and_exit("Pretty-print has not been implemented yet");
}

void sort_procs(Procs *proc_list) {
    int num_procs = 0;
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
    proc_array = (Proc **)malloc(sizeof(Proc *) * num_procs);
    p = proc_list;
    for (int i=0 ; i<num_procs ; i++)
    {
        proc_array[i] = p->first;
        p = p->rest;
    }
    qsort(proc_array, num_procs, sizeof(Proc*), proc_compar);
    //now sort the Proc elements in proc_list
    p = proc_list;
    for (int i=0 ; i<num_procs ; i++)
    {
        p->first = proc_array[i];
        p = p->rest;
    }
    //free memory used for array
    free(proc_array);
}

int proc_compar(const void *p1, const void *p2) {
    Proc *proc1 = * (Proc **) p1;
    Proc *proc2 = * (Proc **) p2;
    return strcmp(proc1->proc_name, proc2->proc_name);
}

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

void print_indent(FILE *fp, int indent_level) {
    for (int i=0 ; i<indent_level*INDENT ; i++) {
        fprintf(fp, " ");
    }
}

void print_statements(FILE *fp, Stmts *statements, int indent) {
    if (statements == NULL) {
        return;
    }

    print_statement(fp, statements->first, indent);
    print_statements(fp, statements->rest, indent);
}

void print_statement(FILE *fp, Stmt *statement, int indent) {
    //switch based on statement type
    switch (statement->kind) {
        case STMT_ASSIGN:
            print_indent(fp, indent);
            fprintf(fp, "%s := ", statement->info.assign.asg_id);
            print_expression(fp, statement->info.assign.asg_expr);
            fprintf(fp, ";\n");
            break;
        case STMT_ARRAYASSIGN:
            print_indent(fp, indent);
            //in this case the LHS value is stored as an expression
            print_expression(fp, statement->info.array_assign.array_access);
            fprintf(fp, " := ");
            print_expression(fp, statement->info.array_assign.asg_expr);
            fprintf(fp, ";\n");
            break;
        case STMT_COND:
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
            break;
        case STMT_READ:
            print_indent(fp, indent);
            fprintf(fp, "read %s;\n", statement->info.read);
            break;
        case STMT_WHILE:
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
            break;
        case STMT_WRITE:
            print_indent(fp, indent);
            //print the write expression
            fprintf(fp, "write ");
            print_expression(fp, statement->info.write);
            fprintf(fp, ";\n");
            break;
        case STMT_PROC:
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
            break;
    }
}

void print_expression(FILE *fp, Expr *expr) {
    //switch on expression type
    switch (expr->kind) {
        case EXPR_ID:
            fprintf(fp, "%s", expr->id);
            break;
        case EXPR_CONST:
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
            break;
        case EXPR_BINOP:
            //decide whether left sub-expression needs parens
            if (check_parens(expr->e1, (void*)expr->binop, expr->kind)) {
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
            if (check_parens(expr->e2, (void*)expr->binop, expr->kind)) {
                fprintf(fp, "(");
                print_expression(fp, expr->e2);
                fprintf(fp, ")");
            } else {
                print_expression(fp, expr->e2);
            }
            break;
        case EXPR_UNOP:
            //switch based on unary expression type
            if (expr->unop == UNOP_MINUS) {
                fprintf(fp, "-");
            } else {
                fprintf(fp, "not ");
            }
            if (check_parens(expr->e1, (void*)expr->unop, expr->kind)) {
                fprintf(fp, "(");
                print_expression(fp, expr->e1);
                fprintf(fp, ")");
            } else {
                print_expression(fp, expr->e1);
            }
            break;
        case EXPR_ARRAY:
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
            break;
    }
}

BOOL check_parens(Expr *inside_expr, void *op, ExprKind kind) {
    //if inside expr is not an operation, no need for parens
    ExprKind inside_kind = inside_expr->kind;
    void *inside_op;
    switch (inside_kind) {
        case EXPR_BINOP:
            inside_op = (void*)inside_expr->binop;
            break;
        case EXPR_UNOP:
            inside_op = (void*)inside_expr->unop;
            break;
        default:
            return FALSE; /* in default case need no parens */
            break;
    }
    int outside_prec = get_precedence(op, kind);
    int inside_prec = get_precedence(inside_op, inside_kind);
    //need parens if the inside operation has lower precedence
    //than the outside one
    if (inside_prec < outside_prec) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * calculates the precedence level of a given operation, using values:
 * 1: or
 * 2: and
 * 3: not
 * 4: = != < <= > >=
 * 5: + -
 * 6: * /
 * 7: - (unary version)
 *
 * input: op: either a UnOp or BinOp cast to void *
 *        kind: the kind of expression (EXPR_BINOP or EXPR_UNOP)
 * output: the precedence level as an int
 * (-1 cases should never occur)
 */
int get_precedence(void *op, ExprKind kind) {
    if (kind == EXPR_UNOP) {
        switch ((UnOp)op) {
            case UNOP_MINUS:
                return 7;
                break;
            case UNOP_NOT:
                return 3;
                break;
            default:
                return -1;
                break;
        }
    } else if (kind == EXPR_BINOP) {
        switch ((BinOp)op) {
            case BINOP_MUL:
            case BINOP_DIV:
                return 6;
                break;
            case BINOP_ADD:
            case BINOP_SUB:
                return 5;
                break;
            case BINOP_EQ:
            case BINOP_NOTEQ:
            case BINOP_LT:
            case BINOP_LTEQ:
            case BINOP_GT:
            case BINOP_GTEQ:
                return 4;
                break;
            case BINOP_AND:
                return 2;
                break;
            case BINOP_OR:
                return 1;
                break;
            default:
                return -1;
                break;
        }
    } else {
        return -1;
    }
}
