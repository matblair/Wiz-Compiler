/* symtable.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Implements functions for looking up the scope of variables for various
    procedures
-----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include "symtable.h"
#include "ast.h"
#include "bbst.h"
#include "std.h"
#include "helper.h"

typedef struct {
    int     next_slot;
    void    *symbols_bbst;
    Symbols *params;
    Symbols *params_end;
} symtable;

/*-----------------------------------------------------------------------------
 * Function prototypes for internal functions
 *---------------------------------------------------------------------------*/

symtable *new_symtable(void);
int next_stackslot(symtable *);
void add_frames_to_stack(symtable *, int);
BOOL add_symbol(void *table, Symbol *sym);
void *create_symtable(void *tables, char *key);
BOOL add_symtable_for_proc(void *, Proc *);
void add_params_to_table(void *table, Params *params);
void add_decls_to_table(void *table, Decls *decls);
void add_bounds_to_symbol(Symbol *sym, Intervals *intvls);

void dump_symtables(void *, Program *);
void dump_symbol_for_id(void *table, char *id);


/*-----------------------------------------------------------------------------
 * Functions from header file
 *---------------------------------------------------------------------------*/

 // Create the symtables for each of the procedures
void *
gen_symtables(Program *prog) {
    Procs *procs;
    void *tables = bbst_initialise();

    for (procs = prog->procedures; procs != NULL; procs = procs->rest) {
        if (!add_symtable_for_proc(tables, procs->first)) {
            Proc *proc = procs->first;
            fprintf(stderr, "[error]:%d: duplicate definition of proc %s\n",
                    proc->lineno, proc->header->id);
        }
    }

    // Print out debugging info
    #ifdef DEBUG
    dump_symtables(tables, prog);
    #endif

    return tables;
}

// Get the size of a symtable
int
slots_needed_for_table(void *table) {
    symtable *t = (symtable *) table;
    return t->next_slot;
}

// Find a symtable by key
void *
find_symtable(void *tables, char *key) {
    return bbst_find(tables, key);
}

// Find a symbol by its id name
Symbol *
find_symbol_by_id(void *table, char *id) {
    symtable *t = (symtable *) table;
    return (Symbol *) bbst_find(t->symbols_bbst, id);
}

Symbols *
param_symbols_from_table(void *table) {
    symtable *t = (symtable *) table;
    return t->params;
}

