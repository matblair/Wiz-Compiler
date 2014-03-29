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
    char        *str_val;
    Decl        *decl_val;
    Decls       *decls_val;
    Expr        *expr_val;
    ExprList    *expr_val_list;
    Stmts       *stmts_val;
    Stmt        *stmt_val;
    Program     *prog_val;
    Proc        *proc_val;
    Procs       *procs_val;
    Args        *args_val;
    Arg         *arg_val;
}

%token '(' ')' ';' '[' ']'
%token ASSIGN_TOKEN 
%token DO_TOKEN   
%token ELSE_TOKEN 
%token FALSE_TOKEN 
%token FI_TOKEN    
%token IF_TOKEN    
%token INT_TOKEN 
%token BOOL_TOKEN 
%token OR_TOKEN 
%token AND_TOKEN 
%token OD_TOKEN     
%token READ_TOKEN 
%token THEN_TOKEN 
%token TRUE_TOKEN 
%token WHILE_TOKEN 
%token WRITE_TOKEN
%token INVALID_TOKEN
%token PROC_TOKEN
%token END_TOKEN
%token FLOAT_TOKEN
%token NOT_TOKEN
%token VAL_TOKEN
%token REF_TOKEN
%token NOTEQ_TOKEN
%token LTEQ_TOKEN
%token GTEQ_TOKEN

%token <str_val> INT_NUMBER_TOKEN
%token <str_val> FLOAT_NUMBER_TOKEN
%token <str_val> IDENT_TOKEN

/* Standard operator precedence */

%left OR_TOKEN
%left AND_TOKEN
%nonassoc NOT_TOKEN
%nonassoc '=' NOTEQ_TOKEN '<' LTEQ_TOKEN '>' GTEQ_TOKEN
%left '+' '-' 
%left '*' '/'
%left UNARY_MINUS
%nonassoc INDIRECT_IDENT_DERIVATION
%nonassoc DIRECT_IDENT_DERIVATION

%type <prog_val>    program
%type <procs_val>   procedures
%type <proc_val>    procedure
%type <args_val>    argument_list
%type <args_val>    args
%type <arg_val>     arg

%type <decls_val>   declarations
%type <decl_val>    decl
%type <stmts_val>   statements 
%type <stmt_val>    stmt
%type <expr_val>    expr 
%type <expr_val>    bool_expr 
%type <expr_val>    num_expr
%type <exprl_val>   expr_list

%type <int_val>     assign
%type <int_val>     start_cond
%type <int_val>     start_read
%type <int_val>     start_while
%type <int_val>     start_write
%type <int_val>     get_lineno

%start program

%%
/*---------------------------------------------------------------------*/

program
    : procedures
        {
            parsed_program = allocate(sizeof(struct prog));
            parsed_program->proc_list = $1;
        }
    ;

procedures
    : procedure procedures
        {
            $$ = allocate(sizeof(struct procs));
            $$->first = $1;
            $$->rest = $2;
        }
    |
        { $$ = NULL; } /* base case */
    ;

procedure
    : PROC_TOKEN IDENT_TOKEN argument_list declarations statements END_TOKEN
        {
            $$ = allocate(sizeof(struct proc));
            $$->proc_name = $2;
            $$->args = $3;
            $$->decls = $4;
            $$->body = $5;
        }

    | error END_TOKEN { yerrok; } proc
        { 
            $$ = $4;
        }
    ;

argument_list
    : '(' arg args ')'
        {
            $$ = allocate(sizeof(struct args));
            $$->first = $2;
            $$->rest = $3
        }
    | '(' ')'
        { $$ = NULL }   /* empty header */
    ;

args
    : ',' arg args
        {
            $$ = allocate(sizeof(struct args));
            $$->first = $2;
            $$->rest = $3;
        }
    | 
        { $$ = NULL }
    ;

arg
    : VAL_TOKEN INT_TOKEN IDENT_TOKEN
        {
            $$ = allocate(sizeof(struct arg));
            $$->pp = PP_VAL;
            $$->type = INT_TYPE;
            $$->name = $3;
        }
    | REF_TOKEN INT_TOKEN IDENT_TOKEN
        {
            $$ = allocate(sizeof(struct arg));
            $$->pp = PP_REF;
            $$->type = INT_TYPE;
            $$->name = $3;
        }
    | VAL_TOKEN FLOAT_TOKEN IDENT_TOKEN
        {
            $$ = allocate(sizeof(struct arg));
            $$->pp = PP_VAL;
            $$->type = FLOAT_TYPE;
            $$->name = $3;
        }
    | REF_TOKEN FLOAT_TOKEN IDENT_TOKEN
        {
            $$ = allocate(sizeof(struct arg));
            $$->pp = PP_REF;
            $$->type = FLOAT_TYPE;
            $$->name = $3;
        }
    | VAL_TOKEN BOOL_TOKEN IDENT_TOKEN
        {
            $$ = allocate(sizeof(struct arg));
            $$->pp = PP_VAL;
            $$->type = BOOL_TYPE;
            $$->name = $3;
        }
    | REF_TOKEN BOOL_TOKEN IDENT_TOKEN
        {
            $$ = allocate(sizeof(struct arg));
            $$->pp = PP_REF;
            $$->type = BOOL_TYPE;
            $$->name = $3;
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
    : INT_TOKEN IDENT_TOKEN ';'
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
    ;

get_lineno
    : /* empty */
        { $$ = ln; }
    ;

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

    | array_expr assign expr ';'
        {
            $$ = allocate(sizeof(struct stmt));
            $$->lineno = $2;
            $$->kind = STMT_ARRAYASSIGN;
            $$->info.array_assign.array_access = $1;
            $$->info.array_assign.asg_expr = $3;
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

    | IDENT_TOKEN '(' expr_list ')' ';'  /* proc call with arguments */
        {
            $$ = allocate(sizeof(struct stmt));
            $$->kind = STMT_PROC;
            $$->info.proc.proc_name = $1;
            $$->info.proc.proc_args = $3;
        } 

    | IDENT_TOKEN '(' ')' ';' /* proc call with no arguments */
        {
            $$ = allocate(sizeof(struct stmt));
            $$->kind = STMT_PROC;
            $$->info.proc.proc_name = $1;
            $$->info.proc.proc_args = NULL;
        }

    | start_cond bool_expr THEN_TOKEN statements FI_TOKEN 
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_COND;
          $$->info.cond.cond = $2;
          $$->info.cond.then_branch = $4;
          $$->info.cond.else_branch = NULL;
        }

    | start_cond bool_expr THEN_TOKEN statements ELSE_TOKEN statements FI_TOKEN
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_COND;
          $$->info.cond.cond = $2;
          $$->info.cond.then_branch = $4;
          $$->info.cond.else_branch = $6;
        }

    | start_while bool_expr DO_TOKEN statements OD_TOKEN
        {
          $$ = allocate(sizeof(struct stmt));
          $$->lineno = $1;
          $$->kind = STMT_WHILE;
          $$->info.loop.cond = $2;
          $$->info.loop.body = $4;
        }
    ;

