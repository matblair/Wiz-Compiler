/* piz.y */

/*-----------------------------------------------------------------------
    A bison syntax spec for Iz, a subset of Wiz.
    For use in the COMP90045 project 2014.
    Harald Sondergaard, March 2014.
-----------------------------------------------------------------------*/

%{

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "std.h"
#include "missing.h"

extern Program *parsed_program;
extern void    report_error_and_exit(const char *msg);
extern char    *yytext;

int ln = 1;
void yyerror(const char *msg);
void *allocate(int size);

%}

%union {
    int        int_val;
    float      float_val;
    char       *str_val;
    Decl       *decl_val;
    Decls      *decls_val;
    Expr       *expr_val;
    Stmts      *stmts_val;
    Stmt       *stmt_val;
    Program    *prog_val;
    Procs      *procs_val;
    Proc       *proc_val;
    Header     *header_val;
    Params     *params_val;
    Param      *param_val;
    Body       *body_val;
    Function   *func_val;
    Args       *args_val;
    Array      *array_val;
    ArrayElem  *aelem_val;
    ArrayElems *aelems_val;
}

%token '(' ')' ';' '[' ']'
%token ASSIGN_TOKEN 
%token AND_TOKEN
%token BOOL_TOKEN 
%token DO_TOKEN   
%token ELSE_TOKEN 
%token END_TOKEN
%token FALSE_TOKEN 
%token FI_TOKEN    
%token IF_TOKEN    
%token INT_TOKEN 
%token NOT_TOKEN
%token OD_TOKEN 
%token OR_TOKEN
%token PROC_TOKEN    
%token READ_TOKEN
%token REF_TOKEN 
%token THEN_TOKEN 
%token TRUE_TOKEN 
%token WHILE_TOKEN
%token VAL_TOKEN 
%token WRITE_TOKEN
%token INVALID_TOKEN
%token GT_TOKEN
%token GTEQ_TOKEN
%token LT_TOKEN
%token LTEQ_TOKEN
%token EQ_TOKEN
%token NOTEQ_TOKEN
%token INTERVAL_TOKEN
%token <int_val> NUMBER_TOKEN
%token <str_val> IDENT_TOKEN
%token <float_val> FLOAT_TOKEN

/* Standard operator precedence */
/* As taken from the provided list of operators */
/* on the assignment worksheet */
%left OR_TOKEN
%left AND_TOKEN
%left NOT_TOKEN
%nonassoc LT_TOKEN GT_TOKEN LTEQ_TOKEN GTEQ_TOKEN EQ_TOKEN NOTEQ_TOKEN
%left '+' '-' 
%left '*' '/'
%left UNARY_MINUS

%type <prog_val>       program
%type <procs_val>      procs
%type <proc_val>       proc
%type <header_val>     header
%type <body_val>       body
%type <params_val>     params
%type <param_val>      param
%type <decls_val>      declarations
%type <decl_val>       decl
%type <stmts_val>      statements 
%type <stmt_val>       stmt
%type <expr_val>       expr 
// %type <expr_val>       bool_expr 
%type <args_val>       args
%type <aelems_val>     arrayelems
%type <array_val>      array
%type <aelem_val>      arrayelem

%type <int_val>   assign
%type <int_val>   start_cond
%type <int_val>   start_read
%type <int_val>   start_while
%type <int_val>   start_write
%type <int_val>   get_lineno

%start program

%%
/*---------------------------------------------------------------------*/

program 
    : procs 
        { 
          parsed_program = allocate(sizeof(struct prog));
          parsed_program->procedures = $1;
        }
    ;

procs 
  : proc procs
      {
        $$ = allocate(sizeof(struct procs));
        $$->first = $1;
        $$->rest = $2;
      }
    ;
  | proc 
      {
        $$ = allocate(sizeof(struct procs));
        $$->first = $1;
        $$->rest = NULL;
      }

proc
  : PROC_TOKEN header body END_TOKEN
        {
          $$ = allocate(sizeof(struct proc));
          $$->header = $2;
          $$->body = $3;
        }
    ;

header 
    : IDENT_TOKEN '(' params ')'
        {
          $$ = allocate(sizeof(struct header));
          $$->id = $1;
          $$->params = $3;
        } 
      ;

params 
    : param ',' params
        {
          $$ = allocate(sizeof(struct params));
          $$->first = $1;
          $$->rest  = $3;
        }
    | param 
        {
          $$ = allocate(sizeof(struct params));
          $$->first = $1;
          $$->rest  = NULL;
        }
    |  /* Empty Params! */
        { $$ = NULL; }
    ;

