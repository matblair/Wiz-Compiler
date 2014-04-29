/* Dictionary structure represented as a self balancing binary search tree.
 *
 * Based off code provided by Tony Wirth, '09
 * 
 * Implemented AA Tree method, as explained at:
 * http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_andersson.aspx
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "helper.h"
#include "dict.h"
#include "std.h"

#define LEFT 0
#define RIGHT 1
#define MINSTRING ""
#define EXTRA 3
#define BINARY 2

typedef struct bnoode{
    void *key;
    void *value;
    int level;
    struct bnoode *c[BINARY];
} bnode;

typedef struct parentchild{
    bnode *parent;    // pointer to parent node of node of interest
    int which;    // tells us whether it is a left or right child
} pc;

BOOL locate(pc *p, void *key, int (*comp)(const void *a, const void *b));
void set_dummy(pc *p, bnode *dummy);

// create an empty bst that has a dummy head node
void *
initialize(void *dummy_key)
{
    bnode *dummy; // its right pointer points to the root (when it exists)
    dummy = (bnode *) checked_malloc(sizeof(bnode));
    dummy->key = dummy_key;    // yes, this is a dummy head node!
    dummy->c[LEFT] = NULL;
    dummy->c[RIGHT] = NULL;
    dummy->level=1;

    return dummy;
}

// Implementations of iterative skew() and split()
void *
skew(bnode *root) {
    // Ensure we're not trying to skew at the end of the tree, and that a skew is
    // actually needed
    if (root->c[LEFT] != NULL && root->c[LEFT]->level == root->level) {
        bnode *temp = root->c[LEFT];
        root->c[LEFT] = temp->c[RIGHT];
        temp->c[RIGHT] = root;
        root = temp;
    }
    return root;
}

void *
split(bnode *root) {
    // Before splitting, make sure that it's valid to look further down the tree
    if (root->c[RIGHT] == NULL || root->c[RIGHT]->c[RIGHT] == NULL) return root;
    // Okay, now do all the work
    if (root->c[RIGHT]->c[RIGHT]->level == root->level) {
        bnode *temp = root->c[RIGHT];
        root->c[RIGHT] = temp->c[LEFT];
        temp->c[LEFT] = root;
        root = temp;
        (root->level)++;
    }
    return root;
}

// find the location of the key: if it is not present, the returned
// pc should have now set to NULL and parent/which set up so that the key
// would be inserted as the 'which' child of parent.
BOOL
locate(pc *p, void *key, int (*comp)(const void *a, const void *b))
{
    bnode *this = p->parent->c[p->which];    // pointer to the child of interest

    if(this == NULL){        // if we have fallen off the tree
        return FALSE;    // the key is not present, but
                // return this location, where it
            // *should* be
    }
    if((*comp)(key, this->key) < 0){
        p->parent = this;
        p->which = LEFT;
        return locate(p, key, comp);
    }
    if((*comp)(key, this->key) > 0){
        p->parent = this;
        p->which = RIGHT;
        return locate(p, key, comp);
    }

    // if we have reached here, then we have found the key
    return TRUE;
}

// Inserts and balances as it goes, based on the AA Tree model
bnode *
insertandbalance(bnode *root, void *key, void *value,
                 int (*comp)(const void *a, const void *b))
{
    int result, which;

    // If pointer is to NULL we have found where to insert
    if (root == NULL) {
        bnode *new;
        new = (bnode *) checked_malloc(sizeof(bnode));
        new->key = key;
        new->value = value;
        new->level = 1;
        new->c[LEFT] = NULL;
        new->c[RIGHT] = NULL;
        return new;
    }

    // Haven't yet found where to insert, so keep looking
    // Determine whether we will look to the right or the left
    result = (*comp)(key, root->key);

    if (result == 0) {
        root->value = value;

    } else {
        if (result < 0){
            which = LEFT;
        } else {
            which = RIGHT;
        }
        root->c[which] = insertandbalance(root->c[which], key, value, comp);
        root = skew(root);
        root = split(root);
    }
    return root;
}

// here we insert a (new) record into the bst
void
insert(void *dict, void *key, void *value,
       int (*comp)(const void *a, const void *b))
{
    bnode *dummy = (bnode *) dict;
    dummy->c[RIGHT] = insertandbalance(dummy->c[RIGHT], key, value, comp);
    return;
}

// given a key, put the value corrsponding to it in the location referenced
BOOL
find(void *dict, void *key, void **value,
     int (*comp)(const void *a, const void *b))
{
    bnode *dummy = (bnode *) dict;
    pc pw;
    pc *p = &pw;

    set_dummy(p,dummy);

    // find out the appropriate location of where this key should be
    if(locate(p, key, comp)){
        *value = p->parent->c[p->which]->value;
        return TRUE;
    } else {
        return FALSE;
    }
}

void
set_dummy(pc *p, bnode *dummy)
{
    p->parent = dummy;
    p->which = RIGHT;
}
