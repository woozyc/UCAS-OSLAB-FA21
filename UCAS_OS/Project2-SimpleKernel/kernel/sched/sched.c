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

list_head ready_queue;

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    // TO DO schedule
    // Modify the current_running pointer.
    pcb_t *last_run = current_running;
    if(ready_queue.prev == &ready_queue){
    	return ;
    }
    //enqueue to head.next, dequeue from head.prev
    list_node_t *last_list = ready_queue.prev;
    list_del(last_list);
    current_running = LIST_TO_PCB(last_list);
    current_running->status = TASK_RUNNING;
    if(last_run->pid != 0 && last_run->status == TASK_RUNNING){//do not enqueue pid 0
    	last_run->status = TASK_READY;
    	list_add(&(last_run->list), &ready_queue);
    }
    //save screen cursor
    last_run->cursor_x = screen_cursor_x;
    last_run->cursor_y = screen_cursor_y;

    // restore the current_runnint's cursor_x and cursor_y
    vt100_move_cursor(current_running->cursor_x,
                      current_running->cursor_y);
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
    // TO DO: switch_to current_running
    switch_to(last_run, current_running);
}

void do_sleep(uint32_t sleep_time)
{
    // TO DO: sleep(seconds)
    // note: you can assume: 1 second = `timebase` ticks
    // 1. block the current_running
    // 2. create a timer which calls `do_unblock` when timeout
    // 3. reschedule because the current_running is blocked.
    current_running->status = TASK_BLOCKED;
    current_running->wake_up_time = get_ticks() + sleep_time * time_base;
    do_scheduler();
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TO DO: block the pcb task into the block queue
    list_del(pcb_node);
    list_add(pcb_node, queue);
    pcb_t *pcb = LIST_TO_PCB(pcb_node);
    pcb->status = TASK_BLOCKED;
    do_scheduler();
}

void do_unblock(list_node_t *pcb_node)
{
    // TO DO: unblock the `pcb` from the block queue
    list_del(pcb_node);
    list_add(pcb_node, &ready_queue);
    pcb_t *pcb = LIST_TO_PCB(pcb_node);
    pcb->status = TASK_READY;
    do_scheduler();
}
