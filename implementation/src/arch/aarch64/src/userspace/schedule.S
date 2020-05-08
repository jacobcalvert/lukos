#include <userspace/syscall_numbers.h>



/**
 * yield 
 *	
 * int syscall_schedule_yield(void);
 */
 
.global syscall_schedule_yield
syscall_schedule_yield:
	mov x0, #SYSCALL_SCHEDULING_YIELD
	svc #0
	ret
	
/**
 * sleep for a specific number of ticks 
 *	
 * int syscall_schedule_sleep(size_t ticks);
 */
 
.global syscall_schedule_sleep
syscall_schedule_sleep:
	mov x1, x0
	mov x0, #SYSCALL_SCHEDULING_SLEEP
	svc #0
	ret

	

