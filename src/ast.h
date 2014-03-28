/* ast.h */

/*-----------------------------------------------------------------------
    Definitions for the abstract syntax trees generated for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#ifndef AST_H
#define AST_H

#include "std.h"

typedef struct decl         Decl;
typedef struct decls        Decls;
typedef struct expr         Expr;
typedef struct stmts        Stmts;
typedef struct stmt         Stmt;
typedef struct param        Param;
typedef struct params       Params;
typedef struct header       Header;
typedef struct body         Body;
typedef struct proc         Proc;
typedef struct procs        Procs;
typedef struct prog         Program;
typedef struct func         Function;
typedef struct args         Args;
typedef struct arrayelems   ArrayElems;
typedef struct arrayelem    ArrayElem;
typedef struct array        Array;

typedef enum {
    BINOP_ADD, BINOP_SUB, BINOP_MUL, BINOP_DIV, BINOP_OR, BINOP_AND,
    BINOP_EQ, BINOP_NTEQ, BINOP_LT, BINOP_LTEQ, BINOP_GTEQ, BINOP_GT
} BinOp;


#define BINOP_NAMES "+","-","*","/","or","and","=","!=","<","<=",">",">="

extern const char *binopname[]; 

typedef enum {
    VAL_IND, REF_IND
} ParamsInd;

#define VAL_NAMES "val", "ref"

extern const char *valnames[];


typedef enum {
    UNOP_MINUS, UNOP_NOT
} UnOp;

#define UNOP_NAMES "-", "not"

extern const char *unopname[];

typedef enum {
    ARRAY_RANGE, ARRAY_INDEX
} ArrayElemType;

typedef enum {
    BOOL_TYPE, INT_TYPE, FLOAT_TYPE, INT_ARRAY_TYPE, 
    FLOAT_ARRAY_TYPE, BOOL_ARRAY_TYPE
} Type;

#define TYPE_NAMES "bool", "int", "float", "int", "float", "bool"

extern const char *typenames[];

typedef union {
    int    int_val;
    BOOL   bool_val;
    float  float_val;
} Value;

typedef struct {
    Type   type;
    Value  val;
} Constant;

typedef enum {
    EXPR_ID, EXPR_CONST, EXPR_BINOP, EXPR_UNOP, EXPR_LIST
} ExprKind;

struct expr {
    int       lineno;
    ExprKind  kind;           
    char      *id;          /* for identifiers */
    Constant  constant;     /* for constant values */
    UnOp      unop;         /* for unary operators */
    BinOp     binop;        /* for binary operators */
    Expr      *e1;          /* for unary and binary operators */
    Expr      *e2;          /* for binary operators */
};

struct decl {
    int       lineno;
    char      *id;
    Type      type;
    Array     *array;
};

struct decls {
    Decl      *first;
    Decls     *rest;
};

typedef enum {
    STMT_ASSIGN, STMT_COND, STMT_READ, STMT_WHILE, STMT_WRITE, STMT_FUNC, 
    STMT_ARRAY_ASSIGN, STMT_ARRAY, STMT_READ_ARRAY
} StmtKind;

typedef struct {
    char      *asg_id;
    Expr      *asg_expr;
} Assign;

typedef struct {
    Expr      *cond;
    Stmts     *then_branch;
    Stmts     *else_branch;
} Cond;

struct array {
    char       *id;
    ArrayElems *values;
};

struct arrayelems {
    ArrayElem *first;
    ArrayElems *rest;
};

struct arrayelem {
    int          upper;
    int          lower;
    ArrayElemType type;
    int          index;
};

typedef struct {
    Expr      *cond;
    Stmts     *body;
} While;

typedef union {
    Assign    assign;
    Stmts     *stmts;
    Cond      cond;
    char      *read;
    Expr      *write;
    Function  *func;
    Array     *array;
    While     loop;
} SInfo;

struct stmt {
    int       lineno;
    StmtKind  kind;
    SInfo     info;
};

struct stmts {
    Stmt      *first;
    Stmts     *rest;
};

struct func {
    char *id;
    Args *args;
};

struct args {
   Expr       *first;
   Args       *rest;
};

struct param {
    ParamsInd ind;
    Type      type;
    char     *id;
};

struct params {
   Param     *first;
   Params    *rest;
};

struct header {
   char      *id;
   Params    *params;
};

struct body {
    Decls      *decls;
    Stmts      *statements;
};

struct proc {
    Header     *header;
    Body       *body;
};

struct procs {
    Proc      *first;
    Procs     *rest;
};

struct prog {
    Procs   *procedures;
    // Possibly something else here?
};


#endif /* AST_H */
