#include <os/list.h>
#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/irq.h>
#include <os/string.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>

extern void ret_from_exception();

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

//priority scheduler
unsigned int cal_pcb_weight(pcb_t *pcb){
	return (pcb->priority << 2) * TIMER_INTERVAL + get_ticks() - pcb->sched_time;
}

list_node_t *find_next_proc(){
	unsigned int max_weight = 0, temp_weight;
	list_node_t * node, * max_node = NULL;
    if(ready_queue.prev == &ready_queue){
    	return ;
    }
    //find the proc with largest weight
	for(node = ready_queue.prev; node != &ready_queue; node = node->prev){
		temp_weight = cal_pcb_weight(LIST_TO_PCB(node));
		if(temp_weight > max_weight){
			max_node = node;
			max_weight = temp_weight;
		}
	}
	return max_node;
}

void do_scheduler(void)
{
    // TO DO schedule
    // Modify the current_running pointer.
    pcb_t *last_run = current_running;
    //modified to use priorities
    current_running->sched_time = get_ticks();
    if(ready_queue.prev == &ready_queue){
    	return ;
    }
    //enqueue to head.next, dequeue from head.prev
    //list_node_t *last_list = ready_queue.prev;
    //modified to use priorities
    list_node_t *last_list = find_next_proc();
    if(!last_list){
    	last_list = &(current_running->list);
    }
    
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
     list_add(&(current_running->list), &sleep_queue);
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
    //do_scheduler();
}

void do_priority(task_priority_t priority)
{
	if(priority < 0 || priority > 9){
		printk("> [WARNING] Priority invalid, default to zero.\n\t");
		priority = 0;
	}
	current_running->priority = priority;
}

int do_fork(){
	int pid;
	for(pid = 1; pid <= NUM_MAX_TASK; pid++){
		if(pcb[pid - 1].status == TASK_EXITED)
			break;
	}
	if(pid > NUM_MAX_TASK){
		printk("> [FORK] Pcb allocation error\n\t");
		return -1;
	}
	int i = pid - 1;
	//copy pcb
    pcb[i].pid = pid;
    pcb[i].kernel_sp = allocPage(1);
    pcb[i].user_sp = allocPage(1);
    pcb[i].preempt_count = current_running->preempt_count;
    pcb[i].type = current_running->type;
   	pcb[i].status = TASK_READY;
   	pcb[i].cursor_x = current_running->cursor_x;
   	pcb[i].cursor_y = current_running->cursor_y;
   	pcb[i].wake_up_time = current_running->wake_up_time;
   	pcb[i].priority = current_running->priority;
   	pcb[i].sched_time = current_running->sched_time;
   	//copy stack
   	regs_context_t *pt_regs =
    	(regs_context_t *)(pcb[i].kernel_sp - sizeof(regs_context_t));
    switchto_context_t *switchto_regs =
    	(switchto_context_t *)(pcb[i].kernel_sp - sizeof(regs_context_t) - sizeof(switchto_context_t));
   	//for debug
   	regs_context_t *pt_regs_c =
    	(regs_context_t *)(current_running->kernel_sp + sizeof(switchto_context_t));
    switchto_context_t *switchto_regs_c =
    	(switchto_context_t *)(current_running->kernel_sp - sizeof(regs_context_t) - sizeof(switchto_context_t));
    reg_t current_kStack_bottom = current_running->kernel_sp + sizeof(regs_context_t) + sizeof(switchto_context_t);
    kmemcpy(pcb[i].kernel_sp - PAGE_SIZE , current_kStack_bottom - PAGE_SIZE , PAGE_SIZE);
    kmemcpy(pcb[i].user_sp - PAGE_SIZE , current_kStack_bottom, PAGE_SIZE);
    //modify sp
    pcb[i].user_sp = (reg_t)(pcb[i].kernel_sp + (current_running->user_sp - current_kStack_bottom));
    pcb[i].kernel_sp = (reg_t)(pcb[i].kernel_sp - (current_kStack_bottom - current_running->kernel_sp));
   	//fix return value, tp, sp, fp in regs_context for child
   	pt_regs->regs[10] = 0;						//a0
   	pt_regs->regs[2] = (reg_t)pcb[i].user_sp;	//sp
   	pt_regs->regs[4] = (reg_t)(pcb + i);		//tp
   	pt_regs->regs[8] += (reg_t)(pcb[i].user_sp - current_running->user_sp);//fp
   	//switch_to context as well
   	switchto_regs->regs[0] = ret_from_exception;
   	//switchto_regs->regs[1] = pcb[i].user_sp;//sp
   	//user_sp is not right, since when doing switch_to, we are in kernel state,
   	//but current_running's kernel_sp wasn't stored when calling sys_fork, so use ret_from_exception
   	//switchto_regs->regs[2] = pt_regs->regs[8];//fp
   	list_add(&(pcb[i].list), &ready_queue);
   	return pid;
}
