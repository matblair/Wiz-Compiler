/* liz.l */

/*-----------------------------------------------------------------------
    A flex specification for Iz, a subset of Wiz.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

INT     [0-9]+
FLOAT   {INT}.[0-9]+
ID      [a-zA-Z_][A-Za-z_0-9]*
nl      [\n\f]
nonl    [^\n\f]

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

%}

%%

do              { return DO_TOKEN; }
else            { return ELSE_TOKEN; }
false           { return FALSE_TOKEN; }
fi              { return FI_TOKEN; }
if              { return IF_TOKEN; }
int             { return INT_TOKEN; }
bool            { return BOOL_TOKEN; }
or              { return OR_TOKEN; }
and             { return AND_TOKEN; }
od              { return OD_TOKEN; }
read            { return READ_TOKEN; }
then            { return THEN_TOKEN; }
true            { return TRUE_TOKEN; }
while           { return WHILE_TOKEN; }
write           { return WRITE_TOKEN; }
proc            { return PROC_TOKEN; }
end             { return END_TOKEN; }
float           { return FLOAT_TOKEN; }
not             { return NOT_TOKEN; }
val             { return VAL_TOKEN; }
ref             { return REF_TOKEN; }


{INT}           { yylval.str_val = (char *) strdup(yytext); return INT_NUMBER_TOKEN; }
{FLOAT}         { yylval.str_val = (char *) strdup(yytext); return FLOAT_NUMBER_TOKEN; }

{ID}            { yylval.str_val = (char *) strdup(yytext); return IDENT_TOKEN; }

[-+/*;()\[\]<>=,]        { return yytext[0]; }         /* single character tokens */

":="            { return ASSIGN_TOKEN; }
"!="            { return NOTEQ_TOKEN; }
"<="            { return LTEQ_TOKEN; }
">="            { return GTEQ_TOKEN; }
".."            { return DOTDOT_TOKEN; }

"#"{nonl}*{nl}  { ln++; }

{nl}            { ln++; }

[ \t]+          ; /* skip whitespace */

.               { return INVALID_TOKEN; }     /* parser may find a use for this */

%%

int yywrap() {
    return 1;
}

