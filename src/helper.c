/*                            helper.c                                 */
/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Based on template code provided by Harald Sondergard for COMP90045.
    Provides helpful functions originally included in wiz.c to be 
    used throughout the wiz compiler.
-----------------------------------------------------------------------*/
#include    <string.h>
#include    <stdlib.h>
#include 	<stdio.h>
#include 	"helper.h"

// Report an error to stderr then exit with EXIT_FAILURE.
void report_error_and_exit(const char *msg){
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

// Simple safe malloc funciton that will return the requested amount of 
// memory if available otherwiuse will exit citing failure. Taken from Wiz.c
void * checked_malloc(int num_bytes) {
    //Memory address
    void *addr;
    // Allocate with malloc
    addr = malloc((size_t) num_bytes);

    //Check if null, if so report error otherwise return.
    if (addr == NULL) 
        report_error_and_exit("Out of memory");
    return addr;
}
