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
    invoke_syscall(SYSCALL_CURSOR, (long)x, (long)y, IGNORE);
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
    invoke_syscall(SYSCALL_PRIORITY, (long)priority, IGNORE, IGNORE);
}

uint32_t sys_get_wall_time(uint32_t *time_elapsed)
{
	return invoke_syscall(SYSCALL_GETWALLTIME, (long)time_elapsed, IGNORE, IGNORE);
}
//project3 added
char sys_serial_read()
{
    return invoke_syscall(SYSCALL_SERIAL_READ, IGNORE, IGNORE, IGNORE);
}

void sys_serial_write(char *s)
{
    invoke_syscall(SYSCALL_SERIAL_WRITE, (long)s, IGNORE, IGNORE);
}

void sys_screen_clear()
{
    invoke_syscall(SYSCALL_SCREEN_CLEAR, IGNORE, IGNORE, IGNORE);
}

char sys_getchar()
{
	return invoke_syscall(SYSCALL_GET_CHAR, IGNORE, IGNORE, IGNORE);
}

void sys_exit()
{
	invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE);
}

int sys_waitpid(pid_t pid)
{
	return invoke_syscall(SYSCALL_WAITPID, (long)pid, IGNORE, IGNORE);
}

pid_t sys_spawn(task_info_t *info, void* arg, spawn_mode_t mode)
{
	return invoke_syscall(SYSCALL_SPAWN, (long)info, (long)arg, (long)mode);
}

int sys_kill(pid_t pid)
{
	return invoke_syscall(SYSCALL_KILL, (long)pid, IGNORE, IGNORE);
}

pid_t sys_getpid()
{
	return invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE);
}

void sys_process_show()
{
    invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE);
}
/* do not use in project2
int sys_fork()
{
    return (int)invoke_syscall(SYSCALL_FORK, IGNORE, IGNORE, IGNORE);
}
*/
