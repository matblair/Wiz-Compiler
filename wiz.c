/* wiz.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Based on template code provided by Harald Sondergard for COMP90045.
    Provides a driver for the pretty printer interface.

    Original message included as follows:

    "A driver for a pretty-printer for Iz, a subset of Wiz.
    For use the COMP90045 project 2014."
-----------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>

#include "ast.h"
#include "std.h"
#include "analyse.h"
#include "pretty.h"
#include "helper.h"
#include "missing.h"

const char  *progname;
const char  *iz_infile;
Program     *parsed_program = NULL;

extern FILE *yyin;

static void usage(void);
// void        report_error_and_exit(const char *msg);
// void        *checked_malloc(int num_bytes);

int
main(int argc, char **argv) {

    const char  *in_filename;
    FILE        *fp = stdout;
    BOOL        pretty_print_only;

    progname = argv[0];
    pretty_print_only = FALSE;

    /* Process command line */
    if ((argc < 2) || (argc > 3)) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (argc == 2)
        in_filename = argv[1];

    if (argc == 3 && streq(argv[1],"-p")) {
        pretty_print_only = TRUE;
        in_filename = argv[2];
    }
    
    iz_infile = in_filename;
    yyin = fopen(in_filename, "r");
    if (yyin == NULL) {
        perror(in_filename);
        exit(EXIT_FAILURE);
    }

    if (yyparse() != 0) {
        /* The error message will already have been printed. */
        exit(EXIT_FAILURE);
    }

    if (pretty_print_only) 
        pretty_prog(fp, parsed_program);
    else
        analyse_prog(parsed_program);
    return 0;
}

/*---------------------------------------------------------------------*/

static void
usage(void) {
    printf("usage: wiz [-p] iz_source_file\n");
}

// void
// report_error_and_exit(const char *msg) {
//     fprintf(stderr, "Error: %s\n", msg);
//     exit(EXIT_FAILURE);
// }

// void *
// checked_malloc(int num_bytes) {

//     void *addr;

//     addr = malloc((size_t) num_bytes);
//     if (addr == NULL) 
//         report_error_and_exit("Out of memory");
//     return addr;
// }

/*---------------------------------------------------------------------*/
