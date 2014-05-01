/* oztree.h */

/*-----------------------------------------------------------------------------
 * Developed by: #undef TEAMNAME
 * Provides full tree implementation for the Oz assembly language.
 *---------------------------------------------------------------------------*/

#ifndef OZTREE_H
#define OZTREE_H

typedef enum {
    // Stack operations
    OP_PUSH_STACK_FRAME, OP_POP_STACK_FRAME,

    // End program
    OP_HALT,

    // Various assignments
    OP_LOAD, OP_STORE, OP_LOAD_ADDRESS, OP_LOAD_INDIRECT, OP_STORE_INDIRECT,
    OP_INT_CONST, OP_REAL_CONST, OP_STRING_CONST,
    OP_INT_TO_REAL, OP_MOVE,

    // Numeric ops
    OP_ADD_INT, OP_ADD_REAL, OP_ADD_OFFSET,
    OP_SUB_INT, OP_SUB_REAL, OP_SUB_OFFSET,
    OP_MUL_INT, OP_MUL_REAL, OP_DIV_INT, OP_DIV_REAL,
    
    // Comparison ops
    OP_CMP_EQ_INT,  OP_CMP_NE_INT,  OP_CMP_GT_INT,  OP_CMP_GE_INT,
    OP_CMP_LT_INT,  OP_CMP_LE_INT,  OP_CMP_EQ_REAL, OP_CMP_NE_REAL,
    OP_CMP_GT_REAL, OP_CMP_GE_REAL, OP_CMP_LT_REAL, OP_CMP_LE_REAL,

    OP_AND, OP_OR, OP_NOT,

    // Branching
    OP_BRANCH_ON_TRUE, OP_BRANCH_ON_FALSE, OP_BRANCH_UNCOND,

    // Function calling
    OP_CALL, OP_CALL_BUILTIN, OP_RETURN,

    // Debug
    OP_DEBUG_REG, OP_DEBUG_SLOT, OP_DEBUG_STACK
} OpCode;


// sections we'll see in code, and want to be able to comment

typedef enum {
    SECTION_PROLOGUE, SECTION_WRITE, SECTION_READ, SECTION_ASSIGN, SECTION_IF,
    SECTION_WHILE, SECTION_EPILOGUE
} OzCommentSection;

#define COMMENTSTRS "prologue", "write", "read", "assignment", "if",\
                    "while", "epilogue"

extern const char *sectionnames[];


// the builtin functions

typedef enum {
    BUILTIN_READ_INT,  BUILTIN_READ_REAL,  BUILTIN_READ_BOOL,
    BUILTIN_PRINT_INT, BUILTIN_PRINT_REAL, BUILTIN_PRINT_BOOL,
    BUILTIN_PRINT_STRING
} OzBuiltinId;

#define BUILTINNAMES "read_int",  "read_real",  "read_bool",\
                     "print_int", "print_real", "print_bool"\
                     "print_string"

extern const char *builtinnames[];



typedef enum {
    OZ_BUILTIN, OZ_PROC, OZ_LABEL, OZ_OP, OZ_COMMENT
} OzKind;

// how we represent which builtin we want to call
typedef struct {
    OzBuiltinId id;
} OzBuiltin;

// How we represent an Oz operation
typedef struct {
    OpCode code;
    void   *arg1;
    void   *arg2;
    void   *arg3;
} OzOp;

// A label for a section in code
typedef struct {
    int id;
} OzLabel;

// A label for a section in code
typedef struct {
    char *id;
} OzProc;

// Represent a comment indicating a section
typedef struct {
    OzCommentSection section;
} OzComment;

// One "line" in Oz code
typedef struct {
    OzKind kind;
    void   *val;
} OzLine;

// Linked list of ordered Oz lines
typedef struct oozlines {
    OzLine          *first;
    struct oozlines *rest;
} OzLines;

// Entire Oz program. Allow for skipping to end of linked list when compiling
typedef struct {
    OzLines *start;
    OzLines *end;
} OzProgram;


// Create an Oz program struct from a Wiz AST
OzProgram *gen_oz_program(Program *p, void *tables);

#endif /* OZTREE_H */
