#ifndef __LUKOS_USERSPACE_SCHEDULING__
#define __LUKOS_USERSPACE_SCHEDULING__

#include <stddef.h>
#include <interfaces/userspace/macros.h>
#include <managers/process-manager.h>

/**
 * yield to the scheduler indicating done for now
 */
KERNEL_SYSCALL0(syscall_schedule_yield);
	
	
#endif
