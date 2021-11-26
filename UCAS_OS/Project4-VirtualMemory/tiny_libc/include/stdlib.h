#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>

#define RAND_MAX (INT32_MAX)

void srand(unsigned seed);
int rand();
int itoa(char *s);
long int atol ( const char * str );

#endif /* STDLIB_H */
