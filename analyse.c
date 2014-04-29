/* analyse.c */

/*-----------------------------------------------------------------------------
 * Developed by: #undef TEAMNAME
 * Performs static analysis on a Wiz program
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include "ast.h"
#include "symtable.h"
#include "helper.h"


/*-----------------------------------------------------------------------------
 * Function prototypes for internal functions
 *---------------------------------------------------------------------------*/
void *generate_symtables(Program *);
BOOL add_symtable_for_proc(void *, Proc *);
void dump_symtables(void *, Program *);


/*-----------------------------------------------------------------------------
 * Functions from header file
 *---------------------------------------------------------------------------*/

void
analyse_prog(Program *prog) {
    void *symtables = generate_symtables(prog);
    dump_symtables(symtables, prog);
}


/*-----------------------------------------------------------------------------
 * Internal functions
 *---------------------------------------------------------------------------*/

// Create the symtables for each of the procedures
void *
generate_symtables(Program *prog) {
    Procs *procs;
    void *tables = initialise_symtables();

    for (procs = prog->procedures; procs != NULL; procs = procs->rest) {
        if (!add_symtable_for_proc(tables, procs->first)) {
            Proc *proc = procs->first;
            fprintf(stderr, "[error]:%d: duplicate definition of proc %s\n",
                    proc->lineno, proc->header->id);
        }
    }

    return tables;
}

// Add a symtable for a proc. Retuns FALSE if there already exists a symtable
// with the same name (that is, proc name already used).
BOOL
add_symtable_for_proc(void *tables, Proc *proc) {
    void *table = create_symtable(tables, proc->header->id);
    symbol *new_sym;
    Params *params;
    Param *param;
    Decls *decls;

    if (table == NULL) {
        return FALSE; // symtable already exists!
    }

    // Add the parameters
    params = proc->header->params;
    while (params != NULL) {
        param = params->first;

        new_sym = checked_malloc(sizeof(symbol));
        new_sym->kind = param->ind == VAL_IND ? SYM_PARAM_VAL : SYM_PARAM_REF;
        new_sym->id = param->id;
        add_symbol(table, new_sym); // TODO: error check!

        params = params->rest;
    }

    // Add the declarations
    decls = proc->body->decls;
    while (decls != NULL) {
        new_sym = checked_malloc(sizeof(symbol));
        new_sym->kind = SYM_DECL;
        new_sym->id = decls->first->id;
        add_symbol(table, new_sym); // TODO: error check!

        decls = decls->rest;
    }

    return TRUE;
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
        fprintf(stderr, "SymTable for proc \"%s\":\n", proc_name);

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




