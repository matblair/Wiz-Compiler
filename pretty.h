/* pretty.h */
/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Based on template code provided by Harald Sondergard for COMP90045.
    Provides function definitions for all functions implemented in
    order to create the pretty printer. 
-----------------------------------------------------------------------*/

#include <stdio.h>
#include "std.h"
#include "ast.h"

/*----------------------------------------------------------------------
    External Functions that will be accessed by other C files.  
-----------------------------------------------------------------------*/
void pretty_prog(FILE *fp, Program *prog);

// For reporting errors during static analysis
void print_expression(FILE *fp, Expr *expr, int prec);
void print_indents(FILE *fp, int indents);
void print_header(FILE *fp, Header *header);
void print_params(FILE *fp, Params *params);
void print_exprs(FILE *fp, Exprs *args);
