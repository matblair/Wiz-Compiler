/* liz.l */

/*-----------------------------------------------------------------------
    A flex specification for Iz, a subset of Wiz.
    For use in the COMP90045 project 2014.
-----------------------------------------------------------------------*/

NUM     -?[0-9]+
DEC 	-?[0-9]+\`.[0-9]+
ID      [a-zA-Z_][A-Za-z_0-9]*
STRING  \"(\\.|[^"])*\"
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

and 			{ return AND_TOKEN;   }
bool 			{ return BOOL_TOKEN;  }
do              { return DO_TOKEN;    }
else            { return ELSE_TOKEN;  }
end 			{ return END_TOKEN;   }
false           { return FALSE_TOKEN; }
fi              { return FI_TOKEN;    }
if              { return IF_TOKEN;    }
int             { return INT_TOKEN;   }
float			{ return FLOAT_TOKEN; }
not 			{ return NOT_TOKEN;   }
od              { return OD_TOKEN;    }
or 				{ return OR_TOKEN;    }
proc            { return PROC_TOKEN;  }
read            { return READ_TOKEN;  }
ref				{ return REF_TOKEN;   }
then            { return THEN_TOKEN;  }
true            { return TRUE_TOKEN;  }
val 			{ return VAL_TOKEN;   }
while           { return WHILE_TOKEN; }
write           { return WRITE_TOKEN; }


{NUM}           { yylval.int_val = atoi(yytext); 
	              return NUMBER_TOKEN; }
{DEC}			{ yylval.int_val = atof(yytext); 
	              return FLOAT_TOKEN; }
{ID}            { yylval.str_val = (char *) strdup(yytext); 
				  return IDENT_TOKEN; }
{STRING}		 { yylval.str_val = (char *) strdup(yytext); 
				  return STRING_TOKEN; }



[-+*,;()\[\]""/]   { return yytext[0]; }         /* single character tokens */

":="            { return ASSIGN_TOKEN;   } /* An assignment */
".."			{ return INTERVAL_TOKEN; } /* An interval */
"<"				{ return LT_TOKEN;       } /* Less than  */
"<="			{ return LTEQ_TOKEN; 	 } /* Less than or equal to  */
">"				{ return GT_TOKEN;  	 } /* Greater than  */
">="			{ return GTEQ_TOKEN; 	 } /* Greater than or equal to */
"="				{ return EQ_TOKEN;   	 } /* Equal to */
"!="			{ return NOTEQ_TOKEN;	 } /* Not equal to */

"#"{nonl}*{nl}  { ln++; } /* Any comments */

{nl}            { ln++; }

[ \t]+          ; /* skip whitespace */

.               { return INVALID_TOKEN; }     /* parser may find a use for this */

%%

int yywrap() {
    return 1;
}

