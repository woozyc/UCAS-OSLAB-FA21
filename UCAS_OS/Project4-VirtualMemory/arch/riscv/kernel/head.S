/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

/* maybe you need another stack for booting? */
.equ boot_stack_top_base, 0x50202000

.section ".entry_function","ax"
ENTRY(_start)
  /* TO DO: */
  /* setup C environment for boot.c */
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /*
   * Disable FPU to detect illegal usage of
   * floating point in kernel space
   */
  li t0, SR_FS
  csrc sstatus, t0

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  //second core, do not clear bss twice
  csrr t0, CSR_MHARTID
  bnez t0, secondary
  
  /* Clear BSS for flat non-ELF images */
  la a3, __bss_start
  la a4, __BSS_END__
  ble a4, a3, clear_bss_done
clear_bss:
  sd zero, (a3)
  add a3, a3, RISCV_SZPTR
  blt a3, a4, clear_bss
clear_bss_done:

  /* setup C environment */
  li sp, boot_stack_top_base
  
  //core 1 finish
  j all_done

secondary:

  li sp, boot_stack_top_base
  
all_done:
  
  csrw CSR_SSCRATCH, x0

  //la a0, riscv_dtb
  //sd a1, (a0)
  csrr a0, CSR_MHARTID 

  /* this should never return */
  call boot_kernel

loop:
  wfi
  j loop

END(_start)
