/* symbol.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides a symbol table for use in semantic analysis and compilation
    of programs from the wiz languge to Oz machine code.
-----------------------------------------------------------------------*/

#include "symbol.h"
#include "helper.h"
#include "uthash.h"

/*-----------------------------------------------------------------------
    Symbol table structure, which is a hash table; mapping a proc-name
    to an environment table (which itself provides mappings from
    variable name to variable information local to that environment),
    along with header of the proc 
-----------------------------------------------------------------------*/

typedef struct symbol_table {
    ETable          *e_table;
    Header          *header;
    UT_hash_handle  hh;  /* provides the magic to allow this structure to
                            be hashable AND be a hash-table at the same
                            time (see uthash.h) */
} STable

/*-----------------------------------------------------------------------
    Environment table structure, which is a hash table; mapping variable
    name to variable information (type, size, location, etc)
-----------------------------------------------------------------------*/

typedef struct environment_table {
    VarInfo         info;   /* contains information about variable */
    UT_hash_handle  hh;     /* see corresponding comment in STable
                               definition */
}

/*-----------------------------------------------------------------------
    creates a new symbol table, which is immediately returned, cast
    as a void pointer (for information hiding purposes)
    - takes as argument a pointer to a Procs object, which is used
    to initialise the environment tables for each procedure
-----------------------------------------------------------------------*/

void *create_new_symtable(Procs *procs) {
    STable *st, *entry, *found;
    st = NULL; //initialise symbol table to be empty
    //iterate over all procedures
    while (procs != NULL) {
        //get next proc, and advance list
        Proc p = procs->first;
        procs = procs->rest;
        //create new symbol table entry and populate its fields
        entry = (STable*) checked_malloc(sizeof(STable));
        entry->e_table = NULL;  //envirinment table initially empty
        entry->header = proc->header;
        //finally, have to actually add entry to table, checking
        //for possible duplicate (this is an error)
        HASH_FIND_STR(s_table, entry->header->id, found);
        if (found != NULL) {
            report_error_and_exit("duplicate proc name");
        }
        HASH_ADD_KEYPTR(hh, s_table, entry->header->id,
                strlen(proc->header->id), entry);
    }
    //finally return the table case as void*
    return (void*) st;
}

/*-----------------------------------------------------------------------
    adds a new entry to the given symbol table (given by symtable)
    - proc_name is name of proc we are adding variable information
      for (used to index symbol table and obtain correct environment
      table)
    - var_info contains all relevant information about the variable
      (including name of variable, which is used as the key)
-----------------------------------------------------------------------*/

void add_symtable_entry(void *symtable, char *proc_name, VarInfo *var_info) {
    STable *st, *st_entry;
    ETable *e_table, *found, new_entry;
    st = (STable*) symtable;
    //attempt to look-up the envirnment table... error if it does not exist
    //(this should never happen)
    HASH_FIND_STR(st, proc_name, st_entry);
    if (st_entry == NULL) {
        report_error_and_exit("attempted to add to symbol table"
                " using invalid procedure name");
    }
    //check for duplicate of variable name in environment table; if so
    //this is an error (means variable declared twice, or attempts to shadow
    //a parameter)
    e_table = st_entry->e_table;
    HASH_FIND_STR(e_table, var_info->id, found);
    if (found != NULL) {
        //indicates that entry with same variable id already exists
        report_error_and_exit("attempted to add an entry to symbol table"
                " wiith variable identifier shadowing existing entry\n"
                "(Check that all variable/parameter names within each proc"
                " are unique)");
    }
    //create new entry within environment table
    new_entry = (ETable*) checked_malloc(sizeof(ETable));
    new_entry->info = var_info;
    HASH_ADD_KEYPTR(hh, e_table, var_info->id,
            strlen(var_info->id), new_entry);
}

/*-----------------------------------------------------------------------
    looks up the symbol table (given by symtable), for the information
    relevant to variable with name "var_name" within procedure "proc_name",
    returning a pointer to a VarInfo object
    - returns NULL if variable does not exist within this environment
-----------------------------------------------------------------------*/

VarInfo *lookup_symtable(void *symtable, char *proc_name, char *var_name) {
    STable *st, *st_entry;
    ETable *e_table, *found_entry;
    st = (STable*) symtable;
    //attempt to look-up the envirnment table... error if it does not exist
    //(this should never happen)
    HASH_FIND_STR(st, proc_name, st_entry);
    if (st_entry == NULL) {
        report_error_and_exit("attempted to add to symbol table"
                " using invalid procedure name");
    }
    //now perform the lookup within environment table
    e_table = st_entry->e_table;
    HASH_FIND_STR(e_table, var_name, found_entry);
    if (found_entry == NULL) {
        //means entry does not exist in table
        return NULL;
    }
    //return pointer to the variable information
    return found_entry->info;
}

/*-----------------------------------------------------------------------
    convenience function for obtaining header of a given procedure from
    the symbol table
    - if the proc given by proc_name does not exist in the table,
      returns NULL
-----------------------------------------------------------------------*/

Header *get_proc_header(void *symtable, char *proc_name) {
    STable *st, st_entry;
    st = (STable*) symtable;
    //attempt to look-up the symbol table node for proc_name
    HASH_FIND_STR(st, proc_name, st_entry);
    if (st_entry == NULL) {
        //indicates entry does not exist, so reutrn NULL
        return NULL;
    }
    //otherwise reutrn pointer to the Header object
    return st_entry->header;
}
