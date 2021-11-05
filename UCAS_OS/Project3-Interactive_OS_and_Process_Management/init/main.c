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
#include <os/stdio.h>
#include <os/time.h>
#include <os/syscall.h>
#include <os/list.h>
#include <os/lock.h>
#include <os/smp.h>
#include <os/mbox.h>
#include <test.h>

#include <sys/syscall.h>
#include <csr.h>

extern void ret_from_exception();
extern void __global_pointer$();

void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb, ptr_t args)
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
    pt_regs->regs[1] = (reg_t)&sys_exit;
    pt_regs->regs[2] = (reg_t)user_stack;
    pt_regs->regs[3] = (reg_t)__global_pointer$;
    pt_regs->regs[4] = (reg_t)pcb;
    pt_regs->regs[10] = (reg_t)args;
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
     //init a pcb of a task
     task = &task_test_shell;
     pcb[0].pid = 1;
     pcb[0].kernel_sp = allocPage(1);
     pcb[0].user_sp = allocPage(1);
     pcb[0].kernel_stack_base = pcb[0].kernel_sp;
     pcb[0].user_stack_base = pcb[0].user_sp;
     pcb[0].preempt_count = 0;
     pcb[0].type = task->type;
     pcb[0].status = TASK_READY;
     pcb[0].cursor_x = 0;
     pcb[0].cursor_y = 0;
     pcb[0].wake_up_time = 0;
     pcb[0].priority = task->priority;
     pcb[0].sched_time = get_ticks();
     pcb[0].mode = AUTO_CLEANUP_ON_EXIT;
     pcb[0].hart_mask = 3;
     init_list_head(&(pcb[0].wait_list));
     init_list_head(&(pcb[0].lock_list));
     //init pcb stack
     init_pcb_stack(pcb[0].kernel_sp, pcb[0].user_sp, task->entry_point, pcb, (ptr_t)NULL);
     //add to ready_queue
     list_add(&(pcb[0].list), &ready_queue);
     //init other pcbs
     for(i = 1; i < NUM_MAX_TASK; i++){
     	pcb[i].status = TASK_EXITED;
     	pcb[i].pid = i + 1;
     	pcb[i].kernel_sp = allocPage(1);
     	pcb[i].user_sp = allocPage(1);
     	pcb[i].kernel_stack_base = pcb[i].kernel_sp;
     	pcb[i].user_stack_base = pcb[i].user_sp;
     	init_list_head(&(pcb[i].wait_list));
     	init_list_head(&(pcb[i].lock_list));
     }

    /* remember to initialize `current_running`*/
     current_running_0 = &pid0_pcb_0;
     current_running_1 = &pid0_pcb_1;
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
    syscall[SYSCALL_SPAWN] = (long int (*)())do_spawn;
    syscall[SYSCALL_EXIT] = (long int (*)())do_exit;
    syscall[SYSCALL_SLEEP] = (long int (*)())&do_sleep;
    syscall[SYSCALL_KILL] = (long int (*)())&do_kill;
    syscall[SYSCALL_WAITPID] = (long int (*)())&do_waitpid;
    syscall[SYSCALL_PS] = (long int (*)())&do_ps;
    syscall[SYSCALL_GETPID] = (long int (*)())&do_getpid;
    syscall[SYSCALL_YIELD] = (long int (*)())&do_scheduler;
    syscall[SYSCALL_MUTEX_INIT] = (long int (*)())&mutex_get;
    //lock
    syscall[SYSCALL_MUTEX_ACQUIRE] = (long int (*)())&mutex_lock;
    syscall[SYSCALL_MUTEX_RELEASE] = (long int (*)())&mutex_unlock;
    syscall[SYSCALL_MUTEX_DESTORY] = (long int (*)())&mutex_destory;
    syscall[SYSCALL_MUTEX_TRYLOCK] = (long int (*)())&mutex_trylock;
    //semaphore
    syscall[SYSCALL_SMP_INIT] = (long int (*)())&smp_get;
    syscall[SYSCALL_SMP_DOWN] = (long int (*)())&smp_down;
    syscall[SYSCALL_SMP_UP] = (long int (*)())&smp_up;
    syscall[SYSCALL_SMP_DESTORY] = (long int (*)())&smp_destory;
    //barrier
    syscall[SYSCALL_BARRIER_INIT] = (long int (*)())&barrier_get;
    syscall[SYSCALL_BARRIER_WAIT] = (long int (*)())&barrier_wait;
    syscall[SYSCALL_BARRIER_DESTORY] = (long int (*)())&barrier_destory;
    
    syscall[SYSCALL_WRITE] = (long int (*)())&screen_write;
    syscall[SYSCALL_READ] = (long int (*)())&sbi_console_getchar;
    syscall[SYSCALL_CURSOR] = (long int (*)())&screen_move_cursor;
    syscall[SYSCALL_REFLUSH] = (long int (*)())&screen_reflush;
    syscall[SYSCALL_SERIAL_READ] = (long int (*)())&screen_read;
    syscall[SYSCALL_SERIAL_WRITE] = (long int (*)())&screen_write;
    syscall[SYSCALL_SCREEN_CLEAR] = (long int (*)())&screen_clear;
    
    syscall[SYSCALL_GET_TIMEBASE] = (long int (*)())&get_time_base;
    syscall[SYSCALL_GET_TICK] = (long int (*)())&get_ticks;
    
    syscall[SYSCALL_PRIORITY] = (long int (*)())&do_priority;
    syscall[SYSCALL_FORK] = (long int (*)())&do_fork;
    syscall[SYSCALL_GET_CHAR] = (long int (*)())&screen_getchar;
    syscall[SYSCALL_GETWALLTIME] = (long int (*)())&do_getwalltime;
    
    syscall[SYSCALL_MAILBOX_OPEN] = (long int (*)())&kernel_mbox_open;
    syscall[SYSCALL_MAILBOX_CLOSE] = (long int (*)())&kernel_mbox_close;
    syscall[SYSCALL_MAILBOX_SEND] = (long int (*)())&kernel_mbox_send;
    syscall[SYSCALL_MAILBOX_RECV] = (long int (*)())&kernel_mbox_recv;
    //init sleep_queue
	init_list_head(&sleep_queue);
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
int main()
{
    lock_kernel();
    // init Process Control Block (-_-!)
    if (get_current_cpu_id() == 0){
	    init_pcb();
    	current_running = &current_running_0;
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
		
		//wake up slave core
		disable_softwareint();
    	sbi_send_ipi((unsigned long *)2);
    	clear_softwareint();
    	enable_softwareint();
    	
	}else{//core 2 init
		current_running = &current_running_1;
		init_exception();
    	printk("> [INIT] Core 2 initialization succeeded.\n\r");
    	
	}
    // TO DO:
    // Setup timer interrupt and enable all interrupt
    if (get_current_cpu_id() == 0){
    	// init screen (QAQ)
    	init_screen();
    	printk("> [INIT] SCREEN initialization succeeded.\n\r");
    	//printk("> [INIT] SCREEN initialization skipped.\n\r");
    }
    sbi_set_timer(get_ticks() + TIMER_INTERVAL);
	unlock_kernel();

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