expr 
    : '(' expr ')'
        { $$ = $2; }

    | '(' error ')'
        { $$ = NULL; }

    | binop_expr
        { $$ = $1; }

    | num_expr
        { $$ = $1; }

    | IDENT_TOKEN            %prec DIRECT_IDENT_DERIVATION
        { 
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_ID;
          $$->id = $1;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }
    | array_expr
        {
            $$ = $1;
        }
    ;

array_expr
    : IDENT_TOKEN '[' expr_list ']'
        {
            $$ = allocate(sizeof(struct expr));
            $$->lineno = ln;
            $$->kind = EXPR_ARRAY;
            $$->id = $1;
            $$->el = $3;
        }
    ;

expr_list
    : expr /* list of size one */
        {
            $$ = allocate(sizeof(struct exprs));
            $$->first = $1;
            $$->rest = NULL;
        }
    | expr ',' expr_list
        {
            $$ = allocate(sizeof(struct exprs));
            $$->first = $1;
            $$->rest = $3;
        }
    ;

num_expr
    : '-' get_lineno num_expr                             %prec UNARY_MINUS
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_UNOP;
          $$->unop = UNOP_MINUS;
          $$->e1 = $3;
          $$->e2 = NULL;
          $$->lineno = $2;
        }

    | num_expr '+' get_lineno num_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_ADD;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | num_expr '-' get_lineno num_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_SUB;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | num_expr '*' get_lineno num_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_MUL;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | num_expr '/' get_lineno num_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_DIV;
          $$->e1 = $1;
          $$->e2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | '(' num_expr ')'
        { $$ = $2; }

    | '(' error ')'
        { $$ = NULL }

    | INT_NUMBER_TOKEN
        {
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_CONST;
          $$->constant.val.int_val = atoi($1);
          $$->constant.type = INT_TYPE;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }

| FLOAT_NUMBER_TOKEN
        {
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_CONST;
          $$->constant.val.float_val = atof($1);
          $$->constant.type = FLOAT_TYPE;
          $$->e1 = NULL;
          $$->e2 = NULL;
        

    | IDENT_TOKEN               %prec INDIRECT_IDENT_DERIVATION
        { 
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_ID;
          $$->id = $1;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }
    ;

bool_expr
    : TRUE_TOKEN
        {
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_CONST;
          $$->constant.val.bool_val = TRUE;
          $$->constant.type = BOOL_TYPE;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }

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

    | '(' bool_expr ')'
        { $$ = $2; }

    | '(' error ')'
        { $$ = NULL }

    | IDENT_TOKEN               %prec INDIRECT_IDENT_DERIVATION
        { 
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_ID;
          $$->id = $1;
          $$->e1 = NULL;
          $$->e2 = NULL;
        }

    | bool_expr '<' get_lineno bool_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_LT;
          $$->eq1 = $1;
          $$->eq2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | bool_expr '<' get_lineno bool_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_GT;
          $$->eq1 = $1;
          $$->eq2 = $4;   
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | bool_expr LTEQ_TOKEN get_lineno bool_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_LTEQ;
          $$->eq1 = $1;
          $$->eq2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | bool_expr GTEQ_TOKEN get_lineno bool_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_GTEQ;
          $$->eq1 = $1;
          $$->eq2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | bool_expr NOTEQ_TOKEN get_lineno bool_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_NOTEQ;
          $$->eq1 = $1;
          $$->eq2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | bool_expr '=' get_lineno bool_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_EQ;
          $$->eq1 = $1;
          $$->eq2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | bool_expr OR_TOKEN get_lineno bool_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_OR;
          $$->eq1 = $1;
          $$->eq2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | bool_expr AND_TOKEN get_lineno bool_expr
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_BINOP;
          $$->binop = BINOP_AND;
          $$->eq1 = $1;
          $$->eq2 = $4;
          $$->lineno = $1->lineno == $4->lineno ? $1->lineno : $3;
        }

    | NOT_TOKEN get_lineno bool_expr 
        {
          $$ = allocate(sizeof(struct expr));
          $$->lineno = ln;
          $$->kind = EXPR_UNOP;
          $$->unop = UNOP_NOT;
          $$->eq1 = $3;
          $$->eq2 = NULL;
        }
    ;
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

