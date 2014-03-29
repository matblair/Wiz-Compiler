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
#include "liz.h"

Program *parsed_program;
extern void    report_error_and_exit(const char *msg);
extern char    *yytext;

int ln = 1;
char *rule;
void yyerror(const char *msg);
void *allocate(int size);
int parse_string(char * inputstring);
extern void yy_delete_buffer(YY_BUFFER_STATE);
extern YY_BUFFER_STATE yy_scan_string(const char *string);
BOOL has_parse_error ;
%}

%union {
    int     int_val;
    char    *str_val;
    Decl    *decl_val;
    Decls   *decls_val;
    Expr    *expr_val;
    Stmts   *stmts_val;
    Stmt    *stmt_val;
    ProcDef *procdef_val;
    Proc    *proc_val;
    Procs   *procs_val;
    Program *prog_val;
    Arguments *args_val;
    Argument *arg_val;
    Constant *const_val;
    ArgType  argtype_val;
    Type     datatype_val;
    Dimensions *dims_val;
    Dimension *dim_val;

}

%token '(' ')' ';' ',' '[' ']' '.'
%token ASSIGN_TOKEN 
%token DO_TOKEN   
%token ELSE_TOKEN 
%token FALSE_TOKEN 
%token FI_TOKEN    
%token IF_TOKEN    
%token INT_TOKEN 
%token BOOL_TOKEN 
%token FLOAT_TOKEN
%token OD_TOKEN     
%token READ_TOKEN 
%token THEN_TOKEN 
%token TRUE_TOKEN 
%token WHILE_TOKEN 
%token WRITE_TOKEN
%token INVALID_TOKEN
%token VAL_TOKEN
%token REF_TOKEN
%token <const_val> NUMBER_TOKEN
%token <str_val> IDENT_TOKEN
%token PROC_TOKEN
%token END_TOKEN
/* Standard operator precedence */

%left '+' '-' 
%left '*'
%left UNARY_MINUS

%type <prog_val>  program
%type <procs_val> procs
%type <proc_val> proc
%type <procdef_val> procdef
%type <args_val> args
%type <arg_val> arg
%type <argtype_val> argtypeval
%type <decls_val> declarations
%type <decl_val>  decl
%type <datatype_val> datatype
%type <dim_val> dim
%type <dims_val> dims
%type <stmts_val> statements 
%type <stmt_val>  stmt
%type <expr_val>  expr 
%type <expr_val>  bool_expr 
%type <int_val>   assign
%type <int_val>   start_cond
%type <int_val>   start_read
%type <int_val>   start_while
%type <int_val>   start_write
%type <int_val>   get_lineno

%start program

%token-table

%%
/*---------------------------------------------------------------------*/

program
    : procs
        {
         parsed_program = allocate (sizeof (struct prog));          
         parsed_program->procs = $1;
        }
    ;

procs
    : proc  procs
        {
          $$ = allocate(sizeof (struct procs));
          $$->first = $1;
          $$->rest = $2;
        }
    |
      proc
        {
         $$ = allocate(sizeof (struct procs));
         $$->first = $1;
         $$->rest = NULL;
        }
    ;

proc   
    : procdef  declarations statements END_TOKEN
        { 
          $$ = allocate(sizeof(struct proc));
          $$->proc_def = $1;
          $$->decls = $2;
          $$->body = $3;
        }
    ;

procdef
    : PROC_TOKEN IDENT_TOKEN '('  args ')'
        {
           $$ = allocate(sizeof (struct procDef));
           $$->name =  $2;
           $$->arguments = $4;
        }
    | PROC_TOKEN IDENT_TOKEN '(' ')'
        {
           $$ = allocate(sizeof (struct procDef));
           $$->name =  $2;
           $$->arguments = NULL;
        }

    ;

args
    : arg  
        {
          $$ = allocate(sizeof (struct arguments));
          $$->first = $1;
          $$->rest = NULL;
        }

    |
      arg ','  args
        {
          $$ = allocate(sizeof (struct arguments));
          $$->first = $1;
          $$->rest = $3;
        }
    ;


arg
      :  argtypeval datatype IDENT_TOKEN 
        {
          $$ = allocate(sizeof (struct argument));
          $$->type = $2;
          $$->id = $3;
          $$->arg_type= $1;
          //TODO: remove this $$->dims = $5;

        }
    ;

argtypeval
    :   VAL_TOKEN
        {
          $$ = VAL;
        }
    | REF_TOKEN
        {
          $$ = REF;
        }
    ;


datatype
    : INT_TOKEN
        {
          $$ = INT_TYPE;
        }
    | BOOL_TOKEN
        {
          $$ = BOOL_TYPE;
        }
    | FLOAT_TOKEN
        {
          $$ = FLOAT_TYPE;

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
     :  datatype IDENT_TOKEN '[' dims ']' ';'
        {
          $$ = allocate(sizeof(struct decl));
          $$->lineno = ln;
          $$->id = $2;
          $$->type = $1;
          $$->dims = $4;
        }
     | datatype IDENT_TOKEN ';'
        {
          $$ = allocate(sizeof(struct decl));
          $$->lineno = ln;
          $$->id = $2;
          $$->type = $1;
        } 
    ;

dims 
    :  dim ',' dims
        {
           $$ = allocate(sizeof (struct dimensions));
           $$->first= $1;
           $$->rest = $3;
        }
    | dim
        {
          $$ = allocate(sizeof (struct dimensions));
          $$->first = $1;
          $$->rest = NULL;
        }
    ;

dim
    : NUMBER_TOKEN '.' '.' NUMBER_TOKEN
        {
          $$ = allocate(sizeof (struct dimension));
          $$->lb = $1->val.int_val;
          $$->ub = $4->val.int_val;
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
        { 
          $$ = $4; 
        }
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
    : '-' get_lineno expr                             %prec UNARY_MINUS
        {
          $$ = allocate(sizeof(struct expr));
          $$->kind = EXPR_UNOP;
          $$->unop = UNOP_MINUS;
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

    | '(' expr ')'
        { $$ = $2; }

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
          $$->constant = *$1;
          $$->constant.type = INT_TYPE;
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
    ;

%%

/*---------------------------------------------------------------------*/

void 
yyerror(const char *msg) {
    has_parse_error = 1;
    char *linebuf = "TODO Acutal lilne";
    fprintf(stderr, "**** Input line %d, %s, near `%s': %s\n",  ln,linebuf, yytext, msg);
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

int parse_file(FILE *fp, parserOutput *parser_output){
    int result ;
    int i=0, ttlen;
    has_parse_error = 0;
    yyin = fp;
    result = yyparse() || has_parse_error;
    
    parser_output->parsed_program = parsed_program;
    /*parser_output->token_table = allocate(sizeof(yytname)) ;
    ttlen = sizeof(yytname)/sizeof(char*); 
    for(i=0;i<ttlen-1; i++){
        printf("%d length of string %s, %lu with size %lu\n",i, yytname[i], strlen(yytname[i]),
        sizeof(yytname[i]));
        parser_output->token_table[i] =  allocate(sizeof(yytname[i]));
        //strncpy(parser_output->token_table[i], yytname[i], strlen(yytname[i]));
    }*/
    return result;

}

int parse_string(char *input_string){
    int result ;
    has_parse_error = 0;
    YY_BUFFER_STATE buffer = yy_scan_string(input_string);
    result = yyparse() || has_parse_error ;
    yy_delete_buffer(buffer);
    return result;
}
/*---------------------------------------------------------------------*/

