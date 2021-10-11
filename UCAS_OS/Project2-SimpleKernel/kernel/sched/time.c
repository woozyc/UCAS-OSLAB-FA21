#include <os/time.h>
#include <os/mm.h>
#include <os/irq.h>
#include <os/list.h>
#include <type.h>

list_head sleep_queue;

uint64_t time_elapsed = 0;
uint32_t time_base = 0;

uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

uint64_t get_timer()
{
    return get_ticks() / time_base;
}

uint64_t get_time_base()
{
    return time_base;
}

void latency(uint64_t time)
{
    uint64_t begin_time = get_timer();

    while (get_timer() - begin_time < time);
    return;
}

void timer_check(){
	list_node_t *sleep_node = &sleep_queue;
	if(sleep_node->next == &sleep_queue)
		return ;
	long int current_tick;
	for(sleep_node = sleep_node->next; sleep_node != sleep_queue; ){
		sleep_node = sleep_node->next;
		current_tick = getticks();
		if(LIST_TO_PCB(sleep_node->prev)->wake_up_time <= current_tick){
			do_unblock(sleep_node->prev);
		}
	}
}
