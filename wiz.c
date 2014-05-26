/* wiz.c */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Based on template code provided by Harald Sondergard for COMP90045.
    Provides a driver for the pretty printer interface.

    Original message included as follows:

    "A driver for a pretty-printer for Iz, a subset of Wiz.
    For use the COMP90045 project 2014."
-----------------------------------------------------------------------*/

#include    <string.h>
#include    <stdlib.h>
#include    "ast.h"
#include    "std.h"
#include    "pretty.h"
#include    "helper.h"
#include    "codegen.h"
#include    "analyse.h"
#include    "missing.h"
#include    "pretty.h"
#include    "wizoptimiser.h"
#include    "error_printer.h"

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
    BOOL        analyse_optimise_print;
    BOOL        optimise;
    BOOL        to_file;


    progname = argv[0];
    pretty_print_only = FALSE;
    analyse_optimise_print = FALSE;
    optimise = FALSE;

    /* Process command line */
    if ((argc < 2) || (argc > 3)) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (argc == 2) {
        in_filename = argv[1];
    }

    if (argc == 3 && streq(argv[1], "-p")) {
        pretty_print_only = TRUE;
        in_filename = argv[2];
    }
    if (argc == 3 && streq(argv[1], "-c")) {
        analyse_optimise_print = TRUE;
        in_filename = argv[2];
    }

    if (argc == 3 && streq(argv[1], "-o")) {
        optimise = TRUE;
        in_filename = argv[2];
    }

    if (argc == 3 && streq(argv[1], "-f")) {
        to_file = TRUE;
        in_filename = argv[2];
    }


    yyin = fopen(in_filename, "r");
    if (yyin == NULL) {
        perror(in_filename);
        exit(EXIT_FAILURE);
    }

    if (yyparse() != 0) {
        /* The error message will already have been printed. */
        exit(EXIT_FAILURE);
    }

    if (pretty_print_only) {
        pretty_prog(fp, parsed_program);
        return 0;
    }

    if (analyse_optimise_print) {
        print_bold("Original Program:");
        pretty_prog(fp, parsed_program);
        reduce_ast(parsed_program);
        print_bold("\nOptimised Program:");
        pretty_prog(fp, parsed_program);
        print_bold("\n Errors Detected:");
        analyse(parsed_program);
        return 0;
    }

    //Standard compilation
    if (optimise) {
        reduce_ast(parsed_program);
    } else if (to_file) {
        reduce_ast(parsed_program);
        int in_filename_len = strlen(in_filename);
        char *outfile = checked_malloc((strlen(in_filename) + 4) * sizeof(char));
        //suffix points to the '.' in filename, assuming it ends in ".wiz"
        const char *suffix = &in_filename[in_filename_len - 4];

        //decide whether there is a ".wiz" suffix to remove or not
        if (in_filename_len > 3 && strcmp(suffix, ".wiz") == 0) {
            //in this case remove the last three characters (w, i and z)
            strncpy(outfile, in_filename, in_filename_len - 3);
            //append null-byte because strncpy does not do this automatically
            outfile[in_filename_len - 3] = '\0';
        }
        else {
            //if suffix is not ".wiz", just append ".oz" to end of name
            strncpy(outfile, in_filename, in_filename_len);
            outfile[in_filename_len] = '.';
            //append nullbute in preparation of appending "oz"
            outfile[in_filename_len + 1] = '\0';
        }

        //finally add the "oz" to end of name
        const char *ending = "oz";
        strcat(outfile, ending);

        printf("%s\n", outfile);
        fp = fopen(outfile, "w");
    }

    compile(fp, parsed_program);
    return 0;
}

/*---------------------------------------------------------------------*/

static void
usage(void) {
    printf("usage: wiz [-p|-o|-c|-f] iz_source_file\n"
           "\t -p : Parses program and pretty prints internal\n"
           "\t      representation to stdout.\n"
           "\t -o : Compile with optimisations enabled.\n"
           "\t      Output is written to stdout.\n"
           "\t -c : Optimise and reduce expressions, printing the before\n"
           "\t      and after results, along with any errors.\n"
           "\t      Output is written to stdout.\n"
           "\t -f : Compile with optimisations enabled.\n"
           "\t      Output is written to file WIZ_SOURCE_PREFIX.oz (where\n"
           "\t      WIZ_SOURCE_PREFIX is the prefix of wiz_source_file\n"
           "\t      - i.e. with '.wiz' suffix removed, if present).\n");

}

/*---------------------------------------------------------------------*/
