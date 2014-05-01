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
#include "error_printer.h"

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

//For analysing thing
void analyse_assign(Assign *cond, sym_table *table,
     char *scope_id, int line_no);
void analyse_if(Cond *cond, sym_table *table, char *scope_id, int line_no);
void analyse_while(While *loop, sym_table *table, char *scope_id, int line_no);
void analyse_read(Expr *read, sym_table *table, char *scope_id, int line_no);
void analyse_write(Expr *write, sym_table *table, char *scope_id, int line_no);
void analyse_function(Function *function, sym_table *table, 
     char *scope_id, int line_no);
void analyse_expression(Expr *expr, sym_table *table, 
     char *scope_id, int line_no);
void check_unused_symbols(sym_table *table, Program *p);

//Helper functions
char* get_type_string(symbol *sym);
int count_list(Exprs *l);
int count_params(Params *l);
int count_array(Intervals *l);
Type get_expr_type(Expr *e, Expr *parent,
    sym_table *table, char *scope_id, int line_no);
Type get_type(symbol *sym);
Type get_const_type(Expr *e);
BOOL check_int_equiv(Type t);
BOOL check_float_equiv(Type t);
Type get_binop_type(Type t1, Type t2, BinOp b, int line_no, Expr *e);
Type get_unop_type(Type t, UnOp u, int line_no, Expr *e);
BOOL validate_array_dims(Expr *e, char *scope_id, 
    sym_table *table, int line_no);
void validate_array_indices(Exprs *indices, char *id,
    int line_no, sym_table *table, char *scope_id, symbol *array_sym);

//Whether we succeed or not.
static BOOL isValid;

/*----------------------------------------------------------------------
FUNCTIONS!!!! cOMMENT THIS LATER
-----------------------------------------------------------------------*/

BOOL analyse(Program *prog){
    //Generate the symbol table
    isValid = TRUE;
     sym_table *table = gen_sym_table(prog);

    //Analyse each proc
    Procs*procs = prog->procedures;
    while(procs!=NULL){
        Proc *p = procs->first;
        analyse_statements(p->body->statements, table, p->header->id);
        procs = procs->rest;
    }

    //For debug purposes;
    #ifdef DEBUG
    dump_symbol_table(table);
    #endif

    //Check for unused symbols
    check_unused_symbols(table, prog);

    //Perform simple analysis
    check_main(table);

    return TRUE;
}

void report_unused_symbols(const void *node){
    symbol *s = (symbol *) node;
    if(!(s->used)){
        print_unused_symbol_error(get_symbol_id(s), s->line_no);
    }
}

void check_unused_symbols(sym_table *table, Program *prog){
    //For each proc get it's symbol table, then for each symbol    
    //Map against all symbols and check if used
    Procs*procs = prog->procedures;
    while(procs!=NULL){
        Proc *p = procs->first;
        //Get the scope 
        scope *s = find_scope(p->header->id, table);

        //Now we want to map over the scope and print any duplicate errors.
        map_over_symbols(s->table, report_unused_symbols);
        procs = procs->rest;
    }
}

