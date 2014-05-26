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
    Structures and enums needed from other files.
-----------------------------------------------------------------------*/

typedef enum {
    SYM_LOCAL, SYM_PARAM_VAL, SYM_PARAM_REF
} SymbolKind;

typedef enum {
    SYM_BOOL, SYM_REAL, SYM_INT
} SymType;

typedef struct {
    int lower;
    int upper;
    int offset_size;
} Bound;

typedef struct bbounds {
    Bound *first;
    struct bbounds *rest;
} Bounds;

typedef struct symbol_data {
    SymbolKind  kind;
    SymType     type;
    void        *sym_value;
    int         line_no;
    int         slot;
    BOOL        used;
    Bounds  *bounds;
} symbol;

typedef struct scope_data {
    char *id;
    void *table;
    void *params;
    int line_no;
    int next_slot;
} scope;


typedef struct symbol_table {
    void *table;
    BOOL initialised;
} sym_table;

/*----------------------------------------------------------------------
    External Functions that will be accessed by other C files.
-----------------------------------------------------------------------*/
// For generating table
sym_table *initialize_sym_table();

BOOL insert_symbol(sym_table *prob, symbol *sym, scope *s);

scope *create_scope(void *table, char *scope_id, void *p, int line_no);

// For finding
symbol *retrieve_symbol(char *id, char *scope_id, sym_table *prog);
symbol *retrieve_symbol_in_scope(char *id, scope *s);
scope *find_scope(char *scope_id, sym_table *prog);


// Get the size of a symtable
int slots_needed_for_table(void *table);

// Get the name for a symbol (for error reporting purposes)
char *get_symbol_id(symbol *a);

// Map a function over all symbols in a the symbol table, in all scopes.
void map_over_symbols(void *sym_table, void (*map_func)(const void *node));

// Diagnostics
void dump_symbol_table(sym_table *prog);

// To remove tree
void free_tree(sym_table *prog);

sym_table *gen_sym_table(Program *prog);
Type get_type(symbol *sym);
