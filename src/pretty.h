/* pretty.h */

#include <stdio.h>
#include "std.h"
#include "ast.h"

Procs* sort_procs(Procs *procs);
void pretty_prog(FILE *fp, Program *prog);
void print_procedures(FILE *fp, Procs *processes);
void print_header(FILE *fp, Header *header);
void print_params(FILE *fp, Params *params);
void print_body(FILE *fp, Body *body);
void print_declarations(FILE *fp, Decls *declarations);
void print_program(FILE *, Program *);
void print_declarations(FILE *, Decls *);
void print_statements(FILE *, Stmts *);
void print_indents(FILE *fp);
void print_statement(FILE *fp, Stmt *statement);
void print_expression(FILE *fp, Expr *expr);
void print_constant(FILE *fp, Constant *cons);
void print_arguments(FILE *fp, Args *args);
void print_expr_list(FILE *fp, ArrayElems *elems);