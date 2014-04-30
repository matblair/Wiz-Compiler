/* analyse.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides static analysis of a given program in a 

-----------------------------------------------------------------------*/

#include "analyse.h"
#include "symbol.h"

/*-----------------------------------------------------------------------
    definitions of constants
-----------------------------------------------------------------------*/

#define MAIN_PROC "main"

/*-----------------------------------------------------------------------
    functions that perform semantic analysis on sub-programs at different
    levels (individual procedures, individual statements, individual
    expressions, etc)

    some return the augmented ast for the corresponding sub-tree
-----------------------------------------------------------------------*/

void analyze_proc(Proc *proc, void *symtable);

/*-----------------------------------------------------------------------
    performs semantic analysis  of the given program (given by prog)
    - throws an error and aborts if any of the static sematics of
      the wiz language are violated
    - if successful, edits the abstract syntax tree pointed to by
      prog with additional information useful for code generation
      (including types of expressions, extra nodes for casting, etc)
    - returns the symbol table generated during this process, which
      is also useful for code generation
-----------------------------------------------------------------------*/

void *semantic_analysis(Program *prog) {
    Procs *procs = prog->procedures;
    //create starting symbol table (passing this also ensures that there
    //are no duplicate procedure names)
    void *symtable = create_new_symtable(procs);
    //now check that main is defined
    Header *main_header = get_proc_header(symtable, MAIN_PROC);
    if (main_header == NULL) {
        report_error_and_exit("no 'main' procedure defined");
    }
    //we have passed semantics of overall function names at this point;
    //now need to check semantics of each function individually
    while (procs != NULL) {
        analyze_proc(procs->first, symtable);
        procs = procs->rest;
    }
    //finally, return populated symbol table
    return symtable;
}

/*-----------------------------------------------------------------------
    performs semantic analysis of given procedure "proc", while populating
    the symbol table with variable information determined, and augmenting
    the abstract syntax tree
-----------------------------------------------------------------------*/

Proc *analyze_proc(Proc *proc, void *symtable) {
    //first analyze each variable declaration in header and in
    //declarations list, and attempt to add to symbol table
    int *next_slot_number; //for keeping track of stack slots allocated
    *next_slot_number = 0;
    char *proc_name = proc->header->id;

    //first cover header parameters
    Params *params = proc->header->params;
    while (params != NULL) {
        Param *param = params->first;
        //parameters are never arrays, so never have an Intervals object
        allocate_new_variable(param->id, param->type, NULL, param->ind,
                next_slot_number, proc_name, symtable);
        //TODO implement above
        params = params->rest;
    }

    //now cover declarations
    Decls *decls = proc->body->decls;
    while (decls != NULL) {
        Decl *decl = decls->first;
        //give local variables the 'pass by value' indicator so that we
        //do not perform any referencing with them
        allocate_new_variable(decl->id, decl->type, decl->array, VAL_IND,
                next_slot_number, proc_name, symtable);
        //TODO implement above
        decls = decls->rest;
    }

    //finally analyse statements
    analyze_statements(proc->body->statements, proc_name, symtable);
    //TODO implement above
}
