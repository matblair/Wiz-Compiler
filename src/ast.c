/* ast.c */

/*-----------------------------------------------------------------------
    Definitions for abstract syntax trees generated for Iz programs.
    (The bulk of these can be found in the header file ast.h).
    For use the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include    "ast.h"

/* The pretty-printer will need to know how to print operators: */

const char  *binopname[] = { BINOP_NAMES };
const char  *unopname[]  = { UNOP_NAMES };

