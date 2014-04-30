/* symbol.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides a symbol table for use in semantic analysis and compilation
    of programs from the wiz languge to Oz machine code.

    The table is laid out as follows

-----------------------------------------------------------------------*/
#include <string.h>
#include "symbol.h"
#include "balanced_bst.h"
#include "helper.h"

/*----------------------------------------------------------------------
    Internal structures.  
-----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
    Internal function definitions.  
-----------------------------------------------------------------------*/
#define TABLE_HEAD 0

// For finding things
int comp_scope(const void *a, const void *b);
int comp_symbol(const void *a, const void *b);
char* print_scope(const void *node);
char* print_symbol(const void *node);

/*----------------------------------------------------------------------
FUNCTIONS!!!! cOMMENT THIS LATER
-----------------------------------------------------------------------*/

sym_table* initialize_sym_table(){
	//We need to create a symbol table for scope
	sym_table *prog_sym = (sym_table *) checked_malloc(sizeof(sym_table));
	//Initialize the bst
	prog_sym->table = bbst_intialize();
	//Set as initialized
	prog_sym->initialised = TRUE;
	// Return table
	return prog_sym;
}

int comp_scope(const void *key, const void *b){
	if(b == NULL){
		return 0;
	}

	char *id_a = (char *) key;
	scope *sb = (scope *) b;
	//Compare scope id
    char *id_b = sb->id;
	return (strcmp(id_a, id_b));	
}

char* get_symbol_id(symbol *a){
	if(a->sym_type == SYM_LOCAL){
		//Then we have a decl
		Decl *d = (Decl *) a->sym_value;
		return d->id;
	} else {
		//We have a param
		Param *p = (Param *) a->sym_value;
		return p->id;
	}
}

int comp_symbol(const void *key, const void *b){
	if(b == NULL){
		return 0;
	}
	char *id_a = (char *) key;
	symbol *symb = (symbol *) b;	

	//Find the symbol id
	char *id_b = get_symbol_id(symb);
    return (strcmp(id_a, id_b));	
}

scope* create_scope(void *table, char *scope_id, void *p){
	//First check if the scope exists already
	scope *s = (scope *) bbst_find_node(scope_id, table, comp_scope);
	if(s != NULL){
		//Already exists
		return NULL;
	}
	// Create the scope and insert the new symbol
	scope *new_scope = (scope *) checked_malloc(sizeof(scope));
	new_scope->table = bbst_intialize();
	new_scope->id = scope_id;
	new_scope->params = p;
	bbst_insert(table, scope_id, new_scope, comp_scope);
	return new_scope;
}

BOOL insert_symbol(sym_table *prog, symbol *sym, scope *s){
	//First check if the symbol exists, otherwise insert.
	symbol *exists = retrieve_symbol(get_symbol_id(sym), s->id, prog);
	if(exists != NULL){
		//Symbol already exists
		return FALSE;
	} else {	
		bbst_insert(s->table, get_symbol_id(sym), sym, comp_symbol);
		return TRUE;
	}
}

symbol* retrieve_symbol(char *id, char *scope_id, sym_table *prog){
	//First find the scope;
	scope *s = bbst_find_node(scope_id, prog->table, comp_scope);
	//If the scope is not null find try find the value, otherwise return null
	if(s!=NULL){
		return bbst_find_node(id, s->table, comp_symbol);
	} else {
		return NULL;
	}
}

scope* find_scope(char *scope_id, sym_table *prog){
	// For information hiding
	return (scope *) bbst_find_node(scope_id, prog->table, comp_scope);
}


char* print_scope(const void *node){
	scope *s = (scope *) node;
	return s->id;
}

void map_print_symbol(const void *node){
	//Each node is a scope
	scope *s = (scope *) node;
	//Print the title
	fprintf(stderr,"Now printing the symbol tree for %s\n", s->id);
	bbst_dump_it(s->table, 0, print_symbol);
	//Print whitespace
	fprintf(stderr,"\n\n");
}

char* print_symbol(const void *node){
	symbol *s = (symbol *) node;
	return get_symbol_id(s);
}

void dump_symbol_table(sym_table *prog){
	//First print the scope tree
	fprintf(stderr,"The scope tree is as follows: \n");
	bbst_dump_it(prog->table, 0, print_scope);
	fprintf(stderr,"\n\n");
	//Now print each symbol tree for each scope
	bbst_map(prog->table, map_print_symbol);
}

void free_tree(sym_table *prog){
	// Also avoid this for now.
}