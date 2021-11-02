#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>
#include <stdio.h>

mutex_array_cell mutex_lock_array[MAX_MUTEX_LOCK];

mthread_mutex_t mutex_get(int key){
	//return 0 when failed
	int i;
	for(i = 1; i < MAX_MUTEX_LOCK; i++){
		if(!mutex_lock_array[i].key){
			mutex_lock_array[i].key = key;
			invoke_syscall(SYSCALL_MUTEX_INIT, (long)&(mutex_lock_array[i].lock_instance), IGNORE, IGNORE);
			return i;
		}
	}
	printf("[MTHREAD] Lock initialization failed\n");
	return 0;
}
int mutex_lock(mthread_mutex_t handle){
	if(handle < 1 || handle >= MAX_MUTEX_LOCK){
		printf("[MTHREAD] Invalid lock, initialize it before use\n");
		return 0;
	}
	invoke_syscall(SYSCALL_MUTEX_ACQUIRE, (long)&(mutex_lock_array[handle].lock_instance), IGNORE, IGNORE);
	return 1;
}
int mutex_unlock(mthread_mutex_t handle){
	if(handle < 1 || handle >= MAX_MUTEX_LOCK){
		printf("[MTHREAD] Lock not initialized, initialize it before use\n");
		return 0;
	}
	invoke_syscall(SYSCALL_MUTEX_RELEASE, (long)&(mutex_lock_array[handle].lock_instance), IGNORE, IGNORE);
	return 1;
}

int mthread_mutex_init(mthread_mutex_t* handle)
{
    /* TO DO: */
    //do_mutex_lock_init(handle);
    //modified to call syscalls
    //invoke_syscall(SYSCALL_MUTEX_INIT, handle, IGNORE, IGNORE);
    //modified to hide lock instance for user
    mthread_mutex_t *id = handle;
    *id = mutex_get((int)handle);
    return 1;
}
int mthread_mutex_lock(mthread_mutex_t* handle) 
{
    /* TO DO: */
    //do_mutex_lock_acquire(handle);
    //invoke_syscall(SYSCALL_MUTEX_ACQUIRE, handle, IGNORE, IGNORE);
    mthread_mutex_t id = *handle;
    return mutex_lock(id);
}
int mthread_mutex_unlock(mthread_mutex_t* handle)
{
    /* TO DO: */
    //do_mutex_lock_release(handle);
    //invoke_syscall(SYSCALL_MUTEX_RELEASE, handle, IGNORE, IGNORE);
    mthread_mutex_t id = *handle;
    return mutex_unlock(id);
}

int mthread_barrier_init(void* handle, unsigned count)
{
    // TODO:
    ;
}
int mthread_barrier_wait(void* handle)
{
    // TODO:
    ;
}
int mthread_barrier_destroy(void* handle)
{
    // TODO:
    ;
}

int mthread_semaphore_init(void* handle, int val)
{
    // TODO:
    ;
}
int mthread_semaphore_up(void* handle)
{
    // TODO:
    ;
}
int mthread_semaphore_down(void* handle)
{
    // TODO:
    ;
}
int mthread_semaphore_destroy(void* handle)
{
    // TODO:
    ;
}
