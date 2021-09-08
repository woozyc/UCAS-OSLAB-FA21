/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#define CSR_SIE			0x104
#define CSR_SIP			0x144
#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
.global _start
_start:
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop
  
  /* Clear BSS for flat non-ELF images */
  la s0, __bss_start
  la s1, __BSS_END__
  mv s2, s0
  bge s2, s1, end
  
loop:
  //wfi
  sw zero, 0(s2)
  addi s2, s2, 4
  bge s2, s1, end
  j loop
  
  /* setup C environment (set sp register)*/
end:
  li sp, KERNEL_STACK
  
  /* Jump to the code in kernel.c*/
  j main
  


