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
 
.global syscall_schedule_sleep_ticks
syscall_schedule_sleep_ticks:
	mov x1, x0
	mov x0, #SYSCALL_SCHEDULING_SLEEP
	svc #0
	ret

	
.global syscall_schedule_thread_create
syscall_schedule_thread_create:
	mov x1, x0
	mov x0, #SYSCALL_SCHEDULING_THREAD_CREATE
	svc #0
	ret
	

.global syscall_schedule_thread_id_get
syscall_schedule_thread_id_get:
	mov x1, x0
	mov x0, #SYSCALL_SCHEDULING_THREAD_ID_GET
	svc #0
	ret

