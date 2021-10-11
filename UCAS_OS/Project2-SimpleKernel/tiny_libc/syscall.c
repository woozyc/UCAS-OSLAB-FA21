#include <sys/syscall.h>
#include <stdint.h>
#include <os/sched.h>

void sys_sleep(uint32_t time)
{
    // TO DO:
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE);
}

void sys_yield()
{
    // TO DO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    // TO DO:
    invoke_syscall(SYSCALL_WRITE, (uintptr_t)buff, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    // TO DO:
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE);
}

void sys_reflush()
{
    // TO DO:
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

long sys_get_timebase()
{
    // TO DO:
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick()
{
    // TO DO:
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE);
}

