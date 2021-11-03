#include <time.h>
#include <os/time.h>

clock_t clock()
{
    return (clock_t)sys_get_tick();
}

