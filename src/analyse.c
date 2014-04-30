/* analyse.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides static analysis of a given program in a 

-----------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include "analyse.h"
#include "symbol.h"
#include "helper.h"
#include "balanced_bst.h"

/*----------------------------------------------------------------------
    Internal structures.  
-----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
    Internal function definitions.  
-----------------------------------------------------------------------*/
void check_main(sym_table *table);
void analyse_statements(Stmts *statements, sym_table *table, char *scope_id);

sym_table*  gen_sym_table(Program *prog);
void generate_scope(Proc *proc, sym_table *table);
void generate_params_symbols(Header *h, scope *sc, sym_table *prog);
void generate_decls_symbols(Decls *decls, scope *sc, sym_table *table);

// For errors caught when generating table
void print_dupe_symbol_error(symbol *first, symbol *sym);

//For analysing thing
void analyse_assign(Assign *cond, sym_table *table, char *scope_id, int line_no);
void analyse_if(Cond *cond, sym_table *table, char *scope_id, int line_no);
void analyse_while(While *loop, sym_table *table, char *scope_id, int line_no);
void analyse_read(Expr *read, sym_table *table, char *scope_id, int line_no);
void analyse_write(Expr *write, sym_table *table, char *scope_id, int line_no);
void analyse_function(Function *function, sym_table *table, char *scope_id, int line_no);
void analyse_expression(Expr *expr, sym_table *table, char *scope_id, int line_no);

//Helper functions
char* get_type_string(symbol *sym);
int count_list(void *l);
Type get_expr_type(Expr *e, sym_table *table, char *scope_id, int line_no);
Type get_type(symbol *sym);
BOOL check_int_equiv(Type t);
BOOL check_float_equiv(Type t);
Type get_binop_type(Type t1, Type t2, BinOp b);
Type get_unop_type(Type t, UnOp u);
BOOL validate_array_dims(Exprs *indices, char *id, char *scope_id);

//Whether we succeed or not.
static BOOL isValid;

/*----------------------------------------------------------------------
FUNCTIONS!!!! cOMMENT THIS LATER
-----------------------------------------------------------------------*/

BOOL analyse(Program *prog){
	//Generate the symbol table
	isValid = TRUE;
	sym_table *table = gen_sym_table(prog);

	//Perform simple analysis
	check_main(table);

	//Analyse each proc
	Procs*procs = prog->procedures;
	while(procs!=NULL){
		Proc *p = procs->first;
		analyse_statements(p->body->statements, table, p->header->id);
		procs = procs->rest;
	}

	//dump_symbol_table(table);

	return isValid;
}


void check_main(sym_table *table){
	scope *m = find_scope("main", table);
	if(m==NULL){
		isValid = FALSE;
		fprintf(stderr,"Error: Program must contain a main function.\n");
	}
}

void analyse_statements(Stmts *statements, sym_table *table, char *scope_id){
	while(statements!=NULL){
		Stmt *statement = statements->first;

		StmtKind kind = statement->kind;
    	SInfo *info = &(statement->info);
    	int line_no = statement->lineno;

    	 // Switch on kind of statement and print appropriately
    	switch(kind) {
	        case STMT_ASSIGN: 
	            analyse_assign(&info->assign, table, scope_id, line_no);
	            break;

	        case STMT_COND:
	            analyse_if(&info->cond, table, scope_id, line_no);
	            break;

	        case STMT_READ:
	            analyse_read(info->read, table, scope_id, line_no);
	            break;

	        case STMT_WHILE:
	            analyse_while(&info->loop, table, scope_id, line_no);
	            break;

	        case STMT_WRITE: 
	            analyse_write(info->write,table, scope_id, line_no);
	            break;

	        case STMT_FUNC:
	            analyse_function(info->func, table, scope_id, line_no);
	            break;
	    }
		statements = statements->rest;	
	}
}


