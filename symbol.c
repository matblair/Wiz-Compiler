/* symbol.c */

/*------------------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides a symbol table for use in semantic analysis and compilation
    of programs from the wiz languge to Oz machine code.

    This table makes use of our balanced binary tree implementation and contains
    any information required for quick analysis and optimisation as well as
    code generation.

    It is laid out as a table of tables, the first level representing scope
    (in this case procs) and then each sub table representing the symbols from
    each proc.

------------------------------------------------------------------------------*/
#include <string.h>
#include "symbol.h"
#include "bbst.h"
#include "helper.h"
#include "error_printer.h"
#include "analyse.h"


/*----------------------------------------------------------------------
    Internal structures.
-----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
    Internal function definitions.
-----------------------------------------------------------------------*/
// For finding things
int comp_scope(const void *a, const void *b);
int comp_symbol(const void *a, const void *b);
char *print_scope(const void *node);
char *print_symbol(const void *node);

void add_bounds_to_symbol(symbol *sym, Intervals *intvls);
void add_frames_to_stack(scope *t, int size);
void generate_scope(Proc *proc, sym_table *table);
void generate_params_symbols(Header *h, scope *sc, sym_table *prog);
void generate_decls_symbols(Decls *decls, scope *sc, sym_table *table);
SymType sym_type_from_ast_type(Type t);


/*----------------------------------------------------------------------
    Function implementations
-----------------------------------------------------------------------*/
// Simply allocates spaced for a symbol table, initialises the bbst
// and returns.
sym_table *
initialize_sym_table() {
    //We need to create a symbol table for scope
    sym_table *prog_sym = (sym_table *) checked_malloc(sizeof(sym_table));
    //Initialize the bst
    prog_sym->table = bbst_intialize();
    //Set as initialized
    prog_sym->initialised = TRUE;
    // Return table
    return prog_sym;
}

// Comparison function for use with the bbst, takes a key (our id) and 
// a scope and checks if they're < > or =
int
comp_scope(const void *key, const void *b) {
    if (b == NULL) {
        return 0;
    }

    char *id_a = (char *) key;
    scope *sb = (scope *) b;
    //Compare scope id
    char *id_b = sb->id;
    return (strcmp(id_a, id_b));
}


// Comparison function for use with the bbst, takes a key (our id) and 
// a symbol and checks if they're < > or =
int
comp_symbol(const void *key, const void *b) {
    if (b == NULL) {
        return 0;
    }
    char *id_a = (char *) key;
    symbol *symb = (symbol *) b;

    //Find the symbol id
    char *id_b = get_symbol_id(symb);
    return (strcmp(id_a, id_b));
}

// Function to create a scope, takes a table to insert the scope into, as well 
// as the parameters, function id and line number. Creates a scope object, 
// initialises all the correct values and then inserts it into the bbst for the
// root table.
scope *
create_scope(void *table, char *scope_id, void *p, int line_no) {
    //First check if the scope exists already
    scope *s = (scope *) bbst_find_node(scope_id, table, comp_scope);
    if (s != NULL) {
        //Already exists
        return NULL;
    }
    // Create the scope and insert the new symbol
    scope *new_scope = (scope *) checked_malloc(sizeof(scope));
    new_scope->table = bbst_intialize();
    new_scope->id = scope_id;
    new_scope->params = p;
    new_scope->line_no = line_no;
    new_scope->next_slot = 0;
    bbst_insert(table, scope_id, new_scope, comp_scope);
    return new_scope;
}

// Inserts a symbol into the given table, returning true or false for success
// or failure. Will only insert a symbol if unique (i.e. not yet inserted)
BOOL
insert_symbol(sym_table *prog, symbol *sym, scope *s) {
    //First check if the symbol exists, otherwise insert.
    symbol *exists = retrieve_symbol(get_symbol_id(sym), s->id, prog);
    if (exists != NULL) {
        //Symbol already exists
        return FALSE;
    } else {
        bbst_insert(s->table, get_symbol_id(sym), sym, comp_symbol);
        return TRUE;
    }
}

