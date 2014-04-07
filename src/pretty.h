/* pretty.h */
/*-----------------------------------------------------------------------
    Developed by: #undef teamname
    Based on template code provided by Harald Sondergard for COMP90045.
    Provides function definitions for all functions implemented in
    order to create the pretty printer. 
-----------------------------------------------------------------------*/

#include <stdio.h>
#include "std.h"
#include "ast.h"

/*----------------------------------------------------------------------
    Definitions for functions that organise the printing process and
    sort the procs to print.  
-----------------------------------------------------------------------*/

Procs* sort_procs(Procs *procs);
void pretty_prog(FILE *fp, Program *prog);
void print_procedures(FILE *fp, Procs *processes);
void print_program(FILE *, Program *);

/*----------------------------------------------------------------------
    Definitions for functions that are responsibile for managing the 
    printing of a procedure
-----------------------------------------------------------------------*/

void print_header(FILE *fp, Header *header);
void print_params(FILE *fp, Params *params);
void print_body(FILE *fp, Body *body);

/*----------------------------------------------------------------------
    Definitions for functions that are responsible for printing 
    declarations
-----------------------------------------------------------------------*/

void print_array_decl(FILE *fp, Intervals *intervals);
void print_declarations(FILE *fp, Decls *declarations, int indents);

/*----------------------------------------------------------------------
    Definitions for functions that are responsible for printing 
    statements 
-----------------------------------------------------------------------*/

void print_conds(FILE *fp, Cond *info, int indents);
void print_while(FILE *fp, While *loop, int indents);
void print_statement(FILE *fp, Stmt *statement, int indents);
void print_statements(FILE *fp, Stmts *statements, int indents);

/*----------------------------------------------------------------------
    Definitions for functions that are responsible for printing 
    individual expressions and constants 
-----------------------------------------------------------------------*/
    
void print_binop(FILE *fp, Expr *bin_expr, int prec);
void print_unop(FILE *fp, Expr *unop_expr, int prec);
void print_exprs(FILE *fp, Exprs *args);
void print_expr_list(FILE *fp, Exprs *elems);
void print_expression(FILE *fp, Expr *expr, int prec);
void print_constant(FILE *fp, Constant *cons);

/*----------------------------------------------------------------------
    Definitions for utility functions to help with printing minimal
    brackets and printing correct indentation
-----------------------------------------------------------------------*/

void print_indents(FILE *fp, int indents);
BOOL is_commutative(Expr *expr);
