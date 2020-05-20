#ifndef __LUKOS_USERSPACE_SCHEDULING__
#define __LUKOS_USERSPACE_SCHEDULING__

#include <stddef.h>
#include <interfaces/userspace/macros.h>
#include <managers/process-manager.h>

typedef struct
{
	char *name;
	void (*entry)(void *arg);
	void *arg;
	size_t stack_size;
	size_t priority;

}thread_info_t;


/**
 * yield to the scheduler indicating done for now
 */
KERNEL_SYSCALL0(syscall_schedule_yield);


/**
 * create a new thread of execution
 */
KERNEL_SYSCALL1(syscall_schedule_thread_create, thread_info_t *tinfo);


/**
 * get id of me (a running thread)
 */
KERNEL_SYSCALL1(syscall_schedule_thread_id_get, size_t* id);	
	
#endif
