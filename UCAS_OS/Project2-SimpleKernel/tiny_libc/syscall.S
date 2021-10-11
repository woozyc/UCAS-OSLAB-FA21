#include <asm.h>

ENTRY(invoke_syscall)
  /* TO DO: */
  mv a7, a0
  mv a1, a0
  mv a2, a1
  mv a3, a2
  ecall
  jr ra
ENDPROC(invoke_syscall)
