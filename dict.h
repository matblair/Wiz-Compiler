/* Generic header file to represent a dictionary structure */

#include "std.h"

void *initialize(void *);
void  insert(void *dict, void *key, void *value, int (*comp)(const void *a, const void *b));
BOOL  find(void *dict, void *key, void **value, int (*comp)(const void *a, const void *b));
