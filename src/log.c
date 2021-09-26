#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "log.h"

#define LOG_BUF_SZ (256)

void info(const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_BUF_SZ];

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SZ, fmt, ap);
    va_end(ap);
    fprintf(stdout, "\033[1;34m[*]\033[0m %s\n", buf);
}

void err(const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_BUF_SZ];

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SZ, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\033[1;31m[!]\033[0m %s\n", buf);
}
