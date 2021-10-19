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

char sys_read()
{
    return invoke_syscall(SYSCALL_READ, IGNORE, IGNORE, IGNORE);
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

void sys_priority(task_priority_t priority)
{
    invoke_syscall(SYSCALL_PRIORITY, priority, IGNORE, IGNORE);
}

uint32_t sys_get_wall_time(uint32_t *time_elapsed)
{
	return invoke_syscall(SYSCALL_GETWALLTIME, time_elapsed, IGNORE, IGNORE);
}
/* do not use in project2
int sys_fork()
{
    return (int)invoke_syscall(SYSCALL_FORK, IGNORE, IGNORE, IGNORE);
}
*/
