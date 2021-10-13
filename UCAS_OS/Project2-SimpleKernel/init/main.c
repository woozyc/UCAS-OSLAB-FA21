/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <common.h>
#include <os/irq.h>
#include <os/mm.h>
#include <os/sched.h>
#include <screen.h>
#include <sbi.h>
#include <stdio.h>
#include <os/time.h>
#include <os/syscall.h>
#include <os/list.h>
#include <os/lock.h>
#include <test.h>

#include <csr.h>

//0 for test_scheduler; 1 for test_lock; 2 for test_scheduler & lock; 3 for test_timer & sleep
//4 for test_scheduler & lock 2; 5 for test_timer & scheduler & lock;
int TEST_TASK = 4;

extern void ret_from_exception();
extern void __global_pointer$();

static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb)
{
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    
    /* TO DO: initialization registers
     * note: sp, gp, ra, sepc, sstatus
     * gp should be __global_pointer$
     * To run the task in user mode,
     * you should set corresponding bits of sstatus(SPP, SPIE, etc.).
     */
    //general-purpose registers
    int i;
    for(i = 0; i < 32; i++){
    	pt_regs->regs[i] = 0;
    }
    pt_regs->regs[2] = (reg_t)user_stack;
    pt_regs->regs[3] = (reg_t)__global_pointer$;
    pt_regs->regs[4] = (reg_t)pcb;
    //csr registers
    pt_regs->sepc = entry_point;
    pt_regs->sstatus = SR_SPIE & ~SR_SPP;
    pt_regs->sbadaddr = 0;
    pt_regs->scause = 0;

    // set sp to simulate return from switch_to
    /* TO DO: you should prepare a stack, and push some values to
     * simulate a pcb context.
     */
    switchto_context_t *switchto_regs =
    	(switchto_context_t *)(kernel_stack - sizeof(regs_context_t) - sizeof(switchto_context_t));
    for(i = 0; i < 14; i++){
    	switchto_regs->regs[i] = 0;
    }
    //init ra and sp, while other general purpose regs stay zero
    if (pcb->type == USER_PROCESS || pcb->type == USER_THREAD)
        switchto_regs->regs[0] = (reg_t)&ret_from_exception;
    else
	    switchto_regs->regs[0] = (reg_t)entry_point;
    switchto_regs->regs[1] = (reg_t)user_stack;
    pcb->kernel_sp = (reg_t)switchto_regs;
}

static void init_pcb()
{
     /* initialize all of your pcb and add them into ready_queue     */
     init_list_head(&ready_queue);
     int i;
     struct task_info *task;
     struct task_info **task_list;
     int task_num = 0;
     switch(TEST_TASK){
     	case 0:
     		task_num = num_sched1_tasks;
     		task_list = sched1_tasks; break;
     	case 1:
     		task_num = num_lock_tasks;
     		task_list = lock_tasks; break;
     	case 2:
     		task_num = num_sched1_lock_tasks;
     		task_list = sched1_lock_tasks; break;
     	case 3:
     		task_num = num_timer_tasks;
     		task_list = timer_tasks; break;
     	case 4:
     		task_num = num_sched2_lock_tasks;
     		task_list = sched2_lock_tasks; break;
     	case 5:
     		task_num = num_timer_sched2_lock_tasks;
     		task_list = timer_sched2_lock_tasks; break;
     	default:
     		printk("test task error\n");
     		while(1);
     }
     for(i = 0; i < task_num; i++){
     	//init a pcb of a task
     	task = task_list[i];
     	pcb[i].pid = i+1;
     	pcb[i].kernel_sp = allocPage(1);
     	pcb[i].user_sp = allocPage(1);
     	pcb[i].preempt_count = 0;
     	pcb[i].type = task->type;
     	pcb[i].status = TASK_READY;
     	pcb[i].cursor_x = 0;
     	pcb[i].cursor_y = 0;
     	pcb[i].wake_up_time = 0;
     	//init pcb stack
     	init_pcb_stack(pcb[i].kernel_sp, pcb[i].user_sp, task->entry_point, pcb + i);
     	//add to ready_queue
     	list_add(&(pcb[i].list), &ready_queue);
     }

    /* remember to initialize `current_running`*/
     current_running = &pid0_pcb;
}

static void err_syscall(int64_t num){
	printk("> [Error] Syscall number error.\n\r");
	while(1);
}

static void init_syscall(void)
{
    // initialize system call table.
    int i;
    for(i = 0; i < NUM_SYSCALLS; i++){
    	syscall[i] = (long int (*)())&err_syscall;
    }
    syscall[SYSCALL_SLEEP] = (long int (*)())&do_sleep;
    syscall[SYSCALL_YIELD] = (long int (*)())&do_scheduler;
    syscall[SYSCALL_MUTEX_INIT] = (long int (*)())&do_mutex_lock_init;
    syscall[SYSCALL_MUTEX_ACQUIRE] = (long int (*)())&do_mutex_lock_acquire;
    syscall[SYSCALL_MUTEX_RELEASE] = (long int (*)())&do_mutex_lock_release;
    syscall[SYSCALL_WRITE] = (long int (*)())&screen_write;
    //syscall[SYSCALL_READ] = ;
    syscall[SYSCALL_CURSOR] = (long int (*)())&screen_move_cursor;
    syscall[SYSCALL_REFLUSH] = (long int (*)())&screen_reflush;
    syscall[SYSCALL_GET_TIMEBASE] = (long int (*)())&get_time_base;
    syscall[SYSCALL_GET_TICK] = (long int (*)())&get_ticks;
    
    //init sleep_queue
	init_list_head(&sleep_queue);
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
int main()
{
    // init Process Control Block (-_-!)
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n\r");

    // read CPU frequency
    time_base = sbi_read_fdt(TIMEBASE);
    printk("> [INIT] Kernel running at a timebase of %d.\n\r", time_base);

    // init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n\r");
    //printk("> [INIT] Interrupt processing initialization skipped.\n\r");

    // init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n\r");
    //printk("> [INIT] System call initialization skipped.\n\r");

    // fdt_print(riscv_dtb);

    // init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n\r");
    //printk("> [INIT] SCREEN initialization skipped.\n\r");

    // TO DO:
    // Setup timer interrupt and enable all interrupt
    sbi_set_timer(get_ticks() + TIMER_INTERVAL);

    while (1) {
        // (QAQQQQQQQQQQQ)
        // If you do non-preemptive scheduling, you need to use it
        // to surrender control do_scheduler();
        enable_interrupt();
        __asm__ __volatile__("wfi\n\r":::);
        //do_scheduler();
    };
    return 0;
}
