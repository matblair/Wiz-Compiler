/*                       error_printer.c                                */
/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides functions to print informative error messages to users
    for a ranger of scenarios that occur during syntactic, semantic
    and optimisation processes

-----------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pretty.h"

/*----------------------------------------------------------------------
    Internal definitons used for colour only.
-----------------------------------------------------------------------*/

#define KNRM   "\x1B[0m"
#define KRED   "\x1B[31m"
#define KGRN   "\x1B[32m"
#define KYEL   "\x1B[33m"
#define KBLU   "\x1B[34m"
#define KMAG   "\x1B[35m"
#define KCYN   "\x1B[36m"
#define KWHT   "\x1B[37m"
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\e[1;97m"      /* Bold White */

/*----------------------------------------------------------------------
    Function implementations.
-----------------------------------------------------------------------*/
// All of these functions simply print to stderr with formatting as appropriate
// because of this we will not be commenting all of these as it is unneccesary.

void print_bold(const char *string) {
    fprintf(stderr, BOLDWHITE "%s\n" KNRM, string);
}

void print_missing_main_error() {
    fprintf(stderr, BOLDRED "FATALERROR:" BOLDWHITE" program must contain a "
            BOLDCYAN "main" BOLDWHITE" function.\n\n");
}

void print_undefined_variable_error(Expr *e, Expr *parent,  int line_no) {
    //Not good. Let the user know.
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in expression:\n" KNRM "", line_no);
    print_indents(stderr, 4);
    if (parent == NULL) {
        print_expression(stderr, e, 0);
    } else {
        print_expression(stderr, parent, 0);
    }
    fprintf(stderr, ";\n");
    fprintf(stderr, "Variable " KYEL "%s" KNRM" is undefined.\n\n", e->id);
}

void print_not_array_error(Expr *e, Decl *d,  int line_no) {
    //Not good. Let the user know.
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in expression:\n" KNRM "", line_no);
    print_indents(stderr, 4);
    if (e != NULL) {
        print_expression(stderr, e, 0);
    }
    fprintf(stderr, ";\n");
    fprintf(stderr, "Variable " KYEL "%s" KNRM" is not an array.\n", e->id);

    if (d != NULL) {
        fprintf(stderr, "Originally declared as:\n");
        fprintf(stderr, KYEL "\t\t%s" KNRM " %s;\n\n",
                typenames[d->type], d->id);
    }
}

void print_undefined_proc_call_error(Function *f, int line_no) {
    //Not good. Let the user know.
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in expression:\n" KNRM "", line_no);
    print_indents(stderr, 4);
    fprintf(stderr, "%s(", f->id);
    print_exprs(stderr, f->args);
    fprintf(stderr, ")\n");
    fprintf(stderr, "Proc " KYEL "%s" KNRM" has not been defined.\n\n", f->id);
}

void print_dupe_proc_errors(Proc *r, Params *p, int duplicate, int original) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "proc " KYEL "%s" KNRM " has been redefined, originally "
            "defined on line %d as:\n", duplicate, r->header->id, original);
    print_indents(stderr, 3);
    print_header(stderr, r->header);
    fprintf(stderr, BOLDWHITE "redefined on line %d as:\n" KNRM, duplicate);
    print_indents(stderr, 3);
    fprintf(stderr, "proc %s (", r->header->id);
    print_params(stderr, p);
    fprintf(stderr, "\n");
}

void print_dupe_symbol_errors(char *id, Type t1, Type t2,
                              int duplicate, int original) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "symbol " KYEL "%s" KNRM " has been redefined, originally "
            "defined here:\n", duplicate, id);
    print_indents(stderr, 3);
    fprintf(stderr, "%s %s;\n", id, typenames[t1]);
    fprintf(stderr, BOLDWHITE "redefined as:\n" KNRM);
    print_indents(stderr, 3);
    fprintf(stderr, "%s %s;\n", id, typenames[t2]);
    fprintf(stderr, "\n");
}

void print_unused_symbol_error(char *id, int line_no) {
    fprintf(stderr, BOLDWHITE "%d " BOLDYELLOW "warning: " BOLDWHITE
            "symbol " KYEL "%s" KNRM " has been defined but is not used.\n\n"
            , line_no, id);
}

void print_if_error(Expr *e, Type c, int line_no) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in expression for " KBLU "if " KNRM "condition:\n", line_no);
    print_indents(stderr, 4);
    print_expression(stderr, e, 0);
    fprintf(stderr, ";\n");
    fprintf(stderr, "Result is incorrect type. Evaluates to " KYEL "%s" KNRM
            ", should evaluate to " KYEL"boolean" KNRM".\n\n", typenames[c]);
}

void print_while_error(Expr *e, Type c, int line_no) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in expression for " KBLU "while " KNRM "condition:\n", line_no);
    print_indents(stderr, 4);
    print_expression(stderr, e, 0);
    fprintf(stderr, ";\n");

    fprintf(stderr, "Result is incorrect type. Evaluates to " KYEL "%s" KNRM
            ", should evaluate to " KYEL"boolean" KNRM".\n\n", typenames[c]);
}


