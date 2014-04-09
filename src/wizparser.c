/* wiz.c */

/*-----------------------------------------------------------------------
    A driver for a pretty-printer for Iz, a subset of Wiz.
    For use the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#include    <string.h>
#include    <stdlib.h>
#include    "ast.h"
#include    "std.h"
#include    "pretty.h"
#include    "missing.h"

const char  *iz_infile;


static void usage(void);
void        report_error_and_exit(const char *msg);
void        *checked_malloc(int num_bytes);

extern int parse_string(char * input_sring);
extern int parse_file(FILE *, parserOutput *);

int parse(BOOL pretty_print_only, FILE *fpin, FILE *fpout){
    
    parserOutput *output = checked_malloc(sizeof (parserOutput));

    if (parse_file(fpin,output)  != 0) {
        /* The error message will already have been printed. */
        exit(EXIT_FAILURE);
    }

    if (pretty_print_only) 
        pretty_prog(fpout, output->parsed_program);
    else
        report_error_and_exit("Unable to generate code");

    return 0;
 
}

int wizparser(const char * in_filename, BOOL pretty_print_only) {

    FILE        *fp = stdout;
    FILE *fpin;

    iz_infile = in_filename;
    fpin = fopen(in_filename, "r");
    if (fpin == NULL) {
        perror(in_filename);
        exit(EXIT_FAILURE);
    }

    return parse(pretty_print_only, fpin, fp);
}


/*---------------------------------------------------------------------*/


void
report_error_and_exit(const char *msg) {

    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

void *
checked_malloc(int num_bytes) {

    void *addr;

    addr = malloc((size_t) num_bytes);
    if (addr == NULL) 
        report_error_and_exit("Out of memory");
    return addr;
}

/*---------------------------------------------------------------------*/
