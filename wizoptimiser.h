/* wizoptimiser.h.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides function definitions and external access rights for 
	ozoptimiser.h
-----------------------------------------------------------------------*/
#include <stdio.h>
#include "std.h"
#include "ast.h"
/*----------------------------------------------------------------------
    Structures and enums needed from other files.  
-----------------------------------------------------------------------*/

Program* reduce_ast(Program *p);
Expr* reduce_expression(Expr *e);
