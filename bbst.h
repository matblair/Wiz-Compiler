/* bbst.h */

/* Generic header file to represent a dictionary structure */

#include "std.h"

void *bbst_initialise(void);
void bbst_insert(void *dict, char *key, void *value);
void *bbst_find(void *dict, char *key);
