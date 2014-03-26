/* pretty.c */

/*-----------------------------------------------------------------------
    A stub for a pretty-printer for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include <stdio.h>
#include "ast.h"

extern void report_error_and_exit(const char *msg);

void print_stmt_while(FILE *fp, While rec, int level);
void print_statement(FILE *fp, Stmt *rec, int level);
void print_stmt_assign(FILE *fp, Assign rec, int level);
void print_expr(FILE *fp, Expr *rec,int level);
void print_expr_const(FILE *fp, Constant rec,int level);
void print_tabs(FILE *fp, int numberOfTabs);
void print_proc(FILE *fp, Proc *proc, int level);
void print_procs(FILE *fp, Procs *procs, int level);
void print_procdef(FILE *fp, ProcDef *rec, int level);
void print_arguments(FILE *fp, Arguments *args, int level);
void print_argument(FILE *fp, Argument *arg, int level);
void print_program(FILE *, Program *);
void print_declarations(FILE *, Decls *, int level);
void print_statements(FILE *, Stmts *, int level);
void print(FILE *fp, Procs *rec, int level);


void
pretty_prog(FILE *fp, Program *prog) {

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
    fprintf(fp, "\n");
    print_procdef(fp , rec->proc_def, level);
    print_declarations(fp , rec->decls, level+1);
    print_statements(fp, rec->body, level+1);
    fprintf(fp,"%s", rec->terminator);
}

void print_procdef(FILE *fp, ProcDef *rec, int level){
    fprintf(fp, "%s ", rec->start_marker);
    fprintf(fp, "%s", rec->name);
    fprintf(fp, "%s", rec->start_paran);
    
    print_arguments(fp, rec->arguments, level);

    fprintf(fp, "%s\n", rec->end_paran);
}

void print_arguments(FILE *fp, Arguments *rec, int level){
    if (rec == NULL) return;

    print_argument(fp, rec->first, level);
    fprintf(fp, "%s", rec->separator);
    print_arguments(fp, rec->rest, level);
}

void print_argument(FILE *fp, Argument *rec, int level){
    fprintf(fp, "%s ", "Type"); //TODO: Fix type to the correct
    fprintf(fp, "%s ", rec->id);
}

void print_declarations(FILE *fp, Decls *declarations, int level) {

    if (declarations == NULL) {
        return;
    }

    Decl *decl = declarations->first;
    fprintf(fp, "%s\n", decl->id);

    print_declarations(fp, declarations->rest, level);
}


void print_statements(FILE *fp, Stmts *rec, int level){
    if (rec == NULL) return;

    print_statement(fp, rec->first, level);
    print_statements(fp, rec->rest, level);
}

void print_statement(FILE *fp, Stmt *rec, int level){
    print_tabs(fp, level);
    switch (rec->kind){
        case STMT_ASSIGN: print_stmt_assign(fp, rec->info.assign, level);
                          break;
        case  STMT_WHILE : print_stmt_while(fp, rec->info.loop, level);
                           break;
    
    }
    fprintf(fp, "\n");
}

void print_stmt_assign(FILE *fp, Assign rec, int level){
    fprintf(fp, "%s ", rec.asg_id);
    print_expr(fp, rec.asg_expr, level);

}


void print_stmt_while(FILE *fp, While rec, int level){
    fprintf(fp, "WHILE\n"); //TODO : fix this remove token
    print_statements(fp, rec.body, level +1 );
    fprintf(fp, "END\n"); //TODO: fix this
}

void print_expr(FILE *fp, Expr *rec, int level){
    switch (rec->kind){
        case EXPR_CONST : print_expr_const(fp, rec->constant, level);
                          break;

    }
}

void print_expr_const(FILE *fp, Constant rec, int level){
    fprintf(fp, "%s", rec.raw);
}


void print_tabs(FILE *fp, int numberOfTabs){
    for(int i=0; i<numberOfTabs; i++)
        fprintf(fp, "\t");
}

