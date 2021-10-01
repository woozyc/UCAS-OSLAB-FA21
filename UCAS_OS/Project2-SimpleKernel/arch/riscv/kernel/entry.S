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
  addi sp, sp, -(OFFSET_SIZE)

  /* TODO: save all general purpose registers here! */

  /*
   * Disable user-mode memory access as it should only be set in the
   * actual user copy routines.
   *
   * Disable the FPU to detect illegal usage of floating point in kernel
   * space.
   */
  li t0, SR_SUM | SR_FS

  /* TODO: save sstatus, sepc, stval, scause and sscratch on user stack */
.endm

.macro RESTORE_CONTEXT
  /* TODO: Restore all registers and sepc,sstatus */
.endm

ENTRY(enable_preempt)
  ld t1, current_running
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
ENDPROC(enable_preempt)

ENTRY(disable_preempt)
  csrw CSR_SIE, zero
  ld t1, current_running
  ld t0, PCB_PREEMPT_COUNT(t1)
  addi t0, t0, 1
  sd t0, PCB_PREEMPT_COUNT(t1)
  jr ra
ENDPROC(disable_preempt)

ENTRY(enable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(enable_interrupt)

ENTRY(disable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(disable_interrupt)

// the address of previous pcb in a0
// the address of next pcb in a1
ENTRY(switch_to)
  // save all callee save registers on kernel stack
  /* TODO: store all callee save registers,
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
  //jump to current_running's entry point
  jr ra
ENDPROC(switch_to)

ENTRY(ret_from_exception)
  /* TODO: */
ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)
  SAVE_CONTEXT

  csrw CSR_SSCRATCH, x0

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  /* TODO: load ret_from_exception into $ra
   * so that we can return to ret_from_exception
   * when interrupt_help complete.
   */

  /* TODO: call interrupt_helper
   * note: don't forget to pass parameters for it.
   */
ENDPROC(exception_handler_entry)