param 
    : VAL_TOKEN FLOAT_TOKEN IDENT_TOKEN 
        {
          $$ = allocate(sizeof(struct param));
          $$->ind = VAL_IND;
          $$->type = FLOAT_TYPE;
          $$->id = $3;
        }
    | VAL_TOKEN BOOL_TOKEN IDENT_TOKEN 
        {
          $$ = allocate(sizeof(struct param));
          $$->ind = VAL_IND;
          $$->type = BOOL_TYPE;
          $$->id = $3;
        }
    | VAL_TOKEN INT_TOKEN IDENT_TOKEN 
        {
          $$ = allocate(sizeof(struct param));
          $$->ind = VAL_IND;
          $$->type = INT_TYPE;
          $$->id = $3;
        }
    | REF_TOKEN FLOAT_TOKEN IDENT_TOKEN 
        {
          $$ = allocate(sizeof(struct param));
          $$->ind = REF_IND;
          $$->type = FLOAT_TYPE;
          $$->id = $3;
        }
    | REF_TOKEN BOOL_TOKEN IDENT_TOKEN 
        {
          $$ = allocate(sizeof(struct param));
          $$->ind = REF_IND;
          $$->type = BOOL_TYPE;
          $$->id = $3;
        }
    | REF_TOKEN INT_TOKEN IDENT_TOKEN 
        {
          $$ = allocate(sizeof(struct param));
          $$->ind = REF_IND;
          $$->type = INT_TYPE;
          $$->id = $3;
        }
      ;

body 
  : declarations statements 
        {
          $$= allocate(sizeof(struct body));
          $$->decls = $1;
          $$->statements = $2;
        }
  ;


declarations
    : decl declarations
        {
          $$ = allocate(sizeof(struct decls));
          $$->first = $1;
          $$->rest = $2;
        }

    | /* empty */
        { $$ = NULL; }
    ;
        
decl
    : FLOAT_TOKEN array ';'
        {
          $$ = allocate(sizeof(struct decl));
          $$->lineno = ln;
          $$->id = NULL;
          $$->array = $2;
          $$->type = INT_ARRAY_TYPE;
        }
    | INT_TOKEN array ';'
        {
          $$ = allocate(sizeof(struct decl));
          $$->lineno = ln;
          $$->id = NULL;
          $$->array = $2;
          $$->type = FLOAT_ARRAY_TYPE;
        }
    | BOOL_TOKEN array ';'
        {
          $$ = allocate(sizeof(struct decl));
          $$->lineno = ln;
          $$->id = NULL;
          $$->array = $2;
          $$->type = BOOL_ARRAY_TYPE;
        }
    | INT_TOKEN IDENT_TOKEN ';'
        {
          $$ = allocate(sizeof(struct decl));
          $$->lineno = ln;
          $$->id = $2;
          $$->type = INT_TYPE;
        }

    | BOOL_TOKEN IDENT_TOKEN ';'
        {
          $$ = allocate(sizeof(struct decl));
          $$->lineno = ln;
          $$->id = $2;
          $$->type = BOOL_TYPE;
        }
    | FLOAT_TOKEN IDENT_TOKEN ';'
        {
          $$ = allocate(sizeof(struct decl));
          $$->lineno = ln;
          $$->id = $2;
          $$->type = FLOAT_TYPE;
        }
    ;



get_lineno
    : /* empty */
        { $$ = ln; }

assign
    : ASSIGN_TOKEN
        { $$ = ln; }

start_cond
    : IF_TOKEN
        { $$ = ln; }

start_read
    : READ_TOKEN
        { $$ = ln; }

start_while
    : WHILE_TOKEN
        { $$ = ln; }

start_write
    : WRITE_TOKEN
        { $$ = ln; }

statements                             /* non-empty list of statements */
    : stmt statements
        {
          $$ = allocate(sizeof(struct stmts));
          $$->first = $1;
          $$->rest = $2;
        }

    | stmt
        {
          $$ = allocate(sizeof(struct stmts));
          $$->first = $1;
          $$->rest = NULL;
        }

      | error ';' { yyerrok; } statements
        { $$ = $4; }
    ;

stmt
    : IDENT_TOKEN assign expr ';'                  /* assignment */
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $2;
          $$->kind = STMT_ASSIGN;
          $$->info.assign.asg_id = $1;
          $$->info.assign.asg_expr = $3;
        }

    | start_read IDENT_TOKEN ';'                   /* read command */
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_READ;
          $$->info.read = $2;
        }

    | start_write expr ';'                         /* write command */
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_WRITE;
          $$->info.write = $2;
        }

    | start_cond expr THEN_TOKEN statements FI_TOKEN 
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_COND;
          $$->info.cond.cond = $2;
          $$->info.cond.then_branch = $4;
          $$->info.cond.else_branch = NULL;
        }

    | start_cond expr THEN_TOKEN statements ELSE_TOKEN statements FI_TOKEN
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_COND;
          $$->info.cond.cond = $2;
          $$->info.cond.then_branch = $4;
          $$->info.cond.else_branch = $6;
        }

    | start_while expr DO_TOKEN statements OD_TOKEN
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_WHILE;
          $$->info.loop.cond = $2;
          $$->info.loop.body = $4;
        }
    | IDENT_TOKEN '(' args ')' ';' get_lineno
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $6;
          $$->kind = STMT_FUNC;
          $$->info.func = allocate(sizeof(struct func));
          $$->info.func->id   = $1;
          $$->info.func->args = $3;
        }
    | array ASSIGN_TOKEN expr ';' get_lineno
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $5;
          $$->kind = STMT_ARRAY_ASSIGN;
          $$->info.array = $1;
          $$->info.assign.asg_expr = $3;
        }
    | start_read array ';'                   /* read command */
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_READ_ARRAY;
          $$->info.array = $2;
        }
    ;