void generate_scope(Proc *proc, sym_table *prog){
	//Create the scope
	char *scope_id = proc->header->id;
	scope *s = create_scope(prog->table, scope_id, proc->header->params);

	if(s != NULL){
		//Now go through and add all the params and internals
		generate_params_symbols(proc->header, s, prog);
		generate_decls_symbols(proc->body->decls, s, prog);
	} else {
		scope *s = find_scope(scope_id, prog);
		fprintf(stderr, "Error: Duplicate proc %s, first declared on line %d, then on line %d.\n",
			scope_id, s->line_no, proc->header->line_no);
		isValid = FALSE;

	}
}

sym_table* gen_sym_table(Program *prog){

	//We walk through each proc, generating a symbol table for it 
	sym_table *table = initialize_sym_table();
	//Get our procedures and generate each thing
	Procs *procs = prog->procedures;
	while(procs!=NULL){
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

void generate_decls_symbols(Decls *decls, scope *sc, sym_table *prog){
	while(decls!=NULL){
		// Get current param
		Decl *d = decls->first;
		// Make a symbol
		symbol *s = checked_malloc(sizeof(symbol));
		s->sym_type = SYM_LOCAL;
		s->sym_value = d;
		s->line_no = d->lineno;
		// Insert the symbol
		if(!insert_symbol(prog, s, sc)){
			print_dupe_symbol_error(retrieve_symbol(get_symbol_id(s),sc->id,prog),s);
		}
		//Contine along
		decls = decls->rest;
	}
}

void generate_params_symbols(Header *h, scope *sc, sym_table *prog){
	//We go through the params and add a symbol for each one.
	Params *params = h->params;
	int line_no = h->line_no;
	while(params!=NULL){
		// Get current param
		Param *p = params->first;
		// Make a symbol
		symbol *s = checked_malloc(sizeof(symbol));
		s->sym_type = SYM_PARAM;
		s->sym_value = p;
		s->line_no = line_no;

		// Insert the symbol
		insert_symbol(prog, s, sc);
		//Contine along
		params = params->rest;
	}	
}

void print_dupe_symbol_error(symbol *first, symbol *sym){
	// Find the types
	// Find the first type
	char *first_type = get_type_string(first);
	// Find the second type
	char *second_type = get_type_string(sym);

	fprintf(stderr, "Error: Duplicate symbol %s, first declared as a %s on line\
	 	%d, then as a %s on line %d.\n", get_symbol_id(sym), first_type,
	 	first->line_no, second_type, sym->line_no);

	free((void *) first_type);
	free((void *) second_type);
	isValid = FALSE;

}

void analyse_assign(Assign *a, sym_table *table, char *scope_id, int line_no){
	//Get the type of the left and right analysis then ensure they match.
	Type left = get_expr_type(a->asg_ident, table, scope_id, line_no);
	Type right = get_expr_type(a->asg_expr, table, scope_id, line_no);
	//Check they are equal, if they are, great otherwise that's shit.

}

void analyse_if(Cond *cond, sym_table *table, char *scope_id, int line_no){
	//Analyse condition

	//Analyse then branch
	analyse_statements(cond->then_branch, table, scope_id);

	//Analyse else branch
	analyse_statements(cond->else_branch, table, scope_id);

}

void analyse_while(While *loop, sym_table *table, char *scope_id, int line_no){
	//Analyse condition

	//Analyse statement
	analyse_statements(loop->body, table, scope_id);
}

void analyse_read(Expr *read, sym_table *table, char *scope_id, int line_no){

}

void analyse_write(Expr *write, sym_table *table, char *scope_id, int line_no){

}

void analyse_function(Function *f, sym_table *prog, char *scope_id, int line_no){
	//Find the scope of the function
	scope *called = find_scope(f->id, prog);
	Params *fcallee = (Params *) called->params;
	Exprs *fcaller = f->args;
	//Check number of variables
	int call_no = count_list((void *)fcaller);
	int expect_no = count_list((void *)fcallee);
	if(call_no!=expect_no){
		fprintf(stderr,"Error: Incorrect parameters for function call %s on\
		 	line %d, expected %d, received %d.\n",f->id, line_no, expect_no,
		 	call_no);

		isValid = FALSE;
	}
	
	//Go through each variable and try match. We know these are the same
	//lenght now.
	int par_num = 1;
	while(fcaller!=NULL){
		Expr *e = fcaller->first;
		Param *p = fcallee->first;
		Type caller = get_expr_type(e, prog, scope_id, line_no);
		if(caller != p->type && caller != INVALID_TYPE){
		 	fprintf(stderr,"Error: Incorrect parameters for function call %s on\
		 	 	line %d, expected %s, received %s.\n",f->id, 
		 	 	line_no, typenames[caller], typenames[p->type]);
			isValid = FALSE;
		}
		par_num ++;
		fcaller = fcaller->rest;
		fcallee = fcallee->rest;
	}
}

char* get_type_string(symbol *sym){
	// Find the second type
	char *type = checked_malloc(16 * sizeof(char));
	if(sym->sym_type == SYM_PARAM){
		Param *p = (Param *) sym->sym_value;
		sprintf(type, "%s parameter", typenames[p->type]);
	} else {
		Decl *d = (Decl *) sym->sym_value;
		sprintf(type, "%s variable", typenames[d->type]);
	}
	return type;
}

Type get_expr_type(Expr *e, sym_table *table, char *scope_id, int line_no){
	//Find the type of an expression
    ExprKind kind = e->kind;
    // Switch on kind to print
    switch(kind) {
        case EXPR_ID:
            //Find the symbol and get it's type
        	if(retrieve_symbol(e->id, scope_id, table)){
        		return get_type(retrieve_symbol(e->id, scope_id, table));
        	} else {
        		fprintf(stderr,"Error: Undefined variable %s on line %d.\n", 
        			e->id, line_no);
        		isValid = FALSE;
        		return INVALID_TYPE;
        	}
            break;
        case EXPR_CONST:
        	return  e->constant.type;
            break;
        case EXPR_BINOP:
        	//We need the types of each expression
        	//Then we determine the type if allowed using the binop
           	return get_binop_type(get_expr_type(e->e1, table, scope_id,
           	 line_no),get_expr_type(e->e2, table, scope_id, line_no),e->binop);
            break;
        case EXPR_UNOP:
        	//Get the type of the expression
        	//Then we determine the type if allowed using the binop
            return get_unop_type(get_expr_type(e->e1, table, scope_id, 
            	line_no),e->unop);
            break;
        case EXPR_ARRAY:
        	//We need to check array dimensions, then check all expressions are
        	//int equiv.
        	if(validate_array_dims(e->indices, e->id, scope_id)){

        	} else {
        		symbol *a = retrieve_symbol(e->id, scope_id, table);
        		Decl *d = (Decl *) a->sym_value;
        		fprintf(stderr,"Error: Incorrect array dimensions for %s on \
        			line %d, expected %d, actual %d.\n", e->id, line_no, 
        			count_list(d->array), count_list(e->indices));
        	}
            return BOOL_TYPE;
            break;
    }
    
    return INVALID_TYPE;
}

Type get_binop_type(Type t1, Type t2, BinOp b){
	return INT_TYPE;
}

Type get_unop_type(Type t, UnOp u){
	return INT_TYPE;
}

BOOL check_int_equiv(Type t){
	return FALSE;
}

BOOL check_float_equiv(Type t){
	return FALSE;
}

BOOL validate_array_dims(Exprs *indices, char *id, char *scope_id){
	return FALSE;
}

Type get_type(symbol *sym){
	// Find the second type
	if(sym->sym_type == SYM_PARAM){
		Param *p = (Param *) sym->sym_value;
		return p->type;
	} else {
		Decl *d = (Decl *) sym->sym_value;
		return d->type;
	}
}

int count_list(void *l){
	int i=0;

	//Given our ast, all lists that follow the structure 
	// 	struct name {
	//		type *first
	//		type *rest
	//	}
	// are equivalent in size so we can use any to count them.

	Procs *p = (Procs *) l;
	while(p!=NULL){
		i++;
		p = p->rest;
	}
	return i;
}



