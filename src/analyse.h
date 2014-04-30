/* analyse.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides function definitions and external access rights for 
	symbol.c
-----------------------------------------------------------------------*/
#include <stdio.h>
#include "std.h"
#include "ast.h"

/*----------------------------------------------------------------------
    External Functions that will be accessed by other C files.  
-----------------------------------------------------------------------*/

BOOL analyse(Program *prog);