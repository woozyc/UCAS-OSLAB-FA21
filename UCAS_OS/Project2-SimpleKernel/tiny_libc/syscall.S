#include <asm.h>

ENTRY(invoke_syscall)
  /* TO DO: */
  mv a7, a0
  mv a0, a1
  mv a1, a2
  mv a2, a3
  ecall
  jr ra
ENDPROC(invoke_syscall)
ENTRY(sys_fork)
  li a0, 0
  li a1, 0
  li a2, 0
  li a7, 12
  ecall
  jr ra
ENDPROC(sys_fork)
