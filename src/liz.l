/* liz.l */

/*-----------------------------------------------------------------------
    A flex specification for Iz, a subset of Wiz.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

NUM     -?[0-9]+
ID      [a-zA-Z_][A-Za-z_0-9]*
nl      [\n\f]
nonl    [^\n\f]
FLOAT   -?[0-9]*\.[0-9]+
dim     [0-9]+\.\.[0-9]+
STRING  \"[^\n\t\"]*\"
%{

#include <stdlib.h>
#include <string.h>
#include "std.h"
#include "ast.h"
#include "piz.h"

#define YY_NO_UNPUT
int yylex(void);
int yywrap(void);

extern int ln;
extern void * allocate(int);
char  linebuf[LINELEN+1];
%}

%%

do              { yylval.str_val = yytext; return DO_TOKEN; }
else            { return ELSE_TOKEN; }
false           { return FALSE_TOKEN; }
fi              { return FI_TOKEN; }
if              { return IF_TOKEN; }
int             { return INT_TOKEN; }
bool            { return BOOL_TOKEN; }
float           { return FLOAT_TOKEN; }
od              { return OD_TOKEN; }
read            { return READ_TOKEN; }
then            { return THEN_TOKEN; }
true            { return TRUE_TOKEN; }
while           { return WHILE_TOKEN; }
write           { return WRITE_TOKEN; }
val             { return VAL_TOKEN; }
ref             { return REF_TOKEN; }
and             { return AND_TOKEN; }
or              { return OR_TOKEN; }
not             { return NOT_TOKEN; }
proc            { yylval.str_val = (char *) strdup(yytext);  return PROC_TOKEN; }
end             { yylval.str_val = (char *) strdup(yytext); return END_TOKEN; }

{NUM}           { yylval.const_val =  allocate(sizeof ( Constant)) ;
                  yylval.const_val->val.int_val = atoi(yytext); 
                  yylval.const_val->raw = (char *) strdup(yytext); 
                  return NUMBER_TOKEN; 
                }

{FLOAT}          { yylval.const_val =  allocate(sizeof ( Constant)) ;
                  yylval.const_val->val.float_val = atof(yytext); 
                  yylval.const_val->raw = (char *) strdup(yytext); 
                  return FLOATNUM_TOKEN; 
                }

{STRING}        { yylval.const_val =  allocate(sizeof ( Constant)) ;
                  yylval.const_val->raw = (char *) strdup(yytext); 
                  return STRING_TOKEN; 
                }
{dim}           {
                  char * cp_yytext, *pos_sep ;
                  yylval.dim_val= allocate(sizeof (Dimension));
                  pos_sep = strstr(yytext, "..");
                  yylval.dim_val->ub= atoi (pos_sep+2);
                  cp_yytext = strdup(yytext);
                  strncpy(cp_yytext, yytext, (strlen( yytext) -
                  strlen(pos_sep)));
                  yylval.dim_val->lb= atoi (cp_yytext);
                  return DIM_TOKEN;
                }

{ID}            { yylval.str_val = (char *) strdup(yytext); return IDENT_TOKEN; }

[-+*;(),/<>=]    { return yytext[0]; }         /* single character tokens */
[\[]             { return yytext[0]; }         /* single character tokens */
[\]]             { return yytext[0]; }         /* single character tokens */
[\.]             { return yytext[0]; }         /* single character tokens */

"!="            { return NEQ_TOKEN; }
"<="            { return LTE_TOKEN;}
">="            { return GTE_TOKEN; }

":="            { return ASSIGN_TOKEN; }

"#"{nonl}*{nl}  { ln++; }

{nl}            { ln++; }

[ \t]+          ; /* skip whitespace */

.               { return INVALID_TOKEN; }     /* parser may find a use for this */

\n.+            {   
                    ln++;
                    strncpy(linebuf, yytext+1, sizeof(linebuf)-sizeof(char)); /* save the next line */
                    yyless(1);      /* give back all but the \n to rescan */
                }
%%

int yywrap() {
    return 1;
}

