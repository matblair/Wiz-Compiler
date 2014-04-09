#ifndef WIZPARSER
#define WIZPARSER

#include "ast.h"
#include "std.h"

extern  char    *izfile;          /* Name of file to parse */

static  void    report_error_and_exit(const char *msg);
extern  void    *checked_malloc(int num_bytes);


int wizparser(const char * in_filename, BOOL pretty_print_only) ;

#endif /* WIZ */

