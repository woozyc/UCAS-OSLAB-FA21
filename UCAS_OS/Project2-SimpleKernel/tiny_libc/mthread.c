#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>


int mthread_mutex_init(void* handle)
{
    /* TO DO: */
    //do_mutex_lock_init(handle);
    //modified to syscalls
    invoke_syscall(SYSCALL_MUTEX_INIT, handle, IGNORE, IGNORE);
    return 0;
}
int mthread_mutex_lock(void* handle) 
{
    /* TO DO: */
    //do_mutex_lock_acquire(handle);
    invoke_syscall(SYSCALL_MUTEX_ACQUIRE, handle, IGNORE, IGNORE);
    return 0;
}
int mthread_mutex_unlock(void* handle)
{
    /* TO DO: */
    //do_mutex_lock_release(handle);
    invoke_syscall(SYSCALL_MUTEX_RELEASE, handle, IGNORE, IGNORE);
    return 0;
}
