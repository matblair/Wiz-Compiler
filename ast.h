/* ast.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Based on template code provided by Harald Sondergard for COMP90045.
    Provides full abstract syntax tree implementation for the Wiz 
    programming language. 

    Original message included as follows:

    "Definitions for the abstract syntax trees generated for Iz programs.
    For use in the COMP90045 project 2014.""
-----------------------------------------------------------------------*/

#ifndef AST_H
#define AST_H

#include "std.h"

/*-----------------------------------------------------------------------
    Typedefs included here to clarify types and for quick reference.
-----------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------
    Definitions for binary and unary operations as well as their 
    associated precedences. 
-----------------------------------------------------------------------*/

typedef enum {
    BINOP_ADD, BINOP_SUB, BINOP_MUL, BINOP_DIV,
    BINOP_OR, BINOP_AND,
    BINOP_EQ, BINOP_NTEQ, BINOP_LT, BINOP_LTEQ, BINOP_GT, BINOP_GTEQ
} BinOp;

// Array Values
#define BINOP_NAMES "+","-","*","/","or","and","=","!=","<","<=",">",">="
#define BINOP_PRECEDENCE 4, 4, 5, 5, 0, 1, 3, 3, 3, 3, 3, 3

// External definitions for array access
extern const char *binopname[]; 
extern const int  binopprec[]; 


typedef enum {
    UNOP_MINUS, UNOP_NOT
} UnOp;

// Array Values
#define UNOP_NAMES "-", "not "
#define UNOP_PRECEDENCE 6, 2

// External definitions for array access
extern const char *unopname[];
extern const int  unopprec[];  

/*----------------------------------------------------------------------
    Definitions for types and argument types. 
-----------------------------------------------------------------------*/

typedef enum {
    VAL_IND, REF_IND
} ParamsInd;

// Array Values
#define VAL_NAMES "val", "ref"
// External definitions for array access
extern const char *valnames[];



typedef enum {
    BOOL_TYPE, INT_TYPE, FLOAT_TYPE, INT_ARRAY_TYPE, 
    FLOAT_ARRAY_TYPE, BOOL_ARRAY_TYPE, STRING_CONST
} Type;

// Array Values
#define TYPE_NAMES "bool", "int", "float", "int", "float", "bool"
// External definitions for array access
extern const char *typenames[];

/*----------------------------------------------------------------------
    Definitions for constants 
-----------------------------------------------------------------------*/

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


/*----------------------------------------------------------------------
    Definitions for expressions. Expressions are stored as a linked
    list of many expressions. Each expression has a type and associated
    value or expression to complete that type's requirements.
-----------------------------------------------------------------------*/

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
    Expr      *e1;          /* for unary and binary operators */
    Expr      *e2;          /* for binary operators */
    Exprs     *indices;     /* for arrays */
};

struct exprs {
    Expr      *first;
    Exprs     *rest;
};

/*----------------------------------------------------------------------
    Definitions for declarations. Decls are stored as a linked list
    of many declarations
-----------------------------------------------------------------------*/

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

// Used for array interval declaration, only ever used for declarations, 
// not for array indice access and therefore should be grouped with the 
// declarations.

struct intervals {
    Interval  *first;
    Intervals *rest;
};

struct interval {
    int  upper;
    int  lower;
};

/*----------------------------------------------------------------------
    Definitions for statements. Each statement has a type as well as
    internal statements, expressions or values as required.
-----------------------------------------------------------------------*/

typedef enum {
    STMT_ASSIGN, STMT_COND, STMT_READ, STMT_WHILE, STMT_WRITE, STMT_FUNC
} StmtKind;

typedef struct {
    Expr      *asg_ident;
    Expr      *asg_expr;
} Assign;

typedef struct {
    Expr      *cond;
    Stmts     *then_branch;
    Stmts     *else_branch;
} Cond;

// An array consisters of an identifier to later be used to access the symbol
// table for information about the array, followed by a series of expressions
// to denote indices for access to a value. 
struct array {
    char   *id;
    Exprs  *indices;
};

// A while loop contains a condition that must be met, followed by
// the body of cody that is run if that condition is satisfied. 
typedef struct {
    Expr      *cond;
    Stmts     *body;
} While;


// A function consists of an identier, follow by a linked list of expressions
// that are arguments to that function. 
struct func {
    char  *id;
    Exprs *args;
};


// SInfo stores the required information to describe a statement in the 
// program. It can be, at most, one of the following seven types: An assignment
// a condition (if etc), a read expression, a write expression, a function
// a while loop or a further list of statements. 

typedef union {
    Assign    assign;
    Stmts     *stmts;
    Cond      cond;
    Expr      *read;
    Expr      *write;
    Function  *func;
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

/*----------------------------------------------------------------------
    Definitions for procs which consist of a header and a body. Each
    header consists of a set of parameters as well as a name. Each
    body is represented by a sequence of non-empty statements (Proc)
-----------------------------------------------------------------------*/

struct param {
    ParamsInd ind;
    Type      type;
    char      *id;
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
    Decls    *decls;
    Stmts    *statements;
};

struct proc {
    int      lineno;
    Header   *header;
    Body     *body;
};

struct procs {
    Proc     *first;
    Procs    *rest;
};

/*----------------------------------------------------------------------
    Definitions for a program. In our case our program is quite
    straightforward, a linked list of procedures but this is where
    we would add global variables or included files if we were to 
    extend the language. 
-----------------------------------------------------------------------*/

struct prog {
    Procs   *procedures;
};

/*----------------------------------------------------------------------*/


#endif /* AST_H */
