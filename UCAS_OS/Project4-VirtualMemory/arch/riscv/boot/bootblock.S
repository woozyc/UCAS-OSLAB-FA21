#include <asm/sbidef.h>
#include <asm/sbiasm.h>
#include <csr.h>

// #define BOOT_LOADER_SIG_OFFSET 0x1fe
// #define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
// #define OS_SIZE_LOC 0x1fc

.equ os_num_loc, 0x502001fc

// 1. kernel address (move kernel to here ~)
.equ kernel, 0x50201000

// 2. kernel main address (jmp here to start kernel main!)
.equ kernel_main, 0x50301000

.text
.global main

main:
	
	// fence on all memory and I/O
	fence
	
	
	// a0 is mhartid, a1 is the pointer of fdt
	move s0,a0
	move s1,a1
	
	.option push
	.option norelax
	la gp, __global_pointer$
	.option pop
	
	// The core whose mhartid=0 is selected to boot the kernel.
	// Other cores should wait until the kernel's initialization
	// has been finished.
 	move a0, s0
	bnez a0, secondary

	// your code
	
	// print a \n to avoid system output error...
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR
	
	
	// 1) task1 call BIOS print string "It's bootblock!"
	//la a0, msg
	lui a5, %hi(msg)
	addi a0, a5, %lo(msg)
	SBI_CALL SBI_CONSOLE_PUTSTR
	lui a5, %hi(msg2)
	addi a0, a5, %lo(msg2)
	SBI_CALL SBI_CONSOLE_PUTSTR
	
	
	lui a5, %hi(os_num_loc)
	addi a4, a5, %lo(os_num_loc)//a4: num_loc
	lh a0, 0(a4)
	mv a3, a0//a3: os_num
	addi a0, a0, 48
	SBI_CALL SBI_CONSOLE_PUTCHAR
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR
	
	//wait for kernel choice
	li a0, -1
waitinput:
	SBI_CALL SBI_CONSOLE_GETCHAR
	addi a0, a0, -48
	ble a0, zero, waitinput
	blt a3, a0, waitinput
	mv s1, a0
	
	//print input
	addi a0, a0, 48
	SBI_CALL SBI_CONSOLE_PUTCHAR
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR
	mv a0, s1
	//decide which kernel to copy
	li a2, 1
addsize://get ready for sd_read
	addi a4, a4, -4
	lh a1, 0(a4)
	addi a0, a0, -1
	beqz a0, runkernel
	add a2, a2, a1
	j addsize

runkernel:	
	// 2) task2 call BIOS read kernel in SD card and jump to kernel startx
	lui a5, %hi(kernel)
	addi a0, a5, %lo(kernel)
	
	//copy at most 64 sections everytime
	mv s0, a1
	li s1, 64
	li s2, 32768
copy:
	blt a1, s1, directcopy
	li a1, 64
directcopy:
	SBI_CALL SBI_SD_READ
	addi a2, a2, 64
	addi s0, s0, -64
	add a0, a0, s2
	bgtz s0, copy 
	
	
	/*jump to kernel start, use jr instead of j, 
	  since code is running in another place in 
	  memory, PC should change directly*/
	lui s1, %hi(kernel_main)
	addi s0, s1, %lo(kernel_main)
	jr s0
	
secondary:
	/* TO DO: */
	//Mask all interrupts
    csrw CSR_SIE, zero
    csrw CSR_SIP, zero
    
	//let stvec pointer to kernel_main
	la t0, kernel
	csrw CSR_STVEC, t0
	
	//enable software interrupt for ipi
	li t0, SIE_SSIE
    csrs CSR_SIE, t0
    
    li t0, SR_SIE
    csrw CSR_SSTATUS, t0
    
	lui a5, %hi(msg1)
	addi a0, a5, %lo(msg1)
	SBI_CALL SBI_CONSOLE_PUTSTR
    
wait_for_wakeup:
	wfi
	j wait_for_wakeup
	
	

// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's a bootloader...\n\r"
msg1: .string "Core 2 starting...\n\r"
msg2: .string "Please input the id of kernel you want to boot, total kernel number: "

