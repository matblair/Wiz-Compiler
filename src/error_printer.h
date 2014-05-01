/*                       error_printer.h                                */
/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides functions to print informative error messages to users
    for a ranger of scenarios that occur during syntactic, semantic 
    and optimisation processes
    
-----------------------------------------------------------------------*/
#include "ast.h"

// General error
void report_error_and_exit(const char *msg);  
void print_bold(const char *string);

//Specific errors
void print_missing_main_error();
void print_dupe_proc_errors(Proc *r, Params *p, int duplicate, int original);
void print_dupe_symbol_errors(char *id, Type t1, Type t2, int duplicate, int original);
void print_assign_error(Assign *a, Type left, Type right, int line_no);
void print_if_error(Expr *e, Type c, int line_no);
void print_while_error(Expr *e, Type c, int line_no);
void print_undefined_variable_error(Expr *e, Expr *parent, int line_no);
void print_func_pmismatch_error(Function *f, Params *fcallee, int line_no,
	int expect_no, int call_no);
void print_array_dims_error(Expr *e, int expected, int actual, int line_no);
void print_binop_error(Expr *e, int line_no, Type right,
	Type left, char *expected);
void print_unop_error(Expr *e, int line_no, Type t, char *expected);
void print_func_ptype_error(int par_num, Type caller,
	 Type func, Function *f, int line_no);
void print_array_index_error(Exprs *indices, char *id, 
	 int line_no, int p_num, Type t);

void print_array_outofbounds_error(Exprs *indices, char *id, int line_no,
	int p_num, Interval *i);

void print_unused_symbol_error(char *id, int line_no);