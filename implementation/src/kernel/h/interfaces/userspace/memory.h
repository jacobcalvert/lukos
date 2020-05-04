#ifndef __LUKOS_USERSPACE_MEMORY__
#define __LUKOS_USERSPACE_MEMORY__


#include <stddef.h>
#include <interfaces/userspace/macros.h>
#include <managers/process-manager.h>
/**
 * allocate some memory for the user process
 * @param sz		the length requested
 * @param flags		flags indicated properties about the memory region
 * @param ptr		out- the VA of the block
 * @return 0 if OK or != on error/issue
 */
KERNEL_SYSCALL3(syscall_memory_alloc, size_t sz, size_t flags, void **ptr);

	
	
	
#endif
