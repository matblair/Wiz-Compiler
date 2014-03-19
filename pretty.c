/* pretty.c */

/*-----------------------------------------------------------------------
    A stub for a pretty-printer for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include <stdio.h>
#include "ast.h"

extern void report_error_and_exit(const char *msg);

void
pretty_prog(FILE *fp, Program *prog) {

    report_error_and_exit("Pretty-print has not been implemented yet");
}

