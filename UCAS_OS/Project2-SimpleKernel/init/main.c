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
#include <test.h>
#include <os/list.h>

#include <csr.h>

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
    for(int i = 0; i < 32; i++){
    	pt_regs->regs[i] = 0;
    }
    pt_regs->regs[3] = (reg_t)__global_pointer$;
    //TODO: some special regs in above regs:
     //csw registers
     //TODO

    // set sp to simulate return from switch_to
    /* TO DO: you should prepare a stack, and push some values to
     * simulate a pcb context.
     */
    switchto_context_t *switchto_regs =
    	(switchto_context_t *)(kernel_stack - sizeof(regs_context_t) - sizeof(switchto_context_t));
    for(int i = 0; i < 14; i++){
    	switchto_regs->regs[i] = 0;
    }
    //init ra and sp, while other general purpose regs stay zero
    switchto_regs->regs[0] = entry_point;
    switchto_regs->regs[1] = (reg_t)user_stack;
    pcb->kernel_sp = (reg_t)switchto_regs;
}

static void init_pcb()
{
     /* initialize all of your pcb and add them into ready_queue     */
     init_list_head(&ready_queue);
     for(int i = 0; i < num_sched1_tasks; i++){
     	//init a pcb of a task
     	struct task_info * task = sched1_tasks[i];
     	pcb[i].pid = i+1;
     	pcb[i].kernel_sp = allocPage(1);
     	pcb[i].user_sp = allocPage(1);
     	pcb[i].preempt_count = 0;
     	pcb[i].type = task->type;
     	pcb[i].status = TASK_READY;
     	pcb[i].cursor_x = 0;
     	pcb[i].cursor_y = 0;
     	//init pcb stack
     	init_pcb_stack(pcb[i].kernel_sp, pcb[i].user_sp, task->entry_point, pcb + i);
     	//add to ready_queue
     	list_add(&(pcb[i].list), &ready_queue);
     }

    /* remember to initialize `current_running`*/
     current_running = &pid0_pcb;
}

static void init_syscall(void)
{
    // initialize system call table.
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

    // init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n\r");

    // init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n\r");

    // fdt_print(riscv_dtb);

    // init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n\r");

    // TODO:
    // Setup timer interrupt and enable all interrupt

    while (1) {
        // (QAQQQQQQQQQQQ)
        // If you do non-preemptive scheduling, you need to use it
        // to surrender control do_scheduler();
        // enable_interrupt();
        // __asm__ __volatile__("wfi\n\r":::);
        do_scheduler();
    };
    return 0;
}
