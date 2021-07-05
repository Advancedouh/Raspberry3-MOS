#include <asm-aarch64/asm.h>
#include <registers.h>
#include <time.h>

.global TRAPSP
.align 12
TRAPSP:
    .double   0

// a dummy exception handler in this tutorial
.global exc_handler
exc_handler:
    RECOVER_ALL
    eret

synch_exc_handler:
    SAVE_ALL
    mov     x0, #4
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    bl      synchronous_exception_handler
    b       exc_handler

irq_exc_handler:
    SAVE_ALL
    mov     x0, #5
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    bl      sched_yield
    b       exc_handler

    // important, code has to be properly aligned
    .global _vectors
    .align 11
_vectors:
    // Current EL with SP0
    // synchronous
    .align  7
    b      exc_handler

    // IRQ
    .align  7
    b       exc_handler

    // FIQ
    .align  7
    b       exc_handler

    // SError
    .align  7
    b       exc_handler

    // Current EL with SPx
    // synchronous
    .align  7
    msr cntp_ctl_el0, xzr
    b       synch_exc_handler

    // IRQ
    .align  7
    msr cntp_ctl_el0, xzr
    sub    sp, sp, #288
    mov    x0, sp
    bl     save_sp
    b      sched_yield

    // FIQ
    .align  7
    msr cntp_ctl_el0, xzr
    b       exc_handler

    // SError
    .align  7
    msr cntp_ctl_el0, xzr
    b       exc_handler

    // Lower EL using aarch64
    // synchronous
    .align  7
    msr cntp_ctl_el0, xzr
    b       synch_exc_handler

    // IRQ
    .align  7
    msr cntp_ctl_el0, xzr
    b       irq_exc_handler

    // FIQ
    .align  7
    msr cntp_ctl_el0, xzr
    b       exc_handler

    // SError
    .align  7
    msr cntp_ctl_el0, xzr
    b       exc_handler

    // Lower EL using aarch32
    // synchronous
    .align  7
    msr cntp_ctl_el0, xzr
    b       exc_handler

    // IRQ
    .align  7
    msr cntp_ctl_el0, xzr
    b       exc_handler

    // FIQ
    .align  7
    msr cntp_ctl_el0, xzr
    b       exc_handler

    // SError
    .align  7
    msr cntp_ctl_el0, xzr
    b       exc_handler

BEGIN_FUNC(handle_syscall)
    mov sp, x6
    blr x0
    b   exc_handler
END_FUNC(handle_syscall)