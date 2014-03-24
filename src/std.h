/* std.h */

/*-----------------------------------------------------------------------
    Some typical macros. 
-----------------------------------------------------------------------*/

#ifndef STD_H
#define STD_H

#define max(a, b)     ((a) < (b) ? (b) : (a))
#define max3(a, b, c) (max((a), max((b), (c))))

#define streq(s1, s2) (strcmp((s1), (s2)) == 0)

#ifndef NULL
#define NULL    0
#endif
#ifndef NORMAL_EXIT
#define NORMAL_EXIT 0
#endif
#ifndef FALSE
#define FALSE ((BOOL) 0)
#endif
#ifndef TRUE
#define TRUE  ((BOOL) 1)
#endif
#ifndef BOOL
#define BOOL    char
#endif

#endif  /* STD_H */
