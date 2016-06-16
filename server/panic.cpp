#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "panic.h"

void noreturn panic(const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);

    abort();
}
