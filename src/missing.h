
/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME

    Provided with source package for COMP90045 Programming Language
    Implenetation, semester 1 2014. Original code provided below is 
    unmodified from that which was provided.

-----------------------------------------------------------------------*/
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

