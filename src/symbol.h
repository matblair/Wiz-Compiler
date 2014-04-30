/* symbol.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides function definitions and external access rights for 
	symbol.c
-----------------------------------------------------------------------*/
#include <stdio.h>
#include "std.h"
#include "ast.h"

/*----------------------------------------------------------------------
    Type containing the information associated with a variable in
    the symbol table; stuct used by external modules
-----------------------------------------------------------------------*/

typdef struct var_info {
    char        *id         /* name of the variable */
    Type        type;       /* type of the variable */
    Intervals   *bounds     /* Intervals object containing bounds of
                               array (only for array types) */
    int         size;       /* size of the variable in memory, in terms of
                               number of memory slots in Oz abstract machine */
    int         addr;       /* the slot number of the variables final stack
                               location (i.e. the starting slot of the
                               variable's relative location in memory, counting
                               from 0 from top of stack downwards) */
} VarInfo
    

/*----------------------------------------------------------------------
    External Functions that will be accessed by other C files.  
-----------------------------------------------------------------------*/

void *create_new_symtable(Procs *proc_list);

void add_symtable_entry(void *symtable, char *proc_name, VarInfo *var_info);

VarInfo *lookup_symtable(void *symtable, char *proc_name, char *var_name);
