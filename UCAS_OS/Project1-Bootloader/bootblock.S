#include <asm/sbidef.h>
#include <asm/sbiasm.h>

// #define BOOT_LOADER_SIG_OFFSET 0x1fe
// #define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
// #define OS_SIZE_LOC 0x1fc

.equ os_size_loc, 0x510001fc

// 1. kernel address (move kernel to here ~)
.equ kernel, 0x50200000

// 2. kernel main address (jmp here to start kernel main!)
.equ kernel_main, 0x50200000

// 3. bootloader copy address (spare place for kernel)
.equ copied_bootloader, 0x51000000

.text
.global main

main:
	
	// fence on all memory and I/O
	fence
	
	//copy bootloader to 0x51000000
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

	// print a \n to avoid system output error...
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR

	// 1) task1 call BIOS print string "It's bootblock!"
	//la a0, msg
	lui a5, %hi(msg)
	addi a0, a5, %lo(msg)
	SBI_CALL SBI_CONSOLE_PUTSTR

	// 2) task2 call BIOS read kernel in SD card and jump to kernel start
	lui a5, %hi(kernel)
	addi a0, a5, %lo(kernel)
	lui a5, %hi(os_size_loc)
	addi a4, a5, %lo(os_size_loc)
	lh a1, 0(a4)
	li a2, 1
	SBI_CALL SBI_SD_READ
	
	/*jump to kernel start, use jr instead of j, 
	  since code is running in another place in 
	  memory, PC should change directly*/
	lui s1, %hi(kernel_main)
	addi s0, s1, %lo(kernel_main)
	jr s0
	
	
	

// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's a bootloader...\n\r"

