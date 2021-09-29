#include <os/list.h>
#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/irq.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack,
    .user_sp = (ptr_t)pid0_stack,
    .preempt_count = 0
};

LIST_HEAD(ready_queue);

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
    pcb *last_run = current_running;
    if(ready_queue.prev == ready_queue){
    	return ;
    }
    //enqueue to head.next, dequeue from head.prev
    list_node_t *last_list = ready_queue.prev;
    list_del(last_list);
    current_running = LIST_TO_PCB(last_list);
    current_running->status = TASK_RUNNING;
    if(*last_run != pid0_pcb){//do not enqueue pid 0
    	last_run->status = TASK_READY;
    	list_add(last_run, &ready_queue);
    }
    //save screen cursor
    last_run->cursor_x = screen_cursor_x;
    last_run->cursor_y = screen_cursor_y;

    // restore the current_runnint's cursor_x and cursor_y
    vt100_move_cursor(current_running->cursor_x,
                      current_running->cursor_y);
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
    // TODO: switch_to current_running
    switch_to(last_run, current_running);
}

void do_sleep(uint32_t sleep_time)
{
    // TODO: sleep(seconds)
    // note: you can assume: 1 second = `timebase` ticks
    // 1. block the current_running
    // 2. create a timer which calls `do_unblock` when timeout
    // 3. reschedule because the current_running is blocked.
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: block the pcb task into the block queue
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: unblock the `pcb` from the block queue
}
