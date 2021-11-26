#include <asm.h>
#include <csr.h>

.macro SAVE_CONTEXT
  .local _restore_kernel_tpsp
  .local _save_context
  /*
   * If coming from userspace, preserve the user thread pointer and load
   * the kernel thread pointer.  If we came from the kernel, sscratch
   * will contain 0, and we should continue on the current TP.
   */
  csrrw tp, CSR_SSCRATCH, tp
  bnez tp, _save_context

_restore_kernel_tpsp:
  csrr tp, CSR_SSCRATCH
  sd sp, PCB_KERNEL_SP(tp)
_save_context:
  sd sp, PCB_USER_SP(tp)
  ld sp, PCB_KERNEL_SP(tp)
  addi sp, sp, SWITCH_TO_SIZE

  /* TO DO: save all general purpose registers here! */
  sd ra, OFFSET_REG_RA(sp)
  sd gp, OFFSET_REG_GP(sp)
  sd t0, OFFSET_REG_T0(sp)
  sd t1, OFFSET_REG_T1(sp)
  sd t2, OFFSET_REG_T2(sp)
  sd s0, OFFSET_REG_S0(sp)
  sd s1, OFFSET_REG_S1(sp)
  sd a0, OFFSET_REG_A0(sp)
  sd a1, OFFSET_REG_A1(sp)
  sd a2, OFFSET_REG_A2(sp)
  sd a3, OFFSET_REG_A3(sp)
  sd a4, OFFSET_REG_A4(sp)
  sd a5, OFFSET_REG_A5(sp)
  sd a6, OFFSET_REG_A6(sp)
  sd a7, OFFSET_REG_A7(sp)
  sd s2, OFFSET_REG_S2(sp)
  sd s3, OFFSET_REG_S3(sp)
  sd s4, OFFSET_REG_S4(sp)
  sd s5, OFFSET_REG_S5(sp)
  sd s6, OFFSET_REG_S6(sp)
  sd s7, OFFSET_REG_S7(sp)
  sd s8, OFFSET_REG_S8(sp)
  sd s9, OFFSET_REG_S9(sp)
  sd s10, OFFSET_REG_S10(sp)
  sd s11, OFFSET_REG_S11(sp)
  sd t3, OFFSET_REG_T3(sp)
  sd t4, OFFSET_REG_T4(sp)
  sd t5, OFFSET_REG_T5(sp)
  sd t6, OFFSET_REG_T6(sp)

  ld s0, PCB_USER_SP(tp)
  sd s0, OFFSET_REG_SP(sp)
  
  csrr s1, CSR_SSCRATCH
  sd s1, OFFSET_REG_TP(sp)
  /*
   * Disable user-mode memory access as it should only be set in the
   * actual user copy routines.
   *
   * Disable the FPU to detect illegal usage of floating point in kernel
   * space.
   */
  li t0, SR_SUM | SR_FS
  csrrc s2, CSR_SSTATUS, t0

  /* TO DO: save sstatus, sepc, stval, scause and sscratch on user stack */
  sd s2, OFFSET_REG_SSTATUS(sp)
  
  csrr s3, CSR_SEPC
  sd s3, OFFSET_REG_SEPC(sp)
  
  csrr s4, CSR_STVAL
  sd s4, OFFSET_REG_SBADADDR(sp)
  
  csrr s5, CSR_SCAUSE
  sd s5, OFFSET_REG_SCAUSE(sp)
  
  addi sp, sp, -SWITCH_TO_SIZE
  
.endm

.macro RESTORE_CONTEXT
  /* TO DO: Restore all registers and sepc,sstatus */
  ld   sp, PCB_KERNEL_SP(tp)
  addi sp, sp, SWITCH_TO_SIZE
  
  ld s0, OFFSET_REG_SEPC(sp)
  csrw CSR_SEPC, s0
  
  ld s1, OFFSET_REG_SSTATUS(sp)
  csrw CSR_SSTATUS, s1
  
  ld ra, OFFSET_REG_RA(sp)
  ld gp, OFFSET_REG_GP(sp)
  ld tp, OFFSET_REG_TP(sp)
  ld t0, OFFSET_REG_T0(sp)
  ld t1, OFFSET_REG_T1(sp)
  ld t2, OFFSET_REG_T2(sp)
  ld s0, OFFSET_REG_S0(sp)
  ld s1, OFFSET_REG_S1(sp)
  ld a0, OFFSET_REG_A0(sp)
  ld a1, OFFSET_REG_A1(sp)
  ld a2, OFFSET_REG_A2(sp)
  ld a3, OFFSET_REG_A3(sp)
  ld a4, OFFSET_REG_A4(sp)
  ld a5, OFFSET_REG_A5(sp)
  ld a6, OFFSET_REG_A6(sp)
  ld a7, OFFSET_REG_A7(sp)
  ld s2, OFFSET_REG_S2(sp)
  ld s3, OFFSET_REG_S3(sp)
  ld s4, OFFSET_REG_S4(sp)
  ld s5, OFFSET_REG_S5(sp)
  ld s6, OFFSET_REG_S6(sp)
  ld s7, OFFSET_REG_S7(sp)
  ld s8, OFFSET_REG_S8(sp)
  ld s9, OFFSET_REG_S9(sp)
  ld s10, OFFSET_REG_S10(sp)
  ld s11, OFFSET_REG_S11(sp)
  ld t3, OFFSET_REG_T3(sp)
  ld t4, OFFSET_REG_T4(sp)
  ld t5, OFFSET_REG_T5(sp)
  ld t6, OFFSET_REG_T6(sp)
  ld sp, OFFSET_REG_SP(sp)
.endm

