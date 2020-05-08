#ifndef __LUKOS_USERSPACE_DEVICE__
#define __LUKOS_USERSPACE_DEVICE__


#include <stddef.h>
#include <interfaces/userspace/macros.h>
#include <managers/process-manager.h>
/**
 * allocate a device region to this process
 * @param base		the base address (PA)
 * @param len		the len of the register set
 * @param ptr 		out - the mapped pointer in this AS
 * @return 0 if OK or != on error/issue
 */
KERNEL_SYSCALL3(syscall_device_alloc, void *base, size_t len, void **ptr);


	
	
#endif
