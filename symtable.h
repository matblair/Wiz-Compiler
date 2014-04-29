/* symtable.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides functions for looking up the scope of variables for various
    procedures
-----------------------------------------------------------------------*/

#include "std.h"

typedef enum {
    SYM_PARAM_VAL, SYM_PARAM_REF, SYM_DECL
} SymKind;

typedef struct {
    SymKind kind;
    char    *id;
    int     slot;
} symbol;

// Create a new collection of symtables
void *initialise_symtables(void);

// Create a new symtable, and return it
void *create_symtable(void *tables, char *key);

// Find a symtable, returns true if found
void *find_symtable(void *tables, char *key);

// Insert a symbol into a symtable. Returns false already exists
BOOL add_symbol(void *table, symbol *sym);

// Find a symbol in a table, returns true if found
symbol *find_symbol_by_id(void *table, char *id);

// Debug printing
void dump_symbol_for_id(void *table, char *id);
