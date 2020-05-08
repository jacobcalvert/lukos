#include <userspace/syscall_numbers.h>



/**
 * allocate some memory for the user process
 * @param sz		the length requested
 * @param flags		flags indicated properties about the memory region
 * @param ptr		out- the VA of the block
 * @return 0 if OK or != on error/issue
 *	
 * int syscall_memory_alloc(size_t sz, size_t flags, void **ptr);
 */
 
.global syscall_memory_alloc

syscall_memory_alloc:
	mov x0, #SYSCALL_SCHEDULING_YIELD
	svc #0
	ret

	