// Finds the id of a symbol depending on the type of the symbol itself.
// Quite straightforward implementation
char *
get_symbol_id(symbol *a) {
    if (a->kind == SYM_LOCAL) {
        //Then we have a decl
        Decl *d = (Decl *) a->sym_value;
        return d->id;
    } else {
        //We have a param
        Param *p = (Param *) a->sym_value;
        return p->id;
    }
}

// A wrapper function that will create the scope for a given procedure, then
// once created will call generate params and generate decls for the symbols
// in the parameters and declarations. Handles any errors that may arrise 
// during this creation
void
generate_scope(Proc *proc, sym_table *prog) {
    //Create the scope
    char *scope_id = proc->header->id;
    scope *s = create_scope(prog->table, scope_id,
                            proc->header->params, proc->header->line_no);

    if (s != NULL) {
        //Now go through and add all the params and internals
        generate_params_symbols(proc->header, s, prog);
        generate_decls_symbols(proc->body->decls, s, prog);
    } else {
        scope *s = find_scope(scope_id, prog);
        print_dupe_proc_errors(proc, s->params, s->line_no,
                               proc->header->line_no);

        setInvalid();
    }
}

// Function to wrap about the generate scope function. Will simply traverse
// the linked list of processes in a program and calls generate_scope for that
// process.
sym_table *
gen_sym_table(Program *prog) {
    //We walk through each proc, generating a symbol table for it
    sym_table *table = initialize_sym_table();
    //Get our procedures and generate each thing
    Procs *procs = prog->procedures;
    while (procs != NULL) {
        // Get the current proc
        Proc *current = procs->first;
        // Generate the scope for this proc
        generate_scope(current, table);
        // Continue along
        procs = procs->rest;
    }
    //dump_symbol_table(table);
    return table;
}

// Traverses the linked list for a set of declarations and for each declaration
// creates a new symbol, initialises all of the correct values and handles 
// geneartion of bounds if it is an array. Then tries to insert the symbol
// into a given scope. If it fails at any point it will print the appropriate
// warning message using the error printer.
void
generate_decls_symbols(Decls *decls, scope *sc, sym_table *prog) {
    while (decls != NULL) {
        // Get current param
        Decl *decl = decls->first;
        // Make a symbol
        symbol *s = checked_malloc(sizeof(symbol));
        s->kind = SYM_LOCAL;
        s->sym_value = decl;
        s->line_no = decl->lineno;
        s->slot = sc->next_slot;
        sc->next_slot++;
        s->type = sym_type_from_ast_type(decl->type);
        s->used = FALSE;
        s->bounds = NULL;
        Bound *bound;
        int frames;

        // create bounds if decl is an array, and add the extra frames needed
        if (decl->array != NULL) {
            add_bounds_to_symbol(s, decl->array);
            bound = s->bounds->first;
            frames = bound->offset_size * (bound->upper - bound->lower + 1);
            add_frames_to_stack((scope *) sc, frames - 1);
        }

        // Insert the symbol
        if (!insert_symbol(prog, s, sc)) {
            symbol *orig = retrieve_symbol(get_symbol_id(s), sc->id, prog);
            print_dupe_symbol_errors(get_symbol_id(s), get_type(orig),
                                     get_type(s), s->line_no);
            setInvalid();

        }
        //Contine along
        decls = decls->rest;
    }
}

