#include <asm-aarch64/asm.h>
#include <registers.h>
#include <time.h>


BEGIN_FUNC(check_time)
    mrs x0, CNTP_CTL_EL0
    ret
END_FUNC(check_time)

#define cntkctl_help (512 + 256 + 3)

BEGIN_FUNC(time_init)
    ldr x0, =cntkctl_help
    msr CNTKCTL_EL1, x0
    IRQ_INIT
    mrs x0, CNTP_CTL_EL0
    ret
END_FUNC(time_init)

BEGIN_FUNC(enable_interrupts)
    msr daifclr, #2
    ret
END_FUNC(enable_interrupts)

BEGIN_FUNC(disable_interrupts)
    msr daifset, #2
END_FUNC(disable_interrupts)


BEGIN_FUNC(get_time_frequency)
    mrs x0, CNTFRQ_EL0
    ret
END_FUNC(get_time_frequency)
