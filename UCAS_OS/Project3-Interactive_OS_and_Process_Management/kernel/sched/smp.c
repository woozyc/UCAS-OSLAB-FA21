#include <sbi.h>
#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>
#include <os/stdio.h>

//only visible to kernel
smp_array_cell smp_array[MAX_SMP];

void do_smp_init(smp_t *smp, int value)
{
    /* TO DO */
    smp->value = value;
    init_list_head(&smp->block_queue);
}

int do_smp_down(smp_t *smp)
{
    /* TO DO */
    int i = 0;
	while(smp->value == 0){
    	i++;
    	do_block(&(current_running->list), &smp->block_queue);
    }
    smp->value--;
    return i;
}

int do_smp_up(smp_t *smp)
{
    /* TO DO */
    int i = 0;
    if(smp->block_queue.prev != &smp->block_queue){
    	i++;
        do_unblock(smp->block_queue.prev);
    }
    smp->value++;
    return i;
}

//syscalls for user

int smp_get(int key, int value){
	//return 0 when failed
	int i;
	for(i = 1; i < MAX_SMP; i++){
		if(!smp_array[i].key){
			smp_array[i].key = key;
			do_smp_init(&(smp_array[i].smp_instance), value);
			return i;
		}
	}
	printk("> [SMP] Semaphore initialization failed\n\r");
	return 0;
}
int smp_down(int handle){
	if(handle < 1 || handle >= MAX_SMP || !smp_array[handle].key){
		printk("> [SMP] Invalid semaphore, initialize it before use\n\r");
		return 0;
	}
	do_smp_down(&(smp_array[handle].smp_instance));
	return 1;
}
int smp_up(int handle){
	if(handle < 1 || handle >= MAX_SMP || !smp_array[handle].key){
		printk("> [SMP] Invalid semaphore, initialize it before use\n\r");
		return 0;
	}
	do_smp_up(&(smp_array[handle].smp_instance));
	return 1;
}
int smp_destory(int handle){
	if(handle < 1 || handle >= MAX_SMP || !smp_array[handle].key){
		printk("> [SMP] Invalid semaphore, initialize it before use\n\r");
		return 0;
	}
	smp_array[handle].key = 0;
	return 1;
}

//only visible to kernel
barrier_array_cell barrier_array[MAX_BARRIER];

void do_barrier_init(barrier_t *barrier, int value){
	/* TO DO */
    barrier->value = value;
    barrier->waiting = 0;
    init_list_head(&barrier->block_queue);
}
void do_barrier_wait(barrier_t *barrier){
	/* TO DO */
	barrier->waiting++;
	if(barrier->waiting != barrier->value)
    	do_block(&(current_running->list), &(barrier->block_queue));
    else{
    	while(barrier->block_queue.next != &(barrier->block_queue))
			do_unblock(barrier->block_queue.next);
		barrier->waiting = 0;
	}
}

//syscalls for user

int barrier_get(int key, int value){
	//return 0 when failed
	int i;
	for(i = 1; i < MAX_BARRIER; i++){
		if(!barrier_array[i].key){
			barrier_array[i].key = key;
			do_barrier_init(&(barrier_array[i].barrier_instance), value);
			return i;
		}
	}
	printk("> [BARRIER] Barrier initialization failed\n\r");
	return 0;
}
int barrier_wait(int handle){
	if(handle < 1 || handle >= MAX_BARRIER || !barrier_array[handle].key){
		printk("> [BARRIER] Invalid barrier, initialize it before use\n\r");
		return 0;
	}
	do_barrier_wait(&(barrier_array[handle].barrier_instance));
	return 1;
}
int barrier_destory(int handle){
	if(handle < 1 || handle >= MAX_BARRIER || !barrier_array[handle].key){
		printk("> [BARRIER] Invalid barrier, initialize it before use\n\r");
		return 0;
	}
	barrier_array[handle].key = 0;
	return 1;
}




void lock_kernel()
{
    /* TODO: */
}

void unlock_kernel()
{
    /* TODO: */
}

