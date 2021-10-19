#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>
#include <assert.h>

void do_mutex_lock_init(mutex_lock_t *lock)
{
    /* TO DO */
    assert_supervisor_mode();
    lock->lock.status = UNLOCKED;
    init_list_head(&lock->block_queue);
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    /* TO DO */
    while(lock->lock.status == LOCKED)
        do_block(&(current_running->list), &lock->block_queue);
    if(lock->lock.status != LOCKED)
        lock->lock.status = LOCKED;
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
    /* TO DO */
    if(lock->block_queue.prev != &lock->block_queue){
        do_unblock(lock->block_queue.prev);
        //do_scheduler();
    }
    lock->lock.status = UNLOCKED;
}
