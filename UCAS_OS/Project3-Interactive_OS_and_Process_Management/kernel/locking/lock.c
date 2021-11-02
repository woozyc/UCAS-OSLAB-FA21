#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>
#include <assert.h>

mutex_array_cell mutex_lock_array[MAX_MUTEX_LOCK];

void do_mutex_lock_init(mutex_lock_t *lock)
{
    /* TO DO */
    //assert_supervisor_mode();
    lock->lock.status = UNLOCKED;
    init_list_head(&lock->block_queue);
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    /* TO DO */
    while(lock->lock.status == LOCKED)
        do_block(&(current_running->list), &lock->block_queue);
    lock->lock.status = LOCKED;
    list_add(&(lock->owner_list), &(current_running->lock_list));
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
    /* TO DO */
    list_del(&(lock->owner_list));
    if(lock->block_queue.prev != &lock->block_queue){
        do_unblock(lock->block_queue.prev);
        //do_scheduler();
    }
    lock->lock.status = UNLOCKED;
}


int mutex_get(int key){
	//return 0 when failed
	int i;
	for(i = 1; i < MAX_MUTEX_LOCK; i++){
		if(!mutex_lock_array[i].key){
			mutex_lock_array[i].key = key;
			do_mutex_lock_init(&(mutex_lock_array[i].lock_instance));
			return i;
		}
	}
	printf("[MTHREAD] Lock initialization failed\n");
	return 0;
}
int mutex_lock(int handle){
	if(handle < 1 || handle >= MAX_MUTEX_LOCK){
		printf("[MTHREAD] Invalid lock, initialize it before use\n");
		return 0;
	}
	do_mutex_lock_acquire(&(mutex_lock_array[handle].lock_instance));
	return 1;
}
int mutex_unlock(int handle){
	if(handle < 1 || handle >= MAX_MUTEX_LOCK){
		printf("[MTHREAD] Lock not initialized, initialize it before use\n");
		return 0;
	}
	do_mutex_lock_release(&(mutex_lock_array[handle].lock_instance));
	return 1;
}

