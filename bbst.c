/* bbst.c */

/*------------------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    
    Provides a balanced binary search tree for use in the symbol table.
------------------------------------------------------------------------------*/
#include "bbst.h"
#include "helper.h"
#include "std.h"

/*------------------------------------------------------------------------------
    Internal structures.
------------------------------------------------------------------------------*/
#define EXTRA 10
#define INITAL_LEVEL 0

// A node for our bbst. Has void types for the node
typedef struct node {
    void *current;
    struct node *left;
    struct node *right;
    int level;
} t_node;

// Parnet child structure for balancing purposes.
typedef struct parentchild {
    t_node *parent; // pointer to parent node of node of interest
    int which;  // tells us whether it is a left or right child
} pc;

/*------------------------------------------------------------------------------
    Internal function definitions.
------------------------------------------------------------------------------*/
t_node *recursive_find(void *id, t_node *head,
                       int (*comp)(const void *, const void *));
t_node *skew(t_node *root);
t_node *split(t_node *root);
t_node *make_node(void *value, int level);

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/

void *bbst_intialize() {
    //Here we create a symbol table with no scopes (i.e. a dummy head);
    t_node *table = make_node(NULL, INITAL_LEVEL);
    //Return the initialized table
    return table;
}


t_node *make_node(void *value, int level) {
    t_node *node = (t_node *) checked_malloc(sizeof(t_node));
    node->current = value;
    node->left = NULL;
    node->right = NULL;
    node->level = level;
    return node;
}

void *bbst_find_node(void *id, void *t, int (*comp)(const void *,
                     const void *)) {
    // Cast to tree
    t_node *tree = (t_node *) t;
    // Find the node
    t_node *node = recursive_find(id, tree, comp);

    // Return the node
    return (void *) node;
}

t_node *
recursive_find(void *id, t_node *head, int (*comp)(const void *,
               const void *)) {
    if (head == NULL) {
        //Ooops we have fallen off the tree.
        return NULL;
    }
    int comparison = (*comp)(id, head->current);
    if (comparison > 0) {
        return recursive_find(id, head->right, comp); // in tree
    } else if (comparison < 0) {
        return recursive_find(id, head->left, comp);
    }
    // if we have reached here, then we have found the key
    return head->current;
}

// A recursive implementation of insertion in a binary tree, it's easier to
// keep track of the skew and split operations applying them like this.
void *
bbst_insert(void *search, void *key, void *value,
            int (*comp)(const void *, const void *)) {
    // If the pointer is NULL
    if (search == NULL) {
        t_node *new_node = make_node(value, 1);
        return (void *)new_node;
    }

    t_node *head = (t_node *) search;

    // If we haven't found it then we have to go fishing!
    // We determine the direction we set off using our handy built in
    // key_comp function. If key compare shows smaller go left, else go right,
    // like a recursive (i.e. lazy) binary search.
    int side = (*comp)(key, head->current);
    if (side == 0) {
        // Then the keys are identical so we should
        //update the records.
        head->current = value;
    } else {
        if (side > 0) {
            head->right = bbst_insert(head->right, key, value, comp);
        } else {
            head->left = bbst_insert(head->left, key, value, comp);;
        }

        // Balance on the way back up
        head = skew(head);
        head = split(head);
    }

    return (void *) head;
}


// Performs a simple skew operation on the given node pointer.
// After checking that it is safe to do so
t_node *skew(t_node *root) {
    // Ensure we're not trying to skew at the end of the tree, and that a
    // skew isactually needed
    if (root->left != NULL && root->left->level == root->level) {
        t_node *temp = root->left;
        root->left = temp->right;
        temp->right = root;
        root = temp;
    }
    return root;
}

// Performs a rotation like operation (splitting) after checking the two
// conditions that require it are satisfied.
t_node *split(t_node *root) {
    // Before splitting, make sure that it's valid to look further down
    if (root->right == NULL || root->right->right == NULL) {
        return root;
    }
    // Okay, now do all the work
    if (root->right->right->level == root->level) {
        t_node *temp = root->right;
        root->right = temp->left;
        temp->left = root;
        root = temp;
        (root->level)++;
    }
    return root; // Return root, if condition one and two are met a split will
    // have been performed, otherwise the pointer will not have been changed.
}


void bbst_dump_it(void *t, int offset, char *(*p_node)(const void *node)) {
    t_node *tree = (t_node *)t;
    if (tree->right != NULL) {
        bbst_dump_it(tree->right, offset + EXTRA, p_node);
    }
    fprintf(stderr, "%*s", offset, "");
    (*p_node)(tree->current);
    fprintf(stderr, "\n");

    if (tree->left != NULL) {
        bbst_dump_it(tree->left, offset + EXTRA, p_node);
    }
}

void  bbst_map(void *t, void (*map_func)(const void *node)) {
    t_node *tree = (t_node *)t;
    if (tree->right != NULL) {
        bbst_map(tree->right, map_func);
    }
    (*map_func)(tree->current);

    if (tree->left != NULL) {
        bbst_map(tree->left, map_func);
    }
}
