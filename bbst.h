/* bbst.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Provides function definitions and external access rights for
    balanced_bst.h
-----------------------------------------------------------------------*/
#include <stdio.h>

/*----------------------------------------------------------------------
    Publically accessible functions for use in building and finding
    items in a bst.
-----------------------------------------------------------------------*/

void *bbst_intialize();

void *bbst_find_node(void *id, void *t,
                     int (*comp)(const void *a, const void *b));

void *bbst_insert(void *t, void *key, void *value,
                  int (*comp)(const void *a, const void *b));

void  bbst_dump_it(void *t, int offset, char *(*p_node)(const void *node));

void  bbst_map(void *t, void (*map_func)(const void *node));