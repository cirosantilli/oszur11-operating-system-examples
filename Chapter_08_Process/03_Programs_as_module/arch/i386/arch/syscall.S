/*! syscall - must be in "user space" (if different from kernel)
 *
 * "Bare function", without usual "frame": int syscall ( id, arg1, arg2, ... )
 *
 * On stack are (top to bottom): [return addres] [id] [arg1] [arg2] ...
 */

#define ASM_FILE        1
#define _USER_MODE_

#include <arch/interrupts.h>

.globl syscall

/*.section .user_code */

syscall:
	int	$SOFTWARE_INTERRUPT
	ret
