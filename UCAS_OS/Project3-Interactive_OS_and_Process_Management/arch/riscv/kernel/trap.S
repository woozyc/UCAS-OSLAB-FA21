#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)
  /* TO DO: 
   * save exception_handler_entry into STVEC
   * enable global exceptions */
   la s0, exception_handler_entry
   csrw CSR_STVEC, s0
   
   li s1, SR_SIE
   csrw CSR_SSTATUS, s1
   
   li s2, SIE_SSIE | SIE_STIE | SIE_SEIE
   csrw CSR_SIE, s2
   
   jr ra

ENDPROC(setup_exception)
