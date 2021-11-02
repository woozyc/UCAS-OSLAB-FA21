#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>
#include <stdio.h>

int mthread_mutex_init(mthread_mutex_t* handle)
{
    /* TO DO: */
    //do_mutex_lock_init(handle);
    //modified to call syscalls
    //invoke_syscall(SYSCALL_MUTEX_INIT, handle, IGNORE, IGNORE);
    //modified to hide lock instance for user
    mthread_mutex_t *id = handle;
    *id = invoke_syscall(SYSCALL_MUTEX_INIT, (long)handle, IGNORE, IGNORE);
    return 1;
}
int mthread_mutex_lock(mthread_mutex_t* handle) 
{
    /* TO DO: */
    //do_mutex_lock_acquire(handle);
    //invoke_syscall(SYSCALL_MUTEX_ACQUIRE, handle, IGNORE, IGNORE);
    mthread_mutex_t id = *handle;
    return invoke_syscall(SYSCALL_MUTEX_ACQUIRE, (long)id, IGNORE, IGNORE);
}
int mthread_mutex_unlock(mthread_mutex_t* handle)
{
    /* TO DO: */
    //do_mutex_lock_release(handle);
    //invoke_syscall(SYSCALL_MUTEX_RELEASE, handle, IGNORE, IGNORE);
    mthread_mutex_t id = *handle;
    return invoke_syscall(SYSCALL_MUTEX_RELEASE, (long)id, IGNORE, IGNORE);
}
int mthread_mutex_destroy(int *handle)
{
	mthread_mutex_t id = *handle;
	return invoke_syscall(SYSCALL_MUTEX_DESTORY, (long)id, IGNORE, IGNORE);
}
int mthread_mutex_trylock(int *handle)
{
	mthread_mutex_t id = *handle;
	return invoke_syscall(SYSCALL_MUTEX_TRYLOCK, (long)id, IGNORE, IGNORE);
}

int mthread_barrier_init(int* handle, unsigned count)
{
    // TODO:
    ;
}
int mthread_barrier_wait(int* handle)
{
    // TODO:
    ;
}
int mthread_barrier_destroy(int* handle)
{
    // TODO:
    ;
}

int mthread_semaphore_init(int* handle, int val)
{
    // TODO:
    ;
}
int mthread_semaphore_up(int* handle)
{
    // TODO:
    ;
}
int mthread_semaphore_down(int* handle)
{
    // TODO:
    ;
}
int mthread_semaphore_destroy(int* handle)
{
    // TODO:
    ;
}
