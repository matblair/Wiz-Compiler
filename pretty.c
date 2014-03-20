/* pretty.c */

/*-----------------------------------------------------------------------
    A stub for a pretty-printer for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include <stdio.h>
#include "ast.h"

extern void report_error_and_exit(const char *msg);

void print_program(FILE *, Program *);
void print_declarations(FILE *, Decls *);
void print_statements(FILE *, Stmts *);

void
pretty_prog(FILE *fp, Program *prog) {

    print_program(fp, prog);
    //report_error_and_exit("Pretty-print has not been implemented yet");
}

void print_program(FILE *fp, Program *program) {

    print_declarations(fp, program->decls);
    fprintf(fp, "\n");
    print_statements(fp, program->body);
}

void print_declarations(FILE *fp, Decls *declarations) {

    if (declarations == NULL) {
        return;
    }

    Decl *decl = declarations->first;
    fprintf(fp, "%s\n", decl->id);

    print_declarations(fp, declarations->rest);
}

void print_statements(FILE *fp, Stmts *statements) {

    if (statements == NULL) {
        return;
    }


}