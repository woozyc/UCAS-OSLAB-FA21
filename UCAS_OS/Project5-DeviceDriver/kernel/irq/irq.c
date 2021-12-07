#include <os/irq.h>
#include <os/time.h>
#include <os/sched.h>
#include <os/string.h>
#include <os/stdio.h>
#include <csr.h>
#include <assert.h>
#include <sbi.h>
#include <screen.h>
#include <os/smp.h>
#include <emacps/xemacps_example.h>
#include <plic.h>

handler_t irq_table[IRQC_COUNT];
handler_t exc_table[EXCC_COUNT];
uintptr_t riscv_dtb;

void reset_irq_timer()
{
    // TO DO clock interrupt handler.
    // TO DO: call following functions when task4
    screen_reflush();
    timer_check();

    // note: use sbi_set_timer
    sbi_set_timer(get_ticks() + TIMER_INTERVAL);
    // remember to reschedule
    do_scheduler();
}

void interrupt_helper(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    // TO DO interrupt handler.
    // call corresponding handler by the value of `cause`
    uint64_t irq_type = cause & SCAUSE_IRQ_FLAG;
    uint64_t irq_code = cause & ~SCAUSE_IRQ_FLAG;
    if(irq_type){
    	irq_table[irq_code](regs, stval, cause);
    }else{
    	exc_table[irq_code](regs, stval, cause);
    }
}

void handle_int(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    reset_irq_timer();
}

void handle_pagefault(regs_context_t *regs, uint64_t stval, uint64_t cause){
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	int swap_node;
	int pid = (*current_running)->parent_id ? (*current_running)->parent_id : (*current_running)->pid;
	ptr_t kva;
	PTE *ppte;
	//check if there is any on the disk
	for(swap_node = swapmem_head; swapmem_pool[swap_node % 4096].valid; swap_node--){
		if((swap_node - 1) % 4096 == swapmem_head)
			break;
		if(pid == swapmem_pool[swap_node % 4096].pid && ((stval >> 12) << 12) == swapmem_pool[swap_node % 4096].vaddr){
			kva = allocPage(1);
			sbi_sd_read(kva2pa(kva), 8, swap_node * 8);
			ppte = swapmem_pool[swap_node % 4096].ppte;
    		set_pfn(ppte, kva2pa(kva) >> NORMAL_PAGE_SHIFT);
    		set_attribute(
       		 ppte, _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE |
                        _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER);
			swapmem_pool[swap_node % 4096].valid = 0;
			return ;
		}
	}
	alloc_page_helper(stval, (*current_running)->pgdir, 1);
}
void handle_irq(regs_context_t *regs, int irq)
{
    // TODO: 
    // handle external irq from network device
    // let PLIC know that handle_irq has been finished
}

void init_exception()
{
    /* TO DO: initialize irq_table and exc_table */
    /* note: handle_int, handle_syscall, handle_other, etc.*/
	int i;
	for(i = 0; i < IRQC_COUNT; i++){
		irq_table[i] = &handle_int;
	}
	for(i = 0; i < EXCC_COUNT; i++){
		exc_table[i] = &handle_other;
	}
	exc_table[EXCC_SYSCALL] = &handle_syscall;
	exc_table[EXCC_INST_PAGE_FAULT] = &handle_pagefault;
	exc_table[EXCC_LOAD_PAGE_FAULT] = &handle_pagefault;
	exc_table[EXCC_STORE_PAGE_FAULT] = &handle_pagefault;

    setup_exception();
}

void handle_other(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    char* reg_name[] = {
        "zero "," ra  "," sp  "," gp  "," tp  ",
        " t0  "," t1  "," t2  ","s0/fp"," s1  ",
        " a0  "," a1  "," a2  "," a3  "," a4  ",
        " a5  "," a6  "," a7  "," s2  "," s3  ",
        " s4  "," s5  "," s6  "," s7  "," s8  ",
        " s9  "," s10 "," s11 "," t3  "," t4  ",
        " t5  "," t6  "
    };
    for (int i = 0; i < 32; i += 3) {
        for (int j = 0; j < 3 && i + j < 32; ++j) {
            printk("%s : %016lx ",reg_name[i+j], regs->regs[i+j]);
        }
        printk("\n\r");
    }
    printk("sstatus: 0x%lx sbadaddr: 0x%lx scause: %lx\n\r",
           regs->sstatus, regs->sbadaddr, regs->scause);
    printk("stval: 0x%lx cause: %lx\n\r",
           stval, cause);
    printk("sepc: 0x%lx\n\r", regs->sepc);
    printk("mhartid: 0x%lx\n\r", get_current_cpu_id());

    uintptr_t fp = regs->regs[8], sp = regs->regs[2];
    printk("[Backtrace]\n\r");
    printk("  addr: %lx sp: %lx fp: %lx\n\r", regs->regs[1] - 4, sp, fp);
    // while (fp < USER_STACK_ADDR && fp > USER_STACK_ADDR - PAGE_SIZE) {
    while (fp > 0x10000) {
        uintptr_t prev_ra = *(uintptr_t*)(fp-8);
        uintptr_t prev_fp = *(uintptr_t*)(fp-16);

        printk("  addr: %lx sp: %lx fp: %lx\n\r", prev_ra - 4, fp, prev_fp);

        fp = prev_fp;
    }

    assert(0);
}
