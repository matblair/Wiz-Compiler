/* codegen.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides function definitions and external access rights for 
	codegen.c
-----------------------------------------------------------------------*/
#include <stdio.h>
#include "std.h"
#include "ast.h"

/*----------------------------------------------------------------------
    External Functions that will be accessed by other C files.  
-----------------------------------------------------------------------*/

void generate_code(Program *prog);