void check_main(sym_table *table){
    scope *m = find_scope("main", table);
    if(m==NULL){
        print_missing_main_error();
        isValid = FALSE;
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
    scope *s = create_scope(prog->table, scope_id, 
        proc->header->params, proc->header->line_no);

    if(s != NULL){
        //Now go through and add all the params and internals
        generate_params_symbols(proc->header, s, prog);
        generate_decls_symbols(proc->body->decls, s, prog);
    } else {
        scope *s = find_scope(scope_id, prog);
        print_dupe_proc_errors(proc, s->params, s->line_no, 
            proc->header->line_no);

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
        s->used = FALSE;
        // Insert the symbol
        if(!insert_symbol(prog, s, sc)){
            symbol *orig = retrieve_symbol(get_symbol_id(s), sc->id,prog);
            print_dupe_symbol_errors(get_symbol_id(s), get_type(orig), 
                get_type(s), s->line_no, orig->line_no);
            
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

void analyse_assign(Assign *a, sym_table *table, char *scope_id, int line_no){
    //Get the type of the left and right analysis then ensure they match.
    Type left = get_expr_type(a->asg_ident, NULL, table, scope_id, line_no);
    Type right = get_expr_type(a->asg_expr, NULL, table, scope_id, line_no);
    
    //Check they are both valid and equivalent 
    if(left == INVALID_TYPE || right == INVALID_TYPE){
        return;
    }
    //We can assign ints to floats but not the other way around
    if(left == right || (left == FLOAT_TYPE && right == INT_TYPE)){
        //All good, return
        return;
    } else {
        //Not good. Let the user know
        print_assign_error( a,left,right, line_no);
        isValid = FALSE;
    }
}

void analyse_if(Cond *ifcond, sym_table *table, char *scope_id, int line_no){
    //Analyse condition
    Type c = get_expr_type(ifcond->cond, NULL, table, scope_id, line_no);
    if(c != BOOL_TYPE){
        print_if_error(ifcond->cond, c, line_no);
        isValid = FALSE;
    }
    //Analyse then branch
    analyse_statements(ifcond->then_branch, table, scope_id);
    //Analyse else branch
    analyse_statements(ifcond->else_branch, table, scope_id);

}

void analyse_while(While *loop, sym_table *table, char *scope_id, int line_no){
    //Analyse condition
    Type c = get_expr_type(loop->cond, NULL, table, scope_id, line_no);
    if(c != BOOL_TYPE){
        print_while_error(loop->cond, c, line_no);
        isValid = FALSE;
    }
    //Analyse statement
    analyse_statements(loop->body, table, scope_id);
}

void analyse_read(Expr *read, sym_table *table, char *scope_id, int line_no){
    //We just need to check the type is valid, so we try get the symbol and if
    //we fail then we can't read.
    get_expr_type(read, NULL, table, scope_id, line_no);
}

void analyse_write(Expr *write, sym_table *table, char *scope_id, int line_no){
    //We just need to check the type is valid or that it is a constant,
    // so we try get the symbol and if we fail then we can't write.
    if(write->kind != EXPR_CONST){
         get_expr_type(write, NULL, table, scope_id, line_no);
    }
}

void 
analyse_function(Function *f, sym_table *prog, char *scope_id, int line_no){
    //Find the scope of the function
    scope *called = find_scope(f->id, prog);
    Params *fcallee = (Params *) called->params;
    Exprs *fcaller = f->args;
    //Check number of variables
    int call_no = count_list((void *)fcaller);
    int expect_no = count_list((void *)fcallee);

    if(call_no!=expect_no){
        print_func_pmismatch_error(f, fcallee, line_no, expect_no, call_no);
        isValid = FALSE;
    } else {
        //Go through each variable and try match. We know these are the same
        //lenght now.
        int par_num = 1;
        while(fcallee!=NULL){
            Expr *e = fcaller->first;
            Param *p = fcallee->first;
            Type caller = get_expr_type(e, NULL, prog, scope_id, line_no);
            if(caller != p->type && p->ind == VAL_IND){
                if((caller == INT_TYPE) && (p->type == FLOAT_TYPE)){
                    // This is valid, we can cast an int val into a float val
                }else {
                    //Then the parameter calls are incorrect
                    print_func_ptype_error(par_num, caller, p->type, f, line_no);
                    isValid = FALSE;
                }
            } else if(caller != p->type && p->ind == REF_IND){
                if((caller == FLOAT_TYPE) && (p->type == INT_TYPE)){
                    // This is valid, we can cast an int into a float to store 
                    // in the ref.
                } else {
                    //Then the parameter calls are incorrect
                    print_func_ptype_error(par_num, caller, p->type, f, line_no);
                    isValid = FALSE;
                }
            } else if(caller != p->type && caller != INVALID_TYPE){
                //Then the parameter calls are incorrect
                print_func_ptype_error(par_num, caller, p->type, f, line_no);
                isValid = FALSE;
            }
            fcallee = fcallee->rest;
            fcaller = fcaller->rest;
            par_num ++;
        }
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

Type get_expr_type(Expr *e, Expr *parent,
    sym_table *table, char *scope_id, int line_no){

    //Find the type of an expression
    ExprKind kind = e->kind;
    // Switch on kind to print
    switch(kind) {
        case EXPR_ID:
            //Find the symbol and get it's type
            if(retrieve_symbol(e->id, scope_id, table)){
                return get_type(retrieve_symbol(e->id, scope_id, table));
            } else {
                 //Not good. Let the user know.
                print_undefined_variable_error(e, parent, line_no);
                isValid = FALSE;
                e->inferred_type = INVALID_TYPE;
                return INVALID_TYPE;
            }
            break;
        case EXPR_CONST:
            return get_const_type(e);
            break;
        case EXPR_BINOP:
            //We need the types of each expression
            //Then we determine the type if allowed using the binop
            return get_binop_type(get_expr_type(e->e1, e, table, scope_id,
                line_no),get_expr_type(e->e2, e, table, scope_id, line_no),
                e->binop, line_no, e);
            break;
        case EXPR_UNOP:
            //Get the type of the expression
            //Then we determine the type if allowed using the binop
            return get_unop_type(get_expr_type(e->e1, e,table, scope_id, 
                line_no),e->unop, line_no, e);
            break;
        case EXPR_ARRAY:
            //We need to check array dimensions, then check all expressions are
            //int equiv.
            if(validate_array_dims(e, scope_id, table, line_no)){
                symbol *a = retrieve_symbol(e->id, scope_id, table);
                validate_array_indices(e->indices, e->id, 
                    line_no, table, scope_id, a);
                e->inferred_type = get_type(a);
                return e->inferred_type;
            } else {
                symbol *a = retrieve_symbol(e->id, scope_id, table);
                if(a != NULL){
                    Decl *d = (Decl *) a->sym_value;
                    print_array_dims_error(e, count_array(d->array), 
                        count_list(e->indices), line_no);
                    isValid = FALSE;
                } else {
                    print_undefined_variable_error(e, NULL, line_no);
                }
                e->inferred_type = INVALID_TYPE;
                return INVALID_TYPE;
            }
            return INVALID_TYPE;
            break;
    }
    
    return INVALID_TYPE;
}

void validate_array_indices(Exprs *indices, char *id,
    int line_no, sym_table *table, char *scope_id, symbol *array_sym){

    int p_num = 1;
    Decl *d = (Decl *) array_sym->sym_value;
    Intervals *is = d->array;
    while(indices!=NULL){
        Expr *e = indices->first;
        Type t = get_expr_type(e, NULL, table, scope_id, line_no);
        if(t!=INT_TYPE){
            print_array_index_error(indices, id, line_no, p_num, t);
        } else if(e->kind == EXPR_CONST){
            //Now check bounds for static
            //We now know it's int and 
            Interval *i = is->first;
            int val = e->constant.val.int_val;
            if(i->lower > val || val > i->upper){
                //Then we have out of bounds here.
                print_array_outofbounds_error(indices, id, line_no, p_num, i);
                e->inferred_type = INVALID_TYPE;
            }
        }
        is = is->rest; 
        indices = indices->rest;
        p_num++;
    }
}

Type get_binop_type(Type t1, Type t2, BinOp b, int line_no, Expr *e){
    if(t1 == INVALID_TYPE || t2 == INVALID_TYPE){
        e->inferred_type = INVALID_TYPE;
        return INVALID_TYPE;
    }
    // Return appropriate type fo each class of function based on the two types
    switch(b){
        case BINOP_OR:
        case BINOP_AND:
            if (t1 != BOOL_TYPE || t2 != BOOL_TYPE){
                print_binop_error(e, line_no, t1, t2, "both be boolean types");
                isValid = FALSE;
                e->inferred_type = INVALID_TYPE;
                return INVALID_TYPE;
            } else {
                e->inferred_type = BOOL_TYPE;
                return BOOL_TYPE;
            }

        case BINOP_EQ:
        case BINOP_NTEQ:
            if( t1 == t2 ){
                e->inferred_type = BOOL_TYPE;
                return BOOL_TYPE;
            } else {
                print_binop_error(e, line_no, t1, t2, "both be equal types");
                isValid = FALSE;
                e->inferred_type = INVALID_TYPE;
                return INVALID_TYPE;
            }

        case BINOP_LT:
        case BINOP_LTEQ:
        case BINOP_GT:
        case BINOP_GTEQ:
            if( t1 == BOOL_TYPE || t2 == BOOL_TYPE){
                print_binop_error(e, line_no, t1, t2, "both be numeric types");
                isValid = FALSE;
                e->inferred_type = INVALID_TYPE;
                return INVALID_TYPE;
            } else {
                e->inferred_type = BOOL_TYPE;
                return BOOL_TYPE;
            }

        case BINOP_ADD:
        case BINOP_MUL:
        case BINOP_DIV:
            //Special case purely for div
            if(t2 != BOOL_TYPE){
                if(t2 == FLOAT_TYPE){
                    //
                } else if(t2 == INT_TYPE){

                }
            }
           
        case BINOP_SUB:
            //If either are bool, then we cannot apply these operators, 
            //otherwise return float over int.
            if( t1 == BOOL_TYPE || t2 == BOOL_TYPE){
                print_binop_error(e, line_no, t1, t2, "both be numeric types");
                e->inferred_type = INVALID_TYPE;
                isValid = FALSE;
                return INVALID_TYPE;
            } else if( t1 == FLOAT_TYPE || t2 == FLOAT_TYPE){
                e->inferred_type = FLOAT_TYPE;
                return FLOAT_TYPE;
            } else {
                e->inferred_type = INT_TYPE;
                return INT_TYPE;
            }

        default:
            e->inferred_type = INVALID_TYPE;
            return INVALID_TYPE;
    }
}

Type get_unop_type(Type t, UnOp u, int line_no, Expr *e){
    switch(u){
        case UNOP_MINUS:
            if(t == BOOL_TYPE){
                print_unop_error(e, line_no, t, "numeric");
                isValid = FALSE;
                e->inferred_type = INVALID_TYPE;
                return INVALID_TYPE;
            } else {
                e->inferred_type = t;
                return t;
            }
        case UNOP_NOT:
            if(t != BOOL_TYPE){
                print_unop_error(e, line_no, t, "boolean");
                isValid = FALSE;
                e->inferred_type = INVALID_TYPE;
                return INVALID_TYPE;
            } else {
                e->inferred_type = t;
                return t;
            }
    }
    return INVALID_TYPE;
}

Type get_const_type(Expr *e){
    e->inferred_type = e->constant.type;
    return e->inferred_type;
}

BOOL 
validate_array_dims(Expr *e, char *scope_id, sym_table *table, int line_no){
    symbol *asym = retrieve_symbol(e->id, scope_id, table);
    if(asym == NULL){
        //Array has not been defined
        return FALSE;
    } else {
        Decl *d = (Decl *) asym->sym_value;
        return count_array(d->array) == count_list(e->indices);
    }   
}

Type get_type(symbol *sym){
    // Find the second type
    sym->used = TRUE;

    if(sym->sym_type == SYM_PARAM){
        Param *p = (Param *) sym->sym_value;
        return p->type;
    } else {
        Decl *d = (Decl *) sym->sym_value;
        return d->type;
    }
}

int count_list(Exprs *l){
    int i=0;
    Exprs *p = (Exprs *) l;
    while(p!=NULL){
        i++;
        p = p->rest;
    }
    return i;
}

int count_array(Intervals *l){
    int i=0;
    Intervals *p = (Intervals *) l;
    while(p!=NULL){
        i++;
        p = p->rest;
    }
    return i;
}

int count_params(Params *l){
    int i=0;
    Params *p = (Params *) l;
    while(p!=NULL){
        i++;
        p = p->rest;
    }
    return i;
}




