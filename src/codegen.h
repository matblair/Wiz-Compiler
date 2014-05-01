/* codegen.h */

/*-----------------------------------------------------------------------------
 * Developed by: #undef TEAMNAME
 * Compiles a Wiz program to Oz
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include "ast.h"

// Compiles a Wiz program to Oz, outputting to fp. Returns 0 for success.
int compile(FILE *fp, Program *prog, void *table);
