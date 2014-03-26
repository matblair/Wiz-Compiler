#ifndef WIZ
#define WIZ

#include "ast.h"
#include "std.h"

extern  char    *izfile;          /* Name of file to parse */
extern  Program *parsed_program;

static  void    report_error_and_exit(const char *msg);
extern  void    *checked_malloc(int num_bytes);


int wizparser(const char * in_filename, BOOL pretty_print_only) ;
int wizparser_str(char * input_string, BOOL pretty_print_only);

#endif /* WIZ */

