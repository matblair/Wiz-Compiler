/*
 * ============================================================================
 * =========
 * 
 * Filename:  pretty.c
 * 
 * Description:  Pretty Prints a program
 * 
 * Version:  1.0 Created:  04/09/2014 14:04:00 Revision:  none Compiler:  gcc
 * 
 * Author:  AE Company: University Of Melbourne
 * 
 * ====================
 * ================================================================ */

#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

#define token_terminator ";"
#define TAB_SPACE  4

extern void	report_error_and_exit(const char *msg);
void		print_expr_list(FILE * fp, ExprList * expr_list);
void		print_stmt_while(FILE * fp, While rec, int level);
void		print_stmt_if(FILE * fp, Cond rec, int level);
void		print_statement(FILE * fp, Stmt * rec, int level);
void		print_stmt_assign(FILE * fp, Assign rec, int level);
void		print_expr(FILE * fp, Expr * rec);
void		print_expr_const(FILE * fp, Constant rec);
void		print_proc(FILE * fp, Proc * proc, int level);
void		print_procs(FILE * fp, Procs * procs, int level);
void		print_procdef(FILE * fp, ProcDef * rec, int level);
void		print_arguments(FILE * fp, Arguments * args);
void		print_argument(FILE * fp, Argument * arg);
void		print_program(FILE *, Program *);
void		print_declarations(FILE *, Decls *, int level);
void		print_statements(FILE *, Stmts *, int level);
void		print     (FILE * fp, Procs * rec, int level);
char           *sp(int);
char           *get_datatype(Type datatype);
void    	print_expr_unop(FILE *, UnOp , Expr *);



/*
 * ===  FUNCTION
 * ======================================================================
 * Name:  get_datatype 
 * Description:  Returns the datatype string associated
 * with the datatype enum
 * ===========================================================================
 * ==========
 */
char           *
get_datatype(Type datatype)
{
	switch (datatype) {
	case INT_TYPE:
		return "int";
	case BOOL_TYPE:
		return "bool";
	case FLOAT_TYPE:
		return "float";
	default:
		report_error_and_exit("Unknow datatype...");
	}
	return "Invalid State Shoudl never see this";
}


/*
 * ===  FUNCTION
 * ======================================================================
 * Name:  get_argtype 
 * Description:  Returns the argtype string associated
 * with the argument enum
 * ===========================================================================
 * ==========
 */
