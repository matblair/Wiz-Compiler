/* symtable.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Implements functions for looking up the scope of variables for various
    procedures
-----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "symtable.h"
#include "bbst.h"
#include "std.h"
#include "helper.h"

typedef struct {
    int next_slot;
    void *symbols_bbst;
} symtable;

/*-----------------------------------------------------------------------------
 * Function prototypes for internal functions
 *---------------------------------------------------------------------------*/

// Create a new symtable
symtable *new_symtable();
int next_stackslot(symtable *);


/*-----------------------------------------------------------------------------
 * Functions from header file
 *---------------------------------------------------------------------------*/

void *
initialise_symtables() {
    return bbst_initialise();
}

// Create a symtable for a key. Returns NULL if the key exists already.
void *
create_symtable(void *tables, char *key) {
    void *table;

    // See if it exists already
    if (find_symtable(tables, key) != NULL) {
        return NULL;
    }

    // Need a new symbol table!
    table = new_symtable();
    bbst_insert(tables, key, table);
    return (void *) table;
}

// Find a symtable by key
void *
find_symtable(void *tables, char *key) {
    return bbst_find(tables, key);
}

// Insert a new sybmol into the symbol table. Returns FALSE if symbol already
// exists for that key. Sets the stack slot for you!
BOOL
add_symbol(void *table, symbol *sym) {
    // Convert to internal representation
    symtable *t = (symtable *)table;

    if (find_symbol_by_id(table, sym->id) != NULL) {
        return FALSE;
    }

    sym->slot = next_stackslot(t);
    bbst_insert(t->symbols_bbst, sym->id, sym);
    return TRUE;
}

// Find a symbol by its id name
symbol *
find_symbol_by_id(void *table, char *id) {
    symtable *t = (symtable *) table;
    return (symbol *) bbst_find(t->symbols_bbst, id);
}

// Dump the information for a symbol
void
dump_symbol_for_id(void *table, char *id) {
    symbol *sym = find_symbol_by_id(table, id);

    if (sym == NULL) {
        fprintf(stderr, "%10s => not in scope!\n", id);

    } else {
        switch(sym->kind) {
            case SYM_PARAM_VAL:
                fprintf(stderr, "%10s => param (val), slot %02d\n",
                        id, sym->slot);
                break;

            case SYM_PARAM_REF:
                fprintf(stderr, "%10s => param (ref), slot %02d\n",
                        id, sym->slot);
                break;

            case SYM_DECL:
                fprintf(stderr, "%10s => decl,        slot %02d\n",
                        id, sym->slot);
                break;
        }
    }
}


/*-----------------------------------------------------------------------------
 * Internal functions
 *---------------------------------------------------------------------------*/

// Create a new symtable
symtable *
new_symtable() {
    symtable *table = checked_malloc(sizeof(symtable));
    table->next_slot = 0;
    table->symbols_bbst = bbst_initialise();
    return table;
}

// Find the next available stack slot
int
next_stackslot(symtable *t) {
    return t->next_slot++;
}

