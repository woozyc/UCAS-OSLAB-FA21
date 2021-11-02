#include <time.h>
#include <os/time.h>

clock_t clock()
{
    return (clock_t)sys_get_tick();
}

uint32_t do_getwalltime(uint32_t *time_elapsed){
	*time_elapsed = get_ticks();
	return time_base;
}