char           *
get_argtype(ArgType argtype)
{
	switch (argtype) {
	case REF:
		return "ref";
	case VAL:
		return "val";
	default:
		report_error_and_exit("unknown argtype...");
	}
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_expr_unop
 *  Description:  Print Unary operations
 * =====================================================================================
 */
void    	print_expr_unop(FILE * fp, UnOp op , Expr * expr){
    switch(op){
        case UNOP_MINUS : fprintf(fp, "-");
                          break;
        case UNOP_NOT : fprintf(fp, "not ");
                          break;

    }
    print_expr(fp, expr);
}

/*
 * ===  FUNCTION
 * ======================================================================
 * Name:  print_program Description:  Prints the Program structure
 * ===========================================================================
 * ==========
 */
void
print_program(FILE * fp, Program * program)
{

	print_procs(fp, program->procs, 0);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_procs
 *  Description:  Prints Procs*  type record 
 * =====================================================================================
 */
void
print_procs(FILE * fp, Procs * rec, int level)
{
	if (rec == NULL)
		return;

	print_proc(fp, rec->first, level);
	print_procs(fp, rec->rest, level);

}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_proc
 *  Description:  Prints Proc* type record
 * =====================================================================================
 */
void
print_proc(FILE * fp, Proc * rec, int level)
{
	print_procdef(fp, rec->proc_def, level);
	print_declarations(fp, rec->decls, level + 1);
	print_statements(fp, rec->body, level + 1);
	fprintf(fp, "%s\n\n", "end");
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_procdef
 *  Description:  Prints ProcDef* type record
 * =====================================================================================
 */
void
print_procdef(FILE * fp, ProcDef * rec, int level)
{
	fprintf(fp, "%s ", "proc");
	fprintf(fp, "%s", rec->name);
	fprintf(fp, "%s", "(");

	print_arguments(fp, rec->arguments);

	fprintf(fp, "%s\n", ")");
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_arguments
 *  Description:  Prints Arguments * type record
 * =====================================================================================
 */
void
print_arguments(FILE * fp, Arguments * rec)
{
	if (rec == NULL)
		return;

	print_argument(fp, rec->first);

	if (rec->rest != NULL)
		fprintf(fp, ", ");
	print_arguments(fp, rec->rest);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_argument
 *  Description:  Prints argument * type record
 * =====================================================================================
 */
void
print_argument(FILE * fp, Argument * rec)
{
	fprintf(fp, "%s ", get_argtype(rec->arg_type));
	fprintf(fp, "%s ", get_datatype(rec->type));
	fprintf(fp, "%s", rec->id);
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_dim
 *  Description:  Prints a dimension , from type Dimension *
 * =====================================================================================
 */
void
print_dim(FILE * fp, Dimension * dim)
{
	if (dim == NULL)
		return;
	fprintf(fp, "%d..%d", dim->lb, dim->ub);
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_dims
 *  Description:  Prints dimensions, from type Dimensions *
 * =====================================================================================
 */
void
print_dims(FILE * fp, Dimensions * dims)
{
	if (dims == NULL)
		return;
	print_dim(fp, dims->first);
	if (dims->rest != NULL)
		fprintf(fp, ",");
	print_dims(fp, dims->rest);

}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_declaration
 *  Description:  Prints a declaration of a procedure based on Decl * type
 * =====================================================================================
 */
void
print_declaration(FILE * fp, Decl * decl, int level)
{

	if (decl == NULL) {
		return;
	}
	fprintf(fp, "%s%s %s", sp(level), get_datatype(decl->type), decl->id);

	//print optional dimensions
		if (decl->dims == NULL) {
		fprintf(fp, ";\n");
		return;
	}
	fprintf(fp, "[");
	print_dims(fp, decl->dims);
	fprintf(fp, "]");
	fprintf(fp, ";\n");

}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_declarations
 *  Description:  Prints declarations based on Decls * 
 * =====================================================================================
 */
void
print_declarations(FILE * fp, Decls * declarations, int level)
{
	if (declarations == NULL) {
		return;
	}
	print_declaration(fp, declarations->first, level);

	print_declarations(fp, declarations->rest, level);
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_statements
 *  Description:  Prints a generic statement, is a wrapper to other functions
 *  which perform the actual print based on the statement type
 * =====================================================================================
 */
void
print_statements(FILE * fp, Stmts * rec, int level)
{
	if (rec == NULL)
		return;

	print_statement(fp, rec->first, level);
	print_statements(fp, rec->rest, level);
}

/*
 * ===  FUNCTION
 * ======================================================================
 * Name:  print_stmt_write Description:  Prints write Statement
 * ===========================================================================
 * ==========
 */
void
print_stmt_write(FILE * fp, Expr * rec, int level)
{
	fprintf(fp, "%swrite ", sp(level));
	print_expr(fp, rec);
	fprintf(fp, ";\n");
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_stmt_funccall
 *  Description:  Prints function call statements based on IdExprList record
 * =====================================================================================
 */
void
print_stmt_funccall(FILE * fp, IdExprList * rec, int level)
{
	fprintf(fp, "%s%s", sp(level), rec->id);
	fprintf(fp, "(");
	print_expr_list(fp, rec->expr_list);
	fprintf(fp, ")");
	fprintf(fp, ";\n");
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_stmt_read
 *  Description:  Prints Read statement based on IdExprList * rec
 * =====================================================================================
 */
void
print_stmt_read(FILE * fp, IdExprList * rec, int level)
{
	fprintf(fp, "%sread %s", sp(level), rec->id);
	if (rec->expr_list != NULL) {
		fprintf(fp, "[");
		print_expr_list(fp, rec->expr_list);
		fprintf(fp, "]");
	}
	fprintf(fp, ";\n");
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_expr_list
 *  Description:  Prints any statement based on Stmt * rec. This is a wrapper
 *  function that calls other functions based on the statement type
 * =====================================================================================
 */
void
print_statement(FILE * fp, Stmt * rec, int level)
{
	switch (rec->kind) {
	case STMT_ASSIGN:
		print_stmt_assign(fp, rec->info.assign, level);
		break;
	case STMT_WHILE:
		print_stmt_while(fp, rec->info.loop, level);
		break;
	case STMT_WRITE:
		print_stmt_write(fp, rec->info.write, level);
		break;
	case STMT_FUNCCALL:
		print_stmt_funccall(fp, rec->info.id_expr_list, level);
		break;
	case STMT_READ:
		print_stmt_read(fp, rec->info.id_expr_list, level);
		break;
	case STMT_COND:
		print_stmt_if(fp, rec->info.cond, level);
		break;
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_expr_list
 *  Description:  Prints Expression Lists 
 * =====================================================================================
 */
void
print_expr_list(FILE * fp, ExprList * expr_list)
{
	if (expr_list == NULL)
		return;

	print_expr(fp, expr_list->first);
	if (expr_list->rest != NULL) {
		fprintf(fp, ", ");
		print_expr_list(fp, expr_list->rest);

	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_stmt_assign
 *  Description:  Prints Assignment Statements
 * =====================================================================================
 */
void
print_stmt_assign(FILE * fp, Assign rec, int level)
{

	fprintf(fp, "%s%s", sp(level), rec.id_expr_list->id);
	if (rec.id_expr_list->expr_list != NULL) {
		fprintf(fp, "[");
		print_expr_list(fp, rec.id_expr_list->expr_list);
		fprintf(fp, "]");
	}
	fprintf(fp, " := ");
	print_expr(fp, rec.asg_expr);
	fprintf(fp, "%s\n", ";");

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_stmt_while
 *  Description:  Prints While Statement based on While record. 
 * =====================================================================================
 */
void
print_stmt_while(FILE * fp, While rec, int level)
{
	fprintf(fp, "%swhile ", sp(level));
	print_expr(fp, rec.cond);
	fprintf(fp, " %s\n", "do");
	print_statements(fp, rec.body, level + 1);
	fprintf(fp, "%sod\n", sp(level));
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_stmt_if
 *  Description:  Prints if statements, based on Cond type rec
 * =====================================================================================
 */
void
print_stmt_if(FILE * fp, Cond rec, int level)
{
	//if
    fprintf(fp, "%sif ", sp(level));
	print_expr(fp, rec.cond);
	//then
    fprintf(fp, " %s\n", "then");
	//statements
    print_statements(fp, rec.then_branch, level + 1);
	//else
    if (rec.else_branch != NULL) {
		fprintf(fp, "%selse\n", sp(level));
		print_statements(fp, rec.else_branch, level + 1);
	}
    //end if
	fprintf(fp, "%sfi\n", sp(level));

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_expr_binop_op
 *  Description:  Prints a binary operation, where the operand is passed as a
 *  string.
 * =====================================================================================
 */
void
print_expr_binop_op(FILE * fp, char *op, Expr * e1, Expr * e2)
{
	BOOL		pb = 0;

    //print left operand
	if (e1->kind == EXPR_BINOP)
		pb = 1;
	if (pb)
		fprintf(fp, "(");
	print_expr(fp, e1);
	if (pb)
		fprintf(fp, ")");
    
    //print operator
	fprintf(fp, " %s ", op);

    //print right operand
	pb = 0;
	if (e2->kind == EXPR_BINOP)
		pb = 1;
	if (pb)
		fprintf(fp, "(");
	print_expr(fp, e2);
	if (pb)
		fprintf(fp, ")");
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_expr_binop
 *  Description:  Prints any binary operation. This is a wrapper function for all
 *  binary operations
 * =====================================================================================
 */
void
print_expr_binop(FILE * fp, BinOp op, Expr * e1, Expr * e2)
{
	switch (op) {
	case BINOP_ADD:
		print_expr_binop_op(fp, "+", e1, e2);
		break;
	case BINOP_DIV:
		print_expr_binop_op(fp, "/", e1, e2);
		break;
	case BINOP_MUL:
		print_expr_binop_op(fp, "*", e1, e2);
		break;
	case BINOP_SUB:
		print_expr_binop_op(fp, "-", e1, e2);
		break;
	case BINOP_LT:
		print_expr_binop_op(fp, "<", e1, e2);
		break;
	case BINOP_GT:
		print_expr_binop_op(fp, ">", e1, e2);
		break;
	case BINOP_LTE:
		print_expr_binop_op(fp, "<=", e1, e2);
		break;
	case BINOP_GTE:
		print_expr_binop_op(fp, ">=", e1, e2);
		break;
	case BINOP_EQ:
		print_expr_binop_op(fp, "=", e1, e2);
		break;
	case BINOP_NEQ:
		print_expr_binop_op(fp, "-", e1, e2);
		break;
	case BINOP_AND:
		print_expr_binop_op(fp, " and ", e1, e2);
		break;
	case BINOP_OR:
		print_expr_binop_op(fp, " or ", e1, e2);
		break;
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_expr
 *  Description:  Prints Expr * record. Just a wrapper function that calls
 *  other functions based on the expression type
 * =====================================================================================
 */
void
print_expr(FILE * fp, Expr * rec)
{
	switch (rec->kind) {
	case EXPR_ID:
		fprintf(fp, "%s", rec->id);
		break;
	case EXPR_CONST:
		print_expr_const(fp, rec->constant);
		break;
	case EXPR_BINOP:
		print_expr_binop(fp, rec->binop, rec->e1, rec->e2);
		break;
	case EXPR_UNOP:
		print_expr_unop(fp, rec->unop, rec->e1);
		break;

	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  print_expr_const
 *  Description:  Prints Constant record, literals such as string, float, bool
 *  etc
 * =====================================================================================
 */
void
print_expr_const(FILE * fp, Constant rec)
{
	fprintf(fp, "%s", rec.raw);
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sp
 *  Description:  Returns n number of spaces based on the level/number of tabs
 * =====================================================================================
 */
char           *
sp(int numberOfTabs)
{
	int		i;
	char           *space = malloc(sizeof(char) * (TAB_SPACE) *
				       (numberOfTabs) + 1);
	for (i = 0; i < numberOfTabs; i++)
		space[i] = '\t';
	space[i] = '\0';
	return space;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  pretty_prog
 *  Description:  Pretty Prints a program and is the main function to be used
 *  by external programs
 * =====================================================================================
 */
void
pretty_prog(FILE * fp, Program * prog)
{
	print_program(fp, prog);
}
