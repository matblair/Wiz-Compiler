/* wiz.c */

/*-----------------------------------------------------------------------
    A driver for a pretty-printer for Iz, a subset of Wiz.
    For use the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include    "wizparser.h"
#include    <string.h>
#include    <stdio.h>
#include    "std.h"
#include    <stdlib.h>

static void
usage(void) {

    printf("usage: wiz [-p] iz_source_file\n");
}


int
main(int argc, char **argv) {
   
    const char  *in_filename;
    FILE        *fp = stdout;
    BOOL        pretty_print_only;
    const char  *progname;

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

    return wizparser(in_filename, pretty_print_only);    
}
