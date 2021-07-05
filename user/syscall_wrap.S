#include <asm-aarch64/asm.h>

BEGIN_FUNC(msyscall)
    SVC #0
    ret
END_FUNC(msyscall)
