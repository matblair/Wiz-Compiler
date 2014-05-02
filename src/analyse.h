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
//Put our value into a void pointer so that we don't have to let
//main know about our program.
void* analyse(Program *prog);
