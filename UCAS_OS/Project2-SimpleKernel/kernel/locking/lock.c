#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

void do_mutex_lock_init(mutex_lock_t *lock)
{
    /* TO DO */
    lock->lock.status = UNLOCKED;
    init_list_head(&lock->block_queue);
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    /* TO DO */
    if(lock->lock.status == LOCKED)
        do_block(&(current_running->list), &lock->block_queue);
    else
        lock->lock.status = LOCKED;
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
    /* TO DO */
    if(lock->block_queue.prev == &lock->block_queue)//no pcb waiting
        lock->lock.status = UNLOCKED;
    else{
        do_unblock(lock->block_queue.prev);
    }
}
