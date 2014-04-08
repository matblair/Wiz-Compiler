/* ast.h */

/*-----------------------------------------------------------------------
    Definitions for the abstract syntax trees generated for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#ifndef AST_H
#define AST_H

#include "std.h"

typedef enum {
    PP_VAL, PP_REF
} ParamMode;

typedef struct decl   Decl;
typedef struct decls  Decls;
typedef struct bounds Bounds;
typedef struct exprs  ExprList;
typedef struct expr   Expr;
typedef struct stmts  Stmts;
typedef struct stmt   Stmt;
typedef struct arg    Arg;
typedef struct args   Args;
typedef struct procs  Procs;
typedef struct proc   Proc;
typedef struct prog   Program;

typedef enum {
    BINOP_ADD, BINOP_SUB, BINOP_MUL, BINOP_DIV,
    BINOP_LT, BINOP_GT, BINOP_LTEQ, BINOP_GTEQ,
    BINOP_NOTEQ, BINOP_EQ, BINOP_OR, BINOP_AND
} BinOp;

#define BINOP_NAMES "+", "-", "*", "/"

extern const char *binopname[]; 

typedef enum {
    UNOP_MINUS, UNOP_NOT
} UnOp;

#define UNOP_NAMES "-"

extern const char *unopname[];

typedef enum {
    BOOL_TYPE, INT_TYPE, FLOAT_TYPE
} Type;

typedef union {
    int    int_val;
    double float_val;
    BOOL   bool_val;
} Value;

typedef struct {
    Type   type;
    Value  val;
} Constant;

typedef enum {
    EXPR_ID, EXPR_CONST, EXPR_BINOP, EXPR_UNOP, EXPR_ARRAY, EXPR_STR
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
    ExprList  *el;          /* for list of indices in array expressions */
};

struct exprs {
    Expr      *first;
    ExprList  *rest;
};

struct decl {
    int       lineno;
    char      *id;
    Type      type;
    Bounds    *array_bounds;
};

struct decls {
    Decl      *first;
    Decls     *rest;
};

struct bounds {
    int       interval_start;
    int       interval_end;
    Bounds    *rest;
};

typedef enum {
    STMT_ASSIGN, STMT_COND, STMT_READ,
    STMT_WHILE, STMT_WRITE, STMT_PROC
} StmtKind;

typedef struct {
    Expr      *asg_id; /* either an ID expression or an array expression */
    Expr      *asg_expr;
} Assign;

typedef struct {
    char      *proc_name;
    ExprList  *proc_args;
} ProcCall;

typedef struct {
    Expr      *cond;
    Stmts     *then_branch;
    Stmts     *else_branch;
} Cond;

typedef struct {
    Expr      *cond;
    Stmts     *body;
} While;

typedef union {
    Assign    assign;
    Stmts     *stmts;
    ProcCall  proc;
    Cond      cond;
    Expr      *read;
    Expr      *write;
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

struct arg {
    ParamMode pp;
    Type      type;
    char      *arg_name;
};

struct args {
    Arg       *first;
    Args      *rest;
}; 

struct proc {
    char      *proc_name;
    Args      *args;
    Decls     *decls;
    Stmts     *body;
};

struct procs {
    Proc      *first;
    Procs     *rest;
};

struct prog {
    Procs     *proc_list;
};

#endif /* AST_H */
