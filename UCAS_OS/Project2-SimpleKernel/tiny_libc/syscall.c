#include <sys/syscall.h>
#include <stdint.h>
#include <os/sched.h>

void sys_sleep(uint32_t time)
{
    // TODO:
}

void sys_write(char *buff)
{
    // TODO:
}

void sys_reflush()
{
    // TODO:
}

void sys_move_cursor(int x, int y)
{
    // TODO:
    //only for part I test
    screen_move_cursor(x, y);
}

long sys_get_timebase()
{
    // TODO:
}

long sys_get_tick()
{
    // TODO:
}

void sys_yield()
{
    // TODO:
    // invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE);
    //   or
    do_scheduler();
    // ???
}
