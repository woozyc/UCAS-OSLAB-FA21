#include <sys/syscall.h>
#include <stdint.h>

void sys_sleep(uint32_t time)
{
    // TO DO:
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE, IGNORE);
}

void sys_yield()
{
    // TO DO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    // TO DO:
    invoke_syscall(SYSCALL_WRITE, (uintptr_t)buff, IGNORE, IGNORE, IGNORE);
}

char sys_read()
{
    return invoke_syscall(SYSCALL_READ, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    // TO DO:
    invoke_syscall(SYSCALL_CURSOR, (long)x, (long)y, IGNORE, IGNORE);
}

void sys_reflush()
{
    // TO DO:
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE, IGNORE);
}

long sys_get_timebase()
{
    // TO DO:
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick()
{
    // TO DO:
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_priority(task_priority_t priority)
{
    invoke_syscall(SYSCALL_PRIORITY, (long)priority, IGNORE, IGNORE, IGNORE);
}

uint32_t sys_get_wall_time(uint32_t *time_elapsed)
{
	return invoke_syscall(SYSCALL_GETWALLTIME, (long)time_elapsed, IGNORE, IGNORE, IGNORE);
}
//project3 added
char sys_serial_read()
{
    return invoke_syscall(SYSCALL_SERIAL_READ, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_serial_write(char *s)
{
    invoke_syscall(SYSCALL_SERIAL_WRITE, (long)s, IGNORE, IGNORE, IGNORE);
}

void sys_screen_clear()
{
    invoke_syscall(SYSCALL_SCREEN_CLEAR, IGNORE, IGNORE, IGNORE, IGNORE);
}

char sys_getchar()
{
	return invoke_syscall(SYSCALL_GET_CHAR, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_exit()
{
	invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sys_waitpid(pid_t pid)
{
	return invoke_syscall(SYSCALL_WAITPID, (long)pid, IGNORE, IGNORE, IGNORE);
}

pid_t sys_spawn(task_info_t *info, void* arg, spawn_mode_t mode, int hart_mask)
{
	return invoke_syscall(SYSCALL_SPAWN, (long)info, (long)arg, (long)mode, (long)hart_mask);
}

int sys_kill(pid_t pid)
{
	return invoke_syscall(SYSCALL_KILL, (long)pid, IGNORE, IGNORE, IGNORE);
}

pid_t sys_getpid()
{
	return invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_process_show()
{
    invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_setmask(int mask, int pid)
{
	invoke_syscall(SYSCALL_SETMASK, mask, pid, IGNORE, IGNORE);
}

pid_t sys_exec(const char* file_name, int argc, char* argv[], spawn_mode_t mode)
{
	return invoke_syscall(SYSCALL_EXEC, (long)file_name, (long)argc, (long)argv, (long)mode);
}

void* shmpageget(int key)
{
    return (void *)invoke_syscall(SYSCALL_SHMPGGET, (long)key, IGNORE, IGNORE, IGNORE);
}
void shmpagedt(void *addr)
{
    invoke_syscall(SYSCALL_SHMPGEDT, (long)addr, IGNORE, IGNORE, IGNORE);
}

int binsemget(int key)
{
	return invoke_syscall(SYSCALL_BINSHMPGET, (long)key, IGNORE, IGNORE, IGNORE);
}
int binsemop(int binsem_id, int op)
{
	return invoke_syscall(SYSCALL_BINSHMPOP, (long)binsem_id, (long)op, IGNORE, IGNORE);
}

void sys_exec_show()
{
	invoke_syscall(SYSCALL_EXECSHOW, IGNORE, IGNORE, IGNORE, IGNORE);
}
long sys_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength)
{
    return invoke_syscall(SYSCALL_NET_RECV, addr, length, num_packet, frLength);
}
void sys_net_send(uintptr_t addr, size_t length)
{
    invoke_syscall(SYSCALL_NET_SEND, addr, length, IGNORE, IGNORE);
}
void do_net_irq_mode(int mode)
{
    invoke_syscall(SYSCALL_NET_IRQ_MODE, mode, IGNORE, IGNORE, IGNORE);
}
/* do not use in project2
int sys_fork()
{
    return (int)invoke_syscall(SYSCALL_FORK, IGNORE, IGNORE, IGNORE);
}
*/
