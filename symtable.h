/* symtable.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides functions for looking up the scope of variables for various
    procedures
-----------------------------------------------------------------------*/

#include "std.h"
#include "ast.H"

typedef enum {
    SYM_PARAM_VAL, SYM_PARAM_REF, SYM_LOCAL
} SymKind;

typedef enum {
    SYM_BOOL, SYM_REAL, SYM_INT
} SymType;

typedef struct {
    SymKind kind;
    SymType type;
    char    *id;
    int     slot;
} Symbol;

typedef struct ssymbols {
    Symbol *first;
    struct ssymbols *rest;
} Symbols;

// Create a new collection of symtables for a program
void *gen_symtables(Program *);

// Find a symtable, returns true if found
void *find_symtable(void *tables, char *key);

// Get the size of a symtable
int slots_needed_for_table(void *table);

// Find a symbol in a table, returns true if found
Symbol *find_symbol_by_id(void *table, char *id);

// Get a list of the the symbols that are params
Symbols *param_symbols_from_table(void *table);

// convert from AST type to sym type
SymType sym_type_from_ast_type(Type t);
