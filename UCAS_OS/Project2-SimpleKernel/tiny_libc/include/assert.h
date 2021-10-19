#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>

static inline void _panic(const char* file_name,int lineno, const char* func_name)
{
    printf("Assertion failed at %s in %s:%d\n\r",
           func_name,file_name,lineno);
    for(;;);
}

static inline void assert_supervisor_mode(){
	__asm__ __volatile__("csrr x0, sscratch\n");
}

#define assert(cond)                                 \
    {                                                \
        if (!(cond)) {                               \
            _panic(__FILE__, __LINE__,__FUNCTION__); \
        }                                            \
    }

#endif /* ASSERT_H */