void print_assign_error(Assign *a, Type left, Type right, int line_no) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in assignment to " KCYN "%s:\n" KNRM, line_no, a->asg_ident->id);
    print_indents(stderr, 4);
    print_expression(stderr, a->asg_ident, 0);
    fprintf(stderr, ":= ");
    print_expression(stderr, a->asg_expr, 0);
    fprintf(stderr, ";\n");
    fprintf(stderr, "Type is incorrect. Received " KYEL "%s" KNRM
            " expression, should evaluate to " KYEL"%s" KNRM".\n\n",
            typenames[right], typenames[left]);
}

void print_func_pmismatch_error(Function *f, Params *fcallee, int line_no,
                                int expect_no, int call_no) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in call to proc " KYEL "%s" KNRM ":\n", line_no, f->id);
    print_indents(stderr, 3);
    fprintf(stderr, "proc %s (", f->id);
    print_params(stderr, fcallee);
    fprintf(stderr, BOLDWHITE "not enough parameters to match"
            " definition in call:\n");
    print_indents(stderr, 3);
    fprintf(stderr, "proc %s (", f->id);
    print_exprs(stderr, f->args);
    fprintf(stderr, ")\n");
    fprintf(stderr, "expected " KMAG "%d" KNRM " received " KMAG "%d"
            KNRM".\n\n", expect_no, call_no);
}

void print_func_ptype_error(int par_num, Type caller, Type func,
                            Function *f, int line_no) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in call to proc " KYEL "%s" KNRM ":\n", line_no, f->id);
    print_indents(stderr, 3);
    fprintf(stderr, "proc %s (", f->id);
    print_exprs(stderr, f->args);
    fprintf(stderr, ")\n");
    fprintf(stderr, "argument " KMAG "%d" KNRM " is incorrect type. expected "
            KMAG "%s" KNRM " received " KMAG "%s" KNRM".\n\n",
            par_num, typenames[func], typenames[caller]);
}

void print_array_index_error(Exprs *indices, char *id, int line_no,
                             int p_num, Type t) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in array access " KYEL "%s" KNRM ":\n", line_no, id);
    print_indents(stderr, 3);
    fprintf(stderr, "%s[", id);
    print_exprs(stderr, indices);
    fprintf(stderr, "]\n");
    fprintf(stderr, "argument " KMAG "%d" KNRM " is incorrect type. expected "
            KMAG "int" KNRM " received " KMAG "%s" KNRM".\n\n",
            p_num, typenames[t]);
}

void print_array_outofbounds_error(Exprs *indices, char *id, int line_no,
                                   int p_num, Interval *i) {
    fprintf(stderr, BOLDWHITE "%d " BOLDYELLOW "warning: " BOLDWHITE
            "in array access " KYEL "%s" KNRM ":\n", line_no, id);
    print_indents(stderr, 3);
    fprintf(stderr, "%s[", id);
    print_exprs(stderr, indices);
    fprintf(stderr, "]\n");
    fprintf(stderr, "argument " KMAG "%d" KNRM " is out of bounds. Index must "
            "be between " KYEL "%d" KNRM " and " KYEL "%d" KNRM
            " (inclusive).\n\n", p_num, i->lower, i->upper);
}


void print_binop_error(Expr *e, int line_no, Type right,
                       Type left, char *expected) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in expression:\n", line_no);
    print_indents(stderr, 4);
    print_expression(stderr, e, 0);
    fprintf(stderr, ";\n");
    fprintf(stderr, "Types are incorrect. Received " KYEL "%s" KNRM
            " and " KYEL "%s" KNRM ", should " KYEL"%s" KNRM".\n\n",
            typenames[right], typenames[left], expected);
}

void print_unop_error(Expr *e, int line_no, Type t, char *expected) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: " BOLDWHITE
            "in expression:\n", line_no);
    print_indents(stderr, 4);
    print_expression(stderr, e, 0);
    fprintf(stderr, ";\n");
    fprintf(stderr, "Type is incorrect. Received " KYEL "%s" KNRM
            KNRM ", should be a " KYEL"%s" KNRM" type.\n\n",
            typenames[t], expected);
}

void print_array_dims_error(Expr *e, int expected, int actual, int line_no) {
    fprintf(stderr, BOLDWHITE "%d " BOLDRED "error: "
            BOLDWHITE "in expression:\n" KNRM "", line_no);
    print_indents(stderr, 4);
    print_expression(stderr, e, 0);
    fprintf(stderr, "\n");
    fprintf(stderr, "Incorrect array dimensions for" KCYN " %s" KNRM
            ", expected " KYEL"%d" KNRM", actual " KYEL"%d" KNRM".\n\n", e->id,
            expected, actual);
}


// Report an error to stderr then exit with EXIT_FAILURE.
void report_error_and_exit(const char *msg) {
    fprintf(stderr, "\a" BOLDRED "FATAL ERROR: " BOLDWHITE "%s\n\n", msg);
    exit(EXIT_FAILURE);
}
