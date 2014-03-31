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
typedef struct exprs        Exprs;
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
typedef struct intervals    Intervals;
typedef struct interval     Interval;

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
    BOOL_TYPE, INT_TYPE, FLOAT_TYPE, INT_ARRAY_TYPE, 
    FLOAT_ARRAY_TYPE, BOOL_ARRAY_TYPE, STRING_CONST
} Type;

#define TYPE_NAMES "bool", "int", "float", "int", "float", "bool"

extern const char *typenames[];

typedef union {
    int    int_val;
    BOOL   bool_val;
    float  float_val;
    char   *string;
} Value;

typedef struct {
    Type   type;
    Value  val;

} Constant;

typedef enum {
    EXPR_ID, EXPR_CONST, EXPR_BINOP, EXPR_UNOP, EXPR_ARRAY
} ExprKind;

struct expr {
    int       lineno;
    ExprKind  kind;           
    char      *id;          /* for identifiers */
    Constant  constant;     /* for constant values */
    UnOp      unop;         /* for unary operators */
    BinOp     binop;        /* for binary operators */
    Expr      *e1;          /* for unary, array index and binary operators */
    Expr      *e2;          /* for binary operators */
    Exprs     *indices;     /* for arrays */
};

struct exprs {
    Expr      *first;
    Exprs     *rest;
};

struct decl {
    int       lineno;
    char      *id;
    Type      type;
    Intervals *array;
};

struct decls {
    Decl      *first;
    Decls     *rest;
};

typedef enum {
    STMT_ASSIGN, STMT_COND, STMT_READ, STMT_WHILE, STMT_WRITE, STMT_FUNC, 
    STMT_ARRAY_ASSIGN, STMT_ARRAY_READ
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


struct intervals {
    Interval  *first;
    Intervals *rest;
};

struct interval {
    int          upper;
    int          lower;
};

struct array {
    char   *id;
    Exprs  *indices;
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
    char      *array_id;
    Expr      *write;
    Function  *func;
    While     loop;
    Exprs     *arrayinds;
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
    char  *id;
    Exprs *args;
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
