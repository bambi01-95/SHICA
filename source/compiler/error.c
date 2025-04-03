#ifndef ERROR_C
#define ERROR_C
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//SHOULD BE APPLIED TO ALL FILES
#define ERROR_DEV "DVELOPER ERROR:"
#define ERROR_000 "unknown error:"
#define ERROR_001 "syntax error:"
#define ERROR_002 "type error:"

#define ERROR_UNDEF_01 "undefined variable:"
#define ERROR_UNDEF_02 "undefined function:"
#define ERROR_UNDEF_03 "undefined type:"
#define ERROR_UNDEF_04 "undefined event:"
#define ERROR_UNDEF_05 "undefined state:"



//SHOULD BE APPLIED ABOVE
void _fatal(char*s, int line,char *msg, ...)
{   
    printf("\x1b[31m%s line %d:\n",s,line);
    va_list ap;
    va_start(ap, msg);
    fprintf(stderr, "\n d-.-b ERROR:");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\x1b[0m\n");
    va_end(ap);
    exit(1);
}
#define fatal(...) _fatal(__FILE__,__LINE__,__VA_ARGS__)
#define fatal_cond(COND,...) ({ \
    if(COND){ \
        _fatal(__FILE__,__LINE__,__VA_ARGS__); \
    } \
})

#endif //ERROR_C