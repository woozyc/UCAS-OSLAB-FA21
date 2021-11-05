#include <os/list.h>
#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/irq.h>
#include <os/string.h>
#include <os/stdio.h>
#include <screen.h>
//#include <stdio.h>
#include <assert.h>
#include <asm/regs.h>
#include <os/smp.h>

extern void ret_from_exception();

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack_0 = INIT_KERNEL_STACK + 2 * PAGE_SIZE;
pcb_t pid0_pcb_0 = {
    .pid = 0,
    .status = TASK_RUNNING,
    .kernel_sp = (ptr_t)(INIT_KERNEL_STACK + PAGE_SIZE - OFFSET_SIZE - SWITCH_TO_SIZE),
    .user_sp = (ptr_t)(pid0_stack_0),//fake user_stack, pid0 always run in s mode
    .preempt_count = 0
};
const ptr_t pid0_stack_1 = INIT_KERNEL_STACK + 4 * PAGE_SIZE;
pcb_t pid0_pcb_1 = {
    .pid = 0,
    .status = TASK_RUNNING,
    .kernel_sp = (ptr_t)(INIT_KERNEL_STACK + 3 * PAGE_SIZE - OFFSET_SIZE - SWITCH_TO_SIZE),
    .user_sp = (ptr_t)(pid0_stack_1),
    .preempt_count = 0
};

list_head ready_queue;

/* current running task PCB */
pcb_t ** current_running;
pcb_t * current_running_0;
pcb_t * current_running_1;


//priority scheduler
unsigned long cal_pcb_weight(pcb_t *pcb){
	//weight was designed carefully
	return (pcb->priority * time_base) / 64 + get_ticks() - pcb->sched_time;
}

list_node_t *find_next_proc(int hart_id){
	unsigned long max_weight = 0, temp_weight;
	list_node_t * node, * max_node = NULL;
    if(ready_queue.prev == &ready_queue){
    	return NULL;
    }
    //find the proc with largest weight
	for(node = ready_queue.prev; node != &ready_queue; node = node->prev){
		if(!((LIST_TO_PCB(node)->hart_mask >> hart_id) % 2))
			continue ;
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
    int hart_id = get_current_cpu_id();
    pcb_t *last_run;
    int switch_to_no_store = 0;
    current_running = hart_id ? &current_running_1 : &current_running_0;
    last_run = (*current_running);
    (*current_running)->sched_time = get_ticks();
    
    /*
    if((*current_running)->status == TASK_EXITED){//has been killed by another core
    	list_del(&((*current_running)->list));
		while((*current_running)->wait_list.next != &((*current_running)->wait_list))
			do_unblock((*current_running)->wait_list.next);
		while((*current_running)->lock_list.next != &((*current_running)->lock_list))
			do_mutex_lock_release((mutex_lock_t *)(*current_running)->lock_list.next);
		*current_running = hart_id ? &(pid0_pcb_1) : &(pid0_pcb_0);
		switch_to_no_store = 1;
    }else{
    	// TO DO schedule
    	// Modify the current_running pointer.
    	//assert_supervisor_mode();
    	last_run = (*current_running);
    	//modified to use priorities
    	(*current_running)->sched_time = get_ticks();
    	//enqueue to head.next, dequeue from head.prev
    	//list_node_t *last_list = ready_queue.prev;
    	//modified to use priorities
    	list_node_t *last_list = find_next_proc(hart_id);
    	if(!last_list){//no job to run
    		if((*current_running)->pid != 0 && (*current_running)->status != TASK_RUNNING){
	    		*current_running = hart_id ? &(pid0_pcb_1) : &(pid0_pcb_0);
    			switch_to(last_run, (*current_running), 0);
    		}
    		return ;
   		}else{
   			list_del(last_list);
   			(*current_running) = LIST_TO_PCB(last_list);
    	}
    	(*current_running)->status = TASK_RUNNING;
    	if(last_run->pid != 0 && last_run->status == TASK_RUNNING){//do not enqueue pid 0
    		last_run->status = TASK_READY;
    		list_add(&(last_run->list), &ready_queue);
    	}
    	//save screen cursor
    	//last_run->cursor_x = screen_cursor_x;
    		//last_run->cursor_y = screen_cursor_y;
	
    	// restore the current_runnint's cursor_x and cursor_y
    	vt100_move_cursor((*current_running)->cursor_x,
    	                  (*current_running)->cursor_y);
    	//screen_cursor_x = current_running->cursor_x;
    	//screen_cursor_y = current_running->cursor_y;
    }
    
    // TO DO: switch_to current_running
    switch_to(last_run, (*current_running), switch_to_no_store);
    */
    
    //find next process to run
    list_node_t *last_list = find_next_proc(hart_id);
    if(!last_list){										//ready_queue empty
    	if((*current_running)->status == TASK_EXITED){	//exited, free sources and switch to pid 0
			switch_to_no_store = 1;						//do not store killed regs when switching
    		list_del(&((*current_running)->list));
			while((*current_running)->wait_list.next != &((*current_running)->wait_list))
				do_unblock((*current_running)->wait_list.next);
			while((*current_running)->lock_list.next != &((*current_running)->lock_list))
				do_mutex_lock_release((mutex_lock_t *)(*current_running)->lock_list.next);
			*current_running = hart_id ? &(pid0_pcb_1) : &(pid0_pcb_0);
    	}else if((*current_running)->status == TASK_RUNNING){	//continue on current process
    		return ;
    	}else{	//blocked or zombie, switch to pid 0
    		*current_running = hart_id ? &(pid0_pcb_1) : &(pid0_pcb_0);
    	}
    }else{	//found one ready process
    	list_del(last_list);
   		(*current_running) = LIST_TO_PCB(last_list);
    }
    //prepare to switch
    (*current_running)->status = TASK_RUNNING;
   	if(last_run->pid != 0 && last_run->status == TASK_RUNNING){//do not enqueue pid 0
   		last_run->status = TASK_READY;
    	list_add(&(last_run->list), &ready_queue);
    }
    
    //switch
    switch_to(last_run, (*current_running), switch_to_no_store);
}

void do_sleep(uint32_t sleep_time)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    // TO DO: sleep(seconds)
    // note: you can assume: 1 second = `timebase` ticks
    // 1. block the current_running
    // 2. create a timer which calls `do_unblock` when timeout
    // 3. reschedule because the current_running is blocked.
    (*current_running)->status = TASK_BLOCKED;
    (*current_running)->wake_up_time = get_ticks() + sleep_time * time_base;
     list_add(&((*current_running)->list), &sleep_queue);
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
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	if(priority < 0 || priority > 9){
		prints("> [WARNING] Priority invalid, default to zero.\n");
		priority = 0;
	}
	(*current_running)->priority = priority;
}

