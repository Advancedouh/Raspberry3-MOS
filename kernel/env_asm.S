#include <registers.h>
#include <asm-aarch64/asm.h>

.global exc_handler

BEGIN_FUNC(env_pop_tf)
    mov sp, x0
    b exc_handler
END_FUNC(env_pop_tf)

BEGIN_FUNC(lcontext)
    msr     ttbr0_el1, x0
    isb
    ret
END_FUNC(lcontext)

BEGIN_FUNC(get_user_pgdir)
    msr     ttbr0_el1, x0
    ret
END_FUNC(get_user_pgdir)

BEGIN_FUNC(get_kernel_pgdir)
    msr     ttbr1_el1, x0
    ret
END_FUNC(get_kernel_pgdir)