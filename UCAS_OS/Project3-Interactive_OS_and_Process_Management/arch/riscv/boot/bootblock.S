#include <asm/sbidef.h>
#include <asm/sbiasm.h>

// #define BOOT_LOADER_SIG_OFFSET 0x1fe
// #define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
// #define OS_SIZE_LOC 0x1fc

.equ os_num_loc, 0x5e0001fc

// 1. kernel address (move kernel to here ~)
.equ kernel, 0x50200000

// 2. kernel main address (jmp here to start kernel main!)
.equ kernel_main, 0x50200000

// 3. bootloader copy address (spare place for kernel)
.equ copied_bootloader, 0x5e000000

.text
.global main

main:
	
	// fence on all memory and I/O
	fence
	
	//copy bootloader to 0x5e000000
	lui a5, %hi(copied_bootloader)
	addi a0, a5, %lo(copied_bootloader)
	li a1, 1
	li a2, 0
	SBI_CALL SBI_SD_READ
	//jump to copied_bootloader to continue
	lui s1, %hi(copied_bootloader)
	addi s0, s1, %lo(copied_bootloader)
	addi s0, s0, 38
	jr s0
	fence.i
	.option push
	.option norelax
	la gp, __global_pointer$
	.option pop
	
	// a0 is mhartid, a1 is the pointer of fdt
	move s0,a0
	move s1,a1

	// print a \n to avoid system output error...
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR
	
	// The core whose mhartid=0 is selected to boot the kernel.
	// Other cores should wait until the kernel's initialization
	// has been finished.
 	move a0, s0
	bnez a0, secondary

	// your code
	
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
	SBI_CALL SBI_SD_READ
	
	
	/*jump to kernel start, use jr instead of j, 
	  since code is running in another place in 
	  memory, PC should change directly*/
	lui s1, %hi(kernel_main)
	addi s0, s1, %lo(kernel_main)
	jr s0
	
secondary:
	/* TODO: 
	 * 1. Mask all interrupts
	 * 2. let stvec pointer to kernel_main
	 * 3. enable software interrupt for ipi
	 */

wait_for_wakeup:
	wfi
	j wait_for_wakeup
	
	

// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's a bootloader...\n\r"
msg2: .string "Please input the id of kernel you want to boot, total kernel number: "

