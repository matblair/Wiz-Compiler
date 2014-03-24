#ifndef MISSING_H
#define MISSING_H

/*
   bison fails to put these in parser.h:
*/

extern  int         yyparse(void);
extern  const char  *yyfile;
extern  int         yylinenum;
extern  int         yydebug;

/*
   Also missing:
*/

extern  int         yylex(void);
extern  FILE        *yyin;

/*
   stdio.h defines this only with some options, not with others:
*/

extern  int         fileno(FILE *);

#endif  /* MISSING_H */

