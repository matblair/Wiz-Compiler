/* ast.h */

/*-----------------------------------------------------------------------
    Definitions for the abstract syntax trees generated for Iz programs.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

#ifndef AST_H
#define AST_H

#include "std.h"

#define LINELEN 560

typedef struct idExprList IdExprList;
typedef struct dimensions Dimensions;
typedef struct exprList ExprList;
typedef struct dimension Dimension;
typedef struct decl   Decl;
typedef struct decls  Decls;
typedef struct expr   Expr;
typedef struct stmts  Stmts;
typedef struct stmt   Stmt;
typedef struct prog   Program;
typedef struct proc   Proc;
typedef struct procs  Procs;
typedef struct arguments Arguments;
typedef struct argument Argument;
typedef struct procDef  ProcDef;
typedef int bool;

typedef enum {
    BINOP_ADD, BINOP_SUB, BINOP_MUL, BINOP_DIV, BINOP_GT, BINOP_LT, BINOP_LTE, BINOP_GTE, BINOP_EQ, BINOP_NEQ, BINOP_OR, BINOP_AND
} BinOp;

#define BINOP_NAMES "+", "-", "*"

extern const char *binopname[]; 

typedef enum {
    UNOP_MINUS
} UnOp;

#define UNOP_NAMES "-"

extern const char *unopname[];

typedef enum {
    BOOL_TYPE, INT_TYPE, FLOAT_TYPE, STR_LITERAL
} Type;

typedef enum{
    VAL, REF
}ArgType;

typedef union {
    int    int_val;
    BOOL   bool_val;
    float  float_val;
} Value;

typedef struct {
    Type   type;
    Value  val;
    char*  raw;
} Constant;

typedef enum {
    EXPR_ID, EXPR_CONST, EXPR_BINOP, EXPR_UNOP
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
    Dimensions *dims;
};

struct decls {
    Decl      *first;
    Decls     *rest;
};

typedef enum {
    STMT_ASSIGN, STMT_COND, STMT_READ, STMT_WHILE, STMT_WRITE, STMT_FUNCCALL
} StmtKind;

struct idExprList {
    char * id;
    ExprList *expr_list;
};

typedef struct {
    IdExprList *id_expr_list;
    Expr      *asg_expr;
} Assign;

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
    Cond      cond;
    IdExprList *id_expr_list;
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

struct prog {
    Procs   *procs;
};

struct proc {
    ProcDef   *proc_def;
    Decls     *decls;
    Stmts     *body;
};

struct procDef {
    char* name;
    Arguments *arguments;
    
};

struct arguments {
    Argument *first;
    Arguments *rest;
};

struct argument {
    ArgType arg_type;
    Type type;
    char* id;
    Dimensions *dims;
};

struct procs {
    Proc    *first;
    Procs   *rest;
};

typedef struct{
    Program *parsed_program;
    char ** token_table;
} parserOutput;

struct dimension{
    int lb;
    int ub;
};

struct dimensions{
    Dimension *first;
    Dimensions *rest;
};

struct exprList {
    Expr *first;
    ExprList *rest;
};

#endif /* AST_H */
