/* helper.h */
/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME
    Based on template code provided by Harald Sondergard for COMP90045.
    
    Provides function definitions for all functions required throughout
    the compiler in multipl locations. Some funcitons taken from orig.
    scaffolding code in wiz.c

-----------------------------------------------------------------------*/

void    report_error_and_exit(const char *msg);  
void    *checked_malloc(size_t num_bytes);
