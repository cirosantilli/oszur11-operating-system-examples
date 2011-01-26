/*! Syscall - call to kernel function through software interrupt */

#include <lib/types.h>
#include <kernel/syscall.h> /* for syscall IDs */

extern int syscall ( uint id, ... ) __attribute__(( noinline ));
