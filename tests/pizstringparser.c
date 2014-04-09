#include "liz.h"

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "std.h"
#include "missing.h"

void *allocate(int size); 
extern void yy_delete_buffer(YY_BUFFER_STATE);
extern YY_BUFFER_STATE yy_scan_string(const char *string);
extern int yyparse();

//Internal variables used in piz.y
extern int ln;
extern BOOL has_parse_error;

int parse_string(char *input_string){
    int result ;
    int str_len = strlen(input_string);
    ln = 0;
    char * nl_pre_inputstr = allocate(sizeof (char)* ( str_len + 2) );
    nl_pre_inputstr[str_len+1] = '\0';
    nl_pre_inputstr[0]='\n';
    strncpy(nl_pre_inputstr+1, input_string, str_len);
    has_parse_error = 0;
    YY_BUFFER_STATE buffer = yy_scan_string(nl_pre_inputstr);
    result = yyparse() || has_parse_error ;
    yy_delete_buffer(buffer);
    free(nl_pre_inputstr);
    return result;
}

int wizparse_str(char * strinput){
    int result = parse_string(strinput);
    return  result;
}