int do_fork(){
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	int pid;
	for(pid = 1; pid <= NUM_MAX_TASK; pid++){
		if(pcb[pid - 1].status == TASK_EXITED)
			break;
	}
	if(pid > NUM_MAX_TASK){
		prints("> [FORK] Pcb allocation error\n");
		return -1;
	}
	int i = pid - 1;
	//copy pcb
    pcb[i].pid = pid;
    pcb[i].kernel_sp = allocPage(1);
    pcb[i].user_sp = allocPage(1);
    pcb[i].preempt_count = (*current_running)->preempt_count;
    pcb[i].type = (*current_running)->type;
   	pcb[i].status = TASK_READY;
   	pcb[i].cursor_x = (*current_running)->cursor_x;
   	pcb[i].cursor_y = (*current_running)->cursor_y;
   	pcb[i].wake_up_time = (*current_running)->wake_up_time;
   	pcb[i].priority = (*current_running)->priority;
   	pcb[i].sched_time = (*current_running)->sched_time;
   	//copy stack
   	regs_context_t *pt_regs =
    	(regs_context_t *)(pcb[i].kernel_sp - sizeof(regs_context_t));
    switchto_context_t *switchto_regs =
    	(switchto_context_t *)(pcb[i].kernel_sp - sizeof(regs_context_t) - sizeof(switchto_context_t));
   	//for debug
   	/*
   	regs_context_t *pt_regs_c =
    	(regs_context_t *)(current_running->kernel_sp + sizeof(switchto_context_t));
    switchto_context_t *switchto_regs_c =
    	(switchto_context_t *)(current_running->kernel_sp - sizeof(regs_context_t) - sizeof(switchto_context_t));
    */
    reg_t current_kStack_bottom = (*current_running)->kernel_sp + sizeof(regs_context_t) + sizeof(switchto_context_t);
    kmemcpy((uint8_t *)(pcb[i].kernel_sp - PAGE_SIZE), (uint8_t *)(current_kStack_bottom - PAGE_SIZE), PAGE_SIZE);
    kmemcpy((uint8_t *)(pcb[i].user_sp   - PAGE_SIZE), (uint8_t *)(current_kStack_bottom            ), PAGE_SIZE);
    //modify sp
    pcb[i].user_sp = (reg_t)(pcb[i].kernel_sp + ((*current_running)->user_sp - current_kStack_bottom));
    pcb[i].kernel_sp = (reg_t)(pcb[i].kernel_sp - (current_kStack_bottom - (*current_running)->kernel_sp));
   	//fix return value, tp, sp, fp in regs_context for child
   	pt_regs->regs[10] = 0;						//a0
   	pt_regs->regs[2] = (reg_t)pcb[i].user_sp;	//sp
   	pt_regs->regs[4] = (reg_t)(pcb + i);		//tp
   	pt_regs->regs[8] += (reg_t)(pcb[i].user_sp - (*current_running)->user_sp);//fp
   	//switch_to context as well
   	switchto_regs->regs[0] = (reg_t)ret_from_exception;
   	//switchto_regs->regs[1] = pcb[i].user_sp;//sp
   	//user_sp is not right, since when doing switch_to, we are in kernel state,
   	//but current_running's kernel_sp wasn't stored when calling sys_fork, so use ret_from_exception
   	//switchto_regs->regs[2] = pt_regs->regs[8];//fp
   	list_add(&(pcb[i].list), &ready_queue);
   	return pid;
}

