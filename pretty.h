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