// Function to add bounds to symbol (for arrays), traverses the 
// intervals and generates a bounds for each of them.
void
add_bounds_to_symbol(symbol *sym, Intervals *intvls) {
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
        offset = (bound->upper) - (bound->lower) + 1;
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

// Function to keep track of next_slot when generating arrays
void
add_frames_to_stack(scope *t, int size) {
    t->next_slot += size;
}

// Traverses the linked list parameters and for each parameter
// creates a new symbol, initialises all of the correct values.
// It then tries to insert the symbol into a given scope. If it fails at any 
// point it will print the appropriate warning message using the error printer.
void
generate_params_symbols(Header *h, scope *sc, sym_table *prog) {
    //We go through the params and add a symbol for each one.
    Params *params = h->params;
    int line_no = h->line_no;
    while (params != NULL) {
        // Get current param
        Param *p = params->first;
        // Make a symbol;
        symbol *s = checked_malloc(sizeof(symbol));
        if (p->ind == VAL_IND) {
            s->kind = SYM_PARAM_VAL;
        } else {
            s->kind = SYM_PARAM_REF;
        }
        s->type = sym_type_from_ast_type(p->type);
        s->slot = sc->next_slot;
        sc->next_slot++;
        s->sym_value = p;
        s->line_no = line_no;

        // Insert the symbol
        if (!insert_symbol(prog, s, sc)) {
            symbol *orig = retrieve_symbol(get_symbol_id(s), sc->id, prog);
            print_dupe_symbol_errors(get_symbol_id(s), get_type(orig),
                                     get_type(s), s->line_no);
            setInvalid();
        }
        //Contine along
        params = params->rest;
    }
}

// Uses the bbst functions to find a symbol from a given scope_id. Will return
// Null if either the scope or the symbol does not exist.
symbol *
retrieve_symbol(char *id, char *scope_id, sym_table *prog) {
    //First find the scope;
    scope *s = bbst_find_node(scope_id, prog->table, comp_scope);
    //If the scope is not null find try find the value, otherwise return null
    if (s != NULL) {
        return bbst_find_node(id, s->table, comp_symbol);
    } else {
        return NULL;
    }
}

// Retrieves a symbol from a given scope without having to find it first, will
// return NULL if it does not exist.
symbol *
retrieve_symbol_in_scope(char *id, scope *s) {
    //First check if the scope exists, otherwise insert.
    if (s != NULL) {
        symbol *sym = (symbol *) bbst_find_node(id, s->table, comp_symbol);
        return sym;
    } else {
        return NULL;
    }
}

// Finds a scope with a given ID from a program scopes table, returns NULL
// if it cannot be found.
scope *
find_scope(char *scope_id, sym_table *prog) {
    // For information hiding
    return (scope *) bbst_find_node(scope_id, prog->table, comp_scope);
}

// A function to return a string for a scope when printing. To be used with 
// the bbst print funciton.
char *
print_scope(const void *node) {
    scope *s = (scope *) node;
    return s->id;
}

// A function for debug to use map and print from bbst to print over a 
// scope and print appropriate information for it.
void
map_print_symbol(const void *node) {
    //Each node is a scope
    scope *s = (scope *) node;
    //Print the title
    fprintf(stderr, "Now printing the symbol tree for %s\n", s->id);
    bbst_dump_it(s->table, 0, print_symbol);
    //Print whitespace
    fprintf(stderr, "\n\n");
}

// A simple function to generate a string to print for a given symbol
// to be used with the bbst function printer.
char *
print_symbol(const void *node) {
    symbol *s = (symbol *) node;
    fprintf(stderr, "%s (%d)", get_symbol_id(s), s->slot);
    return get_symbol_id(s);
}

// A wrapper function that prints all of a symbol table using 
// the previously mentioned functions. For debug purposes.
void
dump_symbol_table(sym_table *prog) {
    //First print the scope tree
    fprintf(stderr, "The scope tree is as follows: \n");
    bbst_dump_it(prog->table, 0, print_scope);
    fprintf(stderr, "\n\n");
    //Now print each symbol tree for each scope
    bbst_map(prog->table, map_print_symbol);
}

// A function that will map a function over a symbol table. Used to preserve
// abstraction around the bbst layer if we were to change the implementation
// of this later.
void
map_over_symbols(void *sym_table, void (*map_func)(const void *node)) {
    //Wraps around bbst to preserve abstraction layer
    bbst_map(sym_table, map_func);
}

// Get the size of a symtable in terms of stack slots.
int
slots_needed_for_table(void *table) {
    scope *s = (scope *) table;
    return s->next_slot;
}

// Takes a type and converts from the ast type to a symbol 
// type to make life easier for code generation. 
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

// Function to return the ast type of a given symbol. Will check the 
// kind of symbol and then find the type appropriately.
Type
get_type(symbol *sym) {
    // Find the second type
    sym->used = TRUE;
    if (sym->kind == SYM_PARAM_VAL || sym->kind == SYM_PARAM_REF) {
        Param *p = (Param *) sym->sym_value;
        return p->type;
    } else {
        Decl *d = (Decl *) sym->sym_value;
        return d->type;
    }
}
