#ifndef __fatal_c
#define __fatal_c

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void fatal(char *fmt, ...)
{
    fflush(stdout);
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "\n");
    // if (input) fprintf(stderr, "%s:%d: ", input->name, input->line);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

#endif // __fatal_c