SymType
sym_type_from_ast_type(Type t) {
    switch (t) {
        case BOOL_TYPE:
            return SYM_BOOL;

        case INT_TYPE:
            return SYM_INT;

        case FLOAT_TYPE:
            return SYM_REAL;

        default:
            report_error_and_exit("invalid type for symbol!");
            return -1; //can't get here, but otherwise gcc complains
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
    table->params = NULL;
    table->params_end = NULL;

    return table;
}

// Find the next available stack slot
int
next_stackslot(symtable *t) {
    return t->next_slot++;
}

// consume more of the slots, for arrays
void
add_frames_to_stack(symtable *t, int size) {
    t->next_slot += size;
}

// Insert a new sybmol into the symbol table. Returns FALSE if symbol already
// exists for that key. Sets the stack slot for you!
BOOL
add_symbol(void *table, Symbol *sym) {
    // Convert to internal representation
    symtable *t = (symtable *)table;

    if (find_symbol_by_id(table, sym->id) != NULL) {
        return FALSE;
    }

    bbst_insert(t->symbols_bbst, sym->id, sym);
    return TRUE;
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

// Add a symtable for a proc. Retuns FALSE if there already exists a symtable
// with the same name (that is, proc name already used).
BOOL
add_symtable_for_proc(void *tables, Proc *proc) {
    void *table = create_symtable(tables, proc->header->id);

    if (table == NULL) {
        return FALSE; // symtable already exists!
    }

    // Add the parameters
    add_params_to_table(table, proc->header->params);
    
    // Add the declarations
    add_decls_to_table(table, proc->body->decls);

    return TRUE;
}

// Add a linked list of params as symbols
void
add_params_to_table(void *table, Params *params) {
    Param *param;
    Symbol *new_sym;
    Symbols *new_syms;
    symtable *t = (symtable *) table;

    while (params != NULL) {
        param = params->first;

        // Create the new symbol
        new_sym = checked_malloc(sizeof(Symbol));
        new_sym->kind = param->ind == VAL_IND ? SYM_PARAM_VAL : SYM_PARAM_REF;
        new_sym->id = param->id;
        new_sym->slot = next_stackslot(table);
        new_sym->type = sym_type_from_ast_type(param->type);
        new_sym->bounds = NULL;
        add_symbol(table, new_sym);

        // Store the new symbol at the end of the params list
        // creates params list if needed
        new_syms = checked_malloc(sizeof(Symbols));
        new_syms->first = new_sym;
        new_syms->rest = NULL;

        if (t->params_end != NULL) {
            t->params_end->rest = new_syms;
        } else {
            t->params = new_syms;
        }

        t->params_end = new_syms;

        params = params->rest;
    }
}

// Add a linked list of decls as symbols
void
add_decls_to_table(void *table, Decls *decls) {
    Symbol *new_sym;
    Decl *decl;
    int frames;
    Bound *bound;

    while (decls != NULL) {
        decl = decls->first;

        // Create the new symbol
        new_sym = checked_malloc(sizeof(Symbol));
        new_sym->kind = SYM_LOCAL;
        new_sym->id = decl->id;
        new_sym->slot = next_stackslot(table);
        new_sym->type = sym_type_from_ast_type(decl->type);
        new_sym->bounds = NULL;
        add_symbol(table, new_sym);

        // create bounds if decl is an array, and add the extra frames needed
        if (decl->array != NULL) {
            add_bounds_to_symbol(new_sym, decl->array);
            bound = new_sym->bounds->first;
            frames = bound->offset_size * (bound->upper - bound->lower + 1);
            add_frames_to_stack((symtable *) table, frames - 1);
        }

        decls = decls->rest;
    }
}

// add bounds to symbol (for arrays)
void
add_bounds_to_symbol(Symbol *sym, Intervals *intvls) {
    if (intvls == NULL) {
        sym->bounds = NULL;
        return; // no more intervals to store!
    }

    // calculate the rest of the bounds
    add_bounds_to_symbol(sym, intvls->rest);

    Interval *intvl;
    Bounds *bounds;
    Bound *bound;
    int offset;

    // get the interval we're working with
    intvl = intvls->first;

    if (sym->bounds != NULL) {
        bound = sym->bounds->first;
        offset = bound->upper - bound->lower;
        offset *= bound->offset_size;
    } else {
        offset = 1;
    }

    // create new bound struct
    bound = checked_malloc(sizeof(Bound));
    bound->lower = intvl->lower;
    bound->upper = intvl->upper;
    bound->offset_size = offset;

    // add to linked list
    bounds = checked_malloc(sizeof(Bounds));
    bounds->rest = sym->bounds;
    bounds->first = bound;
    sym->bounds = bounds;
}


/*-----------------------------------------------------------------------------
 * Debug functions
 *---------------------------------------------------------------------------*/

// Print out the symtables generated for a program
void
dump_symtables(void *tables, Program *prog) {
    void *table;
    Procs *procs;
    Proc *proc;
    Params *params;
    Decls *decls;
    char *proc_name;

    for (procs = prog->procedures; procs != NULL; procs = procs->rest) {
        // Find the proc
        proc = procs->first;
        proc_name = proc->header->id;
        table = find_symtable(tables, proc_name);
        fprintf(stderr, "SymTable for proc \"%s\" (%d slots):\n",
                proc_name, slots_needed_for_table((void *) table));

        // For the parameters
        params = proc->header->params;
        while (params != NULL) {
            dump_symbol_for_id(table, params->first->id);
            params = params->rest;
        }

        // For the declarations
        decls = proc->body->decls;
        while (decls != NULL) {
            dump_symbol_for_id(table, decls->first->id);
            decls = decls->rest;
        }

        // Spacing
        fprintf(stderr, "\n");
    }
}

// Dump the information for a symbol
void
dump_symbol_for_id(void *table, char *id) {
    Symbol *sym = find_symbol_by_id(table, id);
    char *symnames[] = { "bool", "real", "int" };

    if (sym == NULL) {
        fprintf(stderr, "%10s => not in scope!\n", id);

    } else {
        switch(sym->kind) {
            case SYM_PARAM_VAL:
                fprintf(stderr, "%10s => param (val) %4s, slot %02d\n",
                        id, symnames[sym->type], sym->slot);
                break;

            case SYM_PARAM_REF:
                fprintf(stderr, "%10s => param (ref) %4s, slot %02d\n",
                        id, symnames[sym->type], sym->slot);
                break;

            case SYM_LOCAL:
                fprintf(stderr, "%10s => local       %4s, slot %02d\n",
                        id, symnames[sym->type], sym->slot);
                break;
        }
    }
}

