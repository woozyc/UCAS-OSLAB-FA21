#ifndef INCLUDE_STDIO_H_
#define INCLUDE_STDIO_H_

#include <stdarg.h>
#include <stddef.h>

int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list va);
char getchar(void);
void putchar(char c);

#endif
