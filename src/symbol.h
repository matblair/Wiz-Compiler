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
    SYM_LOCAL, SYM_PARAM
} SymbolKind;

typedef struct scope_data {
	char *id;
	void *table;
	void *params;
	int line_no;
} scope;

typedef struct symbol_data{
	SymbolKind 	sym_type;
	void 	 	*sym_value;
	int 		line_no;
	int         stack_slot;
} symbol;

typedef struct symbol_table {
	void *table;
	BOOL initialised;
} sym_table;

/*----------------------------------------------------------------------
    External Functions that will be accessed by other C files.  
-----------------------------------------------------------------------*/
// For generating table
sym_table* initialize_sym_table();
BOOL 		insert_symbol(sym_table *prob, symbol *sym, scope *s);
scope* 		create_scope(void *table, char *scope_id, void *p);
symbol*    	retrieve_symbol(char *id, char *scope_id, sym_table *prog);
scope*      find_scope(char *scope_id, sym_table *prog);
void 		dump_symbol_table(sym_table *prog);
void 		free_tree(sym_table *prog);
char* get_symbol_id(symbol *a);
