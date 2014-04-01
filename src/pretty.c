/* pretty.c */

/*-----------------------------------------------------------------------
    A stub for a pretty-printer for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

#define token_terminator ";"
#define TAB_SPACE  4

extern void report_error_and_exit(const char *msg);
void print_expr_list(FILE * fp, ExprList *expr_list);
void print_stmt_while(FILE *fp, While rec, int level);
void print_stmt_if(FILE *fp, Cond rec, int level);
void print_statement(FILE *fp, Stmt *rec, int level);
void print_stmt_assign(FILE *fp, Assign rec, int level);
void print_expr(FILE *fp, Expr *rec);
void print_expr_const(FILE *fp, Constant rec);
void print_proc(FILE *fp, Proc *proc, int level);
void print_procs(FILE *fp, Procs *procs, int level);
void print_procdef(FILE *fp, ProcDef *rec, int level);
void print_arguments(FILE *fp, Arguments *args);
void print_argument(FILE *fp, Argument *arg);
void print_program(FILE *, Program *);
void print_declarations(FILE *, Decls *, int level);
void print_statements(FILE *, Stmts *, int level);
void print(FILE *fp, Procs *rec, int level);
char * sp(int );

char* get_datatype(Type datatype){
    switch (datatype){
        case INT_TYPE: return "int";
        case BOOL_TYPE: return "bool";
        case FLOAT_TYPE: return "float";
        default: report_error_and_exit("Unknow datatype...");
    }
}

char* get_argtype(ArgType argtype){
    switch (argtype){
        case REF : return "ref";
        case VAL : return "val";
        default: report_error_and_exit("unknown argtype...");
    }
}

void pretty_prog(FILE *fp, Program *prog) {

    print_program(fp, prog);
    //report_error_and_exit("Pretty-print has not been implemented yet");
}

void print_program(FILE *fp, Program *program) {

    print_procs(fp, program->procs, 0);
}

void print_procs(FILE *fp, Procs *rec, int level){
    if (rec == NULL) return;

    print_proc(fp, rec->first, level);
    print_procs(fp, rec->rest, level);

}

void print_proc(FILE *fp, Proc *rec, int level){
    print_procdef(fp , rec->proc_def, level);
    print_declarations(fp , rec->decls, level+1);
    print_statements(fp, rec->body, level+1);
    fprintf(fp,"%s\n\n", "end");
}

void print_procdef(FILE *fp, ProcDef *rec, int level){
    fprintf(fp, "%s ", "proc");
    fprintf(fp, "%s", rec->name);
    fprintf(fp, "%s", "(");
    
    print_arguments(fp, rec->arguments);

    fprintf(fp, "%s\n", ")");
}

void print_arguments(FILE *fp, Arguments *rec){
    if (rec == NULL) return;

    print_argument(fp, rec->first);
    
    if (rec->rest != NULL) fprintf(fp, ", ");
    print_arguments(fp, rec->rest);
}

void print_argument(FILE *fp, Argument *rec){
    fprintf(fp, "%s ", get_argtype(rec->arg_type)); 
    fprintf(fp, "%s ", get_datatype(rec->type)); 
    fprintf(fp, "%s", rec->id);
}

void print_dim(FILE *fp, Dimension *dim){
    if (dim == NULL) return;
    fprintf(fp, "%d..%d", dim->lb, dim->ub);
}

void print_dims(FILE *fp, Dimensions *dims){
    if (dims == NULL) return;
    print_dim(fp,  dims->first);
    if (dims->rest != NULL) fprintf(fp, ",");
    print_dims(fp, dims->rest);

}
void print_declaration(FILE *fp, Decl *decl, int level) {

    if (decl == NULL) {
        return;
    }

    fprintf(fp, "%s%s %s",sp(level), get_datatype( decl->type), decl->id);
   
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


void print_declarations(FILE *fp, Decls *declarations, int level) {

    if (declarations == NULL) {
        return;
    }

    print_declaration(fp, declarations->first, level);

    print_declarations(fp, declarations->rest, level);
}


void print_statements(FILE *fp, Stmts *rec, int level){
    if (rec == NULL) return;

    print_statement(fp, rec->first, level);
    print_statements(fp, rec->rest, level);
}

void print_stmt_write(FILE *fp, Expr *rec,int level){
    fprintf(fp, "%swrite ",sp(level));
    print_expr(fp, rec);
    fprintf(fp, ";\n");
}
void print_stmt_funccall(FILE *fp, IdExprList *rec, int level){
    fprintf(fp, "%s%s",sp(level), rec->id);
    fprintf(fp, "(");
    print_expr_list(fp, rec->expr_list);
    fprintf(fp, ")");
    fprintf(fp, ";\n");
}

void print_stmt_read(FILE *fp, IdExprList *rec, int level){
    fprintf(fp, "%sread %s",sp(level), rec->id);
    if (rec->expr_list != NULL) 
    {
        fprintf(fp, "[");
        print_expr_list(fp, rec->expr_list);
        fprintf(fp, "]");
    }
    fprintf(fp, ";\n");
}

void print_statement(FILE *fp, Stmt *rec, int level){
    switch (rec->kind){
        case STMT_ASSIGN: print_stmt_assign(fp, rec->info.assign, level);
                          break;
        case  STMT_WHILE : print_stmt_while(fp, rec->info.loop, level);
                           break;
        case  STMT_WRITE : print_stmt_write(fp, rec->info.write, level);
                           break;
        case  STMT_FUNCCALL: print_stmt_funccall(fp, rec->info.id_expr_list, level);
                           break;
        case  STMT_READ : print_stmt_read(fp, rec->info.id_expr_list, level);
                           break;
        case  STMT_COND : print_stmt_if(fp, rec->info.cond, level);
                           break;
    }
}

void print_expr_list(FILE * fp, ExprList *expr_list){
    if (expr_list == NULL) return;

    print_expr(fp, expr_list->first);
    if (expr_list->rest != NULL ) {
        fprintf(fp, ",");
        print_expr_list(fp, expr_list->rest);

    }
}

void print_stmt_assign(FILE *fp, Assign rec, int level){
    
    fprintf(fp, "%s%s",sp(level), rec.id_expr_list->id);
    if (rec.id_expr_list->expr_list != NULL) 
    {
        fprintf(fp, "[");
        print_expr_list(fp, rec.id_expr_list->expr_list);
        fprintf(fp, "]");
    }

    fprintf(fp, " := ");
    print_expr(fp, rec.asg_expr);
    fprintf(fp, "%s\n", ";");

}

void print_stmt_while(FILE *fp, While rec, int level){
    fprintf(fp, "%swhile ", sp(level) ); //TODO : fix this remove token
    print_expr(fp, rec.cond);
    fprintf(fp, " %s\n", "then");
    print_statements(fp, rec.body, level +1 );
    fprintf(fp, "%send\n",sp(level)); //TODO: fix this
}

void print_stmt_if(FILE *fp, Cond rec, int level){
    fprintf(fp, "%sif ", sp(level));
    print_expr(fp, rec.cond);
    fprintf(fp, " %s\n", "then");
    print_statements(fp, rec.then_branch, level +1 );
    if (rec.else_branch != NULL){
        fprintf(fp, "%selse\n", sp(level));
        print_statements(fp, rec.else_branch, level +1 );
    }
    fprintf(fp, "%sfi\n",sp(level)); //TODO: fix this

}

void print_expr_binop(FILE *fp, BinOp op, Expr *e1, Expr *e2){
   switch(op){
       case BINOP_ADD : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, "+");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_DIV : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, "/");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_MUL : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, "*");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_SUB : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, "/");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_LT : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, "<");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_GT : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, ">");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_LTE : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, "<=");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_GTE : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, ">=");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_EQ : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, "=");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_NEQ : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, "!=");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_AND : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, " and ");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
       case BINOP_OR : fprintf(fp, "(");
                        print_expr(fp, e1);
                        fprintf(fp, " or ");
                        print_expr(fp, e2);
                        fprintf(fp, ")");
                        break;
   }
}

void print_expr(FILE *fp, Expr *rec){
    switch (rec->kind){
        case EXPR_ID :    fprintf(fp,"%s", rec->id);
                          break;
        case EXPR_CONST : print_expr_const(fp, rec->constant);
                          break;
        case EXPR_BINOP : print_expr_binop(fp, rec->binop, rec->e1, rec->e2);
                          break;

    }
}

void print_expr_const(FILE *fp, Constant rec){
    fprintf(fp, "%s", rec.raw);
}



char * sp(int numberOfTabs){
    int i;
    char * space = malloc(sizeof (char) * ( TAB_SPACE)  *( numberOfTabs) + 1);
    for( i=0; i<numberOfTabs; i++)
      space[i]='\t';
    space[i]='\0';
    return space;
}

