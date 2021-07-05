#include <asm-aarch64/asm.h>
#include <registers.h>

BEGIN_FUNC(flush_tlb)
    tlbi vmalle1is
    dsb sy
    isb
    ret
END_FUNC(flush_tlb)