void do_ps(void){
	prints("[PROCESS TABLE]\n");
	int i, j;
	for(i = 0, j = 0; i < NUM_MAX_TASK; i++){
		if(pcb[i].status != TASK_EXITED){
			prints("[%d] PID : %d  STATUS : ", j, pcb[i].pid);
			switch(pcb[i].status){
				case TASK_BLOCKED:
					prints("BLOCKED "); break;
				case TASK_RUNNING:
					prints("RUNNING "); break;
				case TASK_READY:
					prints("READY   "); break;
				case TASK_ZOMBIE:
					prints("ZOMBIE  "); break;
				default:
					prints("UNKNOWN ");
			}
			prints("MASK : 0x%x\n", pcb[i].hart_mask);
			j++;
		}
	}
}

pid_t do_spawn(task_info_t *info, void* arg, spawn_mode_t mode, int hart_mask){
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	//TO DO:
	int i;
	for(i = 0; i < NUM_MAX_TASK; i++){
		if(pcb[i].status == TASK_EXITED){
     		pcb[i].preempt_count = 0;
    		pcb[i].type = info->type;
     		pcb[i].cursor_x = 0;
     		pcb[i].cursor_y = 0;
     		pcb[i].wake_up_time = 0;
     		pcb[i].priority = info->priority;
     		pcb[i].sched_time = get_ticks();
     		pcb[i].mode = mode;
     		//init stack
     		pcb[i].kernel_sp = pcb[i].kernel_stack_base;
     		pcb[i].user_sp = pcb[i].user_stack_base;
     		pcb[i].hart_mask = hart_mask ? hart_mask : (*current_running)->hart_mask;
			init_pcb_stack(pcb[i].kernel_sp, pcb[i].user_sp, info->entry_point, pcb + i, (ptr_t)arg);
     		init_list_head(&(pcb[i].wait_list));
     		init_list_head(&(pcb[i].lock_list));
     		//ready to run
     		pcb[i].status = TASK_READY;
     		list_add(&(pcb[i].list), &ready_queue);
     		return pcb[i].pid;
		}
	}
	prints("> [SPAWN] Pcb allocation error\n");
	return -1;
}

int do_kill(pid_t pid){
	//TO DO:
	int i = pid - 1;
	if(i < 0 || i >= NUM_MAX_TASK || pcb[i].status == TASK_EXITED){
		prints("> [KILL] Can not kill a process that doesn't exist\n");
		return 0;
	}
	if(i == 0){
		prints("> [KILL] Can not kill shell\n");
		return 0;
	}
	if(pcb[i].status != TASK_RUNNING){//not running on the other core
		list_del(&(pcb[i].list));
		while(pcb[i].wait_list.next != &(pcb[i].wait_list))
			do_unblock(pcb[i].wait_list.next);
		while(pcb[i].lock_list.next != &(pcb[i].lock_list))
			do_mutex_lock_release((mutex_lock_t *)pcb[i].lock_list.next);
	}
	pcb[i].status = TASK_EXITED;
	return 1;
}

void do_exit(void){
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	//TO DO:
	if((*current_running)->status != TASK_EXITED && (*current_running)->mode == ENTER_ZOMBIE_ON_EXIT){
		(*current_running)->status = TASK_ZOMBIE;
	}else{
		(*current_running)->status = TASK_EXITED;
	}
		list_del(&((*current_running)->list));
		while((*current_running)->wait_list.next != &((*current_running)->wait_list))
			do_unblock((*current_running)->wait_list.next);
		while((*current_running)->lock_list.next != &((*current_running)->lock_list))
			do_mutex_lock_release((mutex_lock_t *)(*current_running)->lock_list.next);
	do_scheduler();
}

int do_waitpid(pid_t pid){
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	//TO DO:
	int i = pid - 1;
	if(i < 0 || i >= NUM_MAX_TASK || pcb[i].status == TASK_EXITED){
		prints("> [WAIT] Can not wait for a process that doesn't exist\n");
		return 0;
	}
	if(i == 0){
		prints("> [WAIT] Can not wait for shell\n");
		return 0;
	}
	if(pcb[i].status == TASK_ZOMBIE){
		do_kill(pcb[i].pid);
		return 1;
	}
	do_block(&((*current_running)->list), &(pcb[i].wait_list));
	return 1;
}

pid_t do_getpid(void){
	//TO DO:
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	return (*current_running)->pid;
}

void do_setmask(int mask, int pid){
	//TO DO:
	pcb[pid-1].hart_mask = mask;
}
