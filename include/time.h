#define ARCH_TIMER_CTRL_ENABLE (1 << 0)
#define ARCH_TIMER_CTRL_IT_MASK (1 << 1)
#define ARCH_TIMER_CTRL_IT_STAT (1 << 2)

#define COUNTDOWN_VALUE  6250000 * 2 // 1s between time interrupt

#define IRQ_INIT \
    ldr x10, =COUNTDOWN_VALUE; \
    msr CNTP_TVAL_EL0, x10; \
    ldr x10, =ARCH_TIMER_CTRL_ENABLE; \
    msr CNTP_CTL_EL0, x10; \
    msr daifclr, #2; \

#define IRQ_OPEN \
    msr daifclr, #2; \
    ldr x29, =ARCH_TIMER_CTRL_ENABLE; \
    msr CNTP_CTL_EL0, x29;