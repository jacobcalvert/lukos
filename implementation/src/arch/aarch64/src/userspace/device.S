#include <userspace/syscall_numbers.h>
/**
 * allocate a device region to this process
 * @param base		the base address (PA)
 * @param len		the len of the register set
 * @param ptr 		out - the mapped pointer in this AS
 * @return 0 if OK or != on error/issue
 */
.global syscall_device_alloc
syscall_device_alloc:
	mov x3, x2
	mov x2, x1
	mov x1, x0
	mov x0, #SYSCALL_DEVICE_ALLOC
	svc #0
	ret
