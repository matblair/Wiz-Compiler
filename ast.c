/* ast.c */
/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Definitions for abstract syntax trees generated for Wiz programs,
    based of scaffolding code provided by Harald Sondergard for COMP90045
    Original comment as follows:

    Definitions for abstract syntax trees generated for Iz programs.
    (The bulk of these can be found in the header file ast.h).
    For use the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include    "ast.h"

/* The pretty-printer will need to know how to print binary operators: */
const char  *binopname[] = { BINOP_NAMES };
/* As well as knowing how to print unary operators: */
const char  *unopname[]  = { UNOP_NAMES };
/* And will need to know how to print the names of types and values: */
const char  *valnames[]  = { VAL_NAMES };
const char  *typenames[]  = { TYPE_NAMES };

/*
   The pretty printer must also have knowledge of the precedence levels
   for both binary and unary operations to print the smallest number
   of brackets
*/
const int  binopprec[] = { BINOP_PRECEDENCE };
const int  unopprec[] = { UNOP_PRECEDENCE };