ENTRY(enable_preempt)
  csrr t0, CSR_MHARTID
  bnez t0, enable_1
  ld t1, current_running_0
enable_chk:
  ld t0, PCB_PREEMPT_COUNT(t1)
  beq t0, zero, do_enable
  addi t0, t0, -1
  sd t0, PCB_PREEMPT_COUNT(t1)
  beq t0, zero, do_enable
  jr ra
do_enable:
  not t0, x0
  csrs CSR_SIE, t0
  jr ra
enable_1:
  ld t1, current_running_1
  j enable_chk
ENDPROC(enable_preempt)

ENTRY(disable_preempt)
  csrw CSR_SIE, zero
  csrr t0, CSR_MHARTID
  bnez t0, disable_1
  ld t1, current_running_0
do_disable:
  ld t0, PCB_PREEMPT_COUNT(t1)
  addi t0, t0, 1
  sd t0, PCB_PREEMPT_COUNT(t1)
  jr ra
disable_1:
  ld t1, current_running_1
  j do_disable
ENDPROC(disable_preempt)

ENTRY(enable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(enable_interrupt)

ENTRY(disable_interrupt)
  li t0, SR_SIE
  csrc CSR_SSTATUS, t0
  jr ra
ENDPROC(disable_interrupt)

ENTRY(enable_softwareint)
  li t0, SIE_SSIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(enable_softwareint)

ENTRY(disable_softwareint)
  li t0, SIE_SSIE
  csrc CSR_SIE, t0
  jr ra
ENDPROC(disable_softwareint)

ENTRY(clear_softwareint)
  li t0, SIE_SSIE
  csrc CSR_SIP, t0
  jr ra
ENDPROC(clear_softwareint)

ENTRY(clear_interrupt)
  mv t0, zero
  csrs CSR_SIP, t0
  jr ra
ENDPROC(clear_interrupt)

// the address of previous pcb in a0
// the address of next pcb in a1
ENTRY(switch_to)
  //if in no_store mode, only ld, no sd
  bnez a2, no_store
  // save all callee save registers on kernel stack
  /* TO DO: store all callee save registers,
   * see the definition of `struct switchto_context` in sched.h*/
  //tp: last_run pcb addr, we can also use a0 reg
  mv tp, a0
  ld tp, PCB_KERNEL_SP(tp)
  //tp now is the kernel_sp of last_run pcb
  //continue to add on tp to find switch_to context
  sd ra, SWITCH_TO_RA(tp)
  sd sp, SWITCH_TO_SP(tp)
  sd s0, SWITCH_TO_S0(tp)
  sd s1, SWITCH_TO_S1(tp)
  sd s2, SWITCH_TO_S2(tp)
  sd s3, SWITCH_TO_S3(tp)
  sd s4, SWITCH_TO_S4(tp)
  sd s5, SWITCH_TO_S5(tp)
  sd s6, SWITCH_TO_S6(tp)
  sd s7, SWITCH_TO_S7(tp)
  sd s8, SWITCH_TO_S8(tp)
  sd s9, SWITCH_TO_S9(tp)
  sd s10, SWITCH_TO_S10(tp)
  sd s11, SWITCH_TO_S11(tp)
  // restore next
  /* TO DO: restore all callee save registers,
   * see the definition of `struct switchto_context` in sched.h*/
no_store:
  mv tp, a1
  //tp now is the addr of current_running pcb
  ld tp, PCB_KERNEL_SP(tp)
  //tp now is the kernel_sp of last_run pcb
  //continue to add on tp to find switch_to context
  ld ra, SWITCH_TO_RA(tp)
  ld sp, SWITCH_TO_SP(tp)
  ld s0, SWITCH_TO_S0(tp)
  ld s1, SWITCH_TO_S1(tp)
  ld s2, SWITCH_TO_S2(tp)
  ld s3, SWITCH_TO_S3(tp)
  ld s4, SWITCH_TO_S4(tp)
  ld s5, SWITCH_TO_S5(tp)
  ld s6, SWITCH_TO_S6(tp)
  ld s7, SWITCH_TO_S7(tp)
  ld s8, SWITCH_TO_S8(tp)
  ld s9, SWITCH_TO_S9(tp)
  ld s10, SWITCH_TO_S10(tp)
  ld s11, SWITCH_TO_S11(tp)
  //according to the routine,
  //tp should stay the same as current_running
  mv tp, a1
  csrw CSR_SSCRATCH, tp
  li t0, SR_SUM
  csrs CSR_SSTATUS, t0
  //jump to current_running's entry point
  jr ra
ENDPROC(switch_to)

ENTRY(ret_from_exception)
  /* TO DO: */
  call unlock_kernel
  
  RESTORE_CONTEXT
  //restore user process scratch
  csrw CSR_SSCRATCH, tp
  
  li t0, SR_SUM
  csrs CSR_SSTATUS, t0
  sret
ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)
  SAVE_CONTEXT
  
  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop
  
  call lock_kernel

  csrw CSR_SSCRATCH, x0

  li t0, SR_SUM
  csrs CSR_SSTATUS, t0

  /* TO DO: load ret_from_exception into $ra
   * so that we can return to ret_from_exception
   * when interrupt_help complete.
   */
  la ra, ret_from_exception

  /* TO DO: call interrupt_helper
   * note: don't forget to pass parameters for it.
   */
  ld a0, PCB_KERNEL_SP(tp)
  addi a0, a0, SWITCH_TO_SIZE
  csrr a1, CSR_STVAL
  csrr a2, CSR_SCAUSE
  //do not omit x0
  call x0, interrupt_helper
  
ENDPROC(exception_handler_entry)