expr 
    : '-' get_lineno expr                             %prec UNARY_MINUS
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_UNOP;
          $$->unop = UNOP_MINUS;
          $$->e1 = $3;
          $$->e2 = NULL;
          $$->lineno = $2;
        }

    | NOT_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_UNOP;
          $$->unop = UNOP_NOT;
          $$->e1 = $3;
          $$->e2 = NULL;
          $$->lineno = $2;
        }

    | expr '+' get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_ADD;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | expr '-' get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_SUB;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | expr '*' get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_MUL;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | expr '/' get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_DIV;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | expr OR_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_OR;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | expr AND_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_AND;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | expr EQ_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_EQ;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | expr LT_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_LT;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }
    
    | expr NOTEQ_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_NTEQ;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }
    
    | expr LTEQ_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_LTEQ;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }
    
    | expr GT_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_GT;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }
    
    | expr GTEQ_TOKEN get_lineno expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_GTEQ;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | '(' expr ')'
        { $$ = $2;}

    | '(' error ')'
        { $$ = NULL; }

    | FALSE_TOKEN
        {
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_CONST;
          $$->constant.val.bool_val = FALSE;
          $$->constant.type = BOOL_TYPE;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }

    | TRUE_TOKEN
        {
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_CONST;
          $$->constant.val.bool_val = TRUE;
          $$->constant.type = BOOL_TYPE;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }

    | IDENT_TOKEN
        { 
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_ID;
          $$->id = $1;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }

    | NUMBER_TOKEN
        {
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_CONST;
          $$->constant.val.int_val = $1;
          $$->constant.type = INT_TYPE;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }
    | FLOAT_TOKEN
            {
              $$ = allocate(sizeof(struct expr));
              $$->lineno = ln;
              $$->kind = EXPR_CONST;
              $$->constant.val.float_val = $1;
              $$->constant.type = FLOAT_TYPE;
              $$->e1 = NULL;
              $$->e2 = NULL;
            }
        ;

args
    : expr ',' args 
        {
          $$ = allocate(sizeof(struct args));
          $$->first = $1;
          $$->rest  = $3;
        }
    | expr 
        {
          $$ = allocate(sizeof(struct args));
          $$->first = $1;
          $$->rest  = NULL;
        }
    | //  Empty args //
        { $$ = NULL; }
    ;

array
    : IDENT_TOKEN '[' arrayelems ']'
        {
          $$ = allocate(sizeof(struct array));
          $$->id = $1;
          $$->values = $3;
        }
    ;

arrayelems
    : arrayelem ',' arrayelems
        {
          $$ = allocate(sizeof(struct arrayelems));
          $$->first = $1;
          $$->rest = $3;
        }
    | arrayelem
        {
          $$ = allocate(sizeof(struct arrayelems));
          $$->first = $1;
          $$->rest = NULL;
        }
    |// Empty string //
        { $$ = NULL;}
    ;

arrayelem
    : NUMBER_TOKEN INTERVAL_TOKEN NUMBER_TOKEN
        {
          $$ = allocate(sizeof(struct arrayelem));
          $$->type = ARRAY_RANGE;
          $$->upper = $3;
          $$->lower = $1;
        }
    | NUMBER_TOKEN
        {
          $$ = allocate(sizeof(struct arrayelem));
          $$->type = ARRAY_INDEX;
          $$->index = $1;
        }
    ;
    
// bool_expr
//     : TRUE_TOKEN
//         {
//           $$ = allocate(sizeof(struct expr));
//           $$->lineno = ln;
//           $$->kind = EXPR_CONST;
//           $$->constant.val.bool_val = TRUE;
//           $$->constant.type = BOOL_TYPE;
//           $$->e1 = NULL;
//           $$->e2 = NULL;
//         }

//     | FALSE_TOKEN
//         {
//           $$ = allocate(sizeof(struct expr));
//           $$->lineno = ln;
//           $$->kind = EXPR_CONST;
//           $$->constant.val.bool_val = FALSE;
//           $$->constant.type = BOOL_TYPE;
//           $$->e1 = NULL;
//           $$->e2 = NULL;
//         }
//     ;
%%

/*---------------------------------------------------------------------*/

void 
yyerror(const char *msg) {
    fprintf(stderr, "**** Input line %d, near `%s': %s\n", ln, yytext, msg);
    return;
}

void *
allocate(int size) {

    void    *addr;

    addr = malloc((size_t) size);
    if (addr == NULL) 
        report_error_and_exit("Out of memory");
    return addr;
}

/*---------------------------------------------------------------------*/

