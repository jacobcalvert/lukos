/**
 * @file process-manager.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date April 30, 2020
 * @brief This file defines the interface into the Process Manager (PM).
 * 
 * The Process Manager is responsible for creating and
 * maintaining processes and their threads, scheduling, and
 * synchronizing.
 * 
 *
 * 
 * Copyright (c) 2020 Jacob Calvert
 * All rights reserved. 
 *
 * This file is subject to the terms and conditions
 * defined in 'LICENSE.txt' provided with this source
 * code package. 
 * 
 */
#ifndef __LUKOS_PM__
#define __LUKOS_PM__

#include <managers/virtual-memory-manager.h>

#include <stddef.h>

typedef struct
{

	char *name;
	size_t stack_size;
	size_t priority;
	void *stack_base;
	void *stack_pointer;
	void *entry;
	int argc;
	char **argv;
	size_t flags;

}thread_t;


typedef struct thread_list_node
{
	struct thread_list_node*next;
	thread_t *thread;

}thread_list_node_t;

typedef enum
{
	PM_SCHEDULER_ROUND_ROBIN,
	PM_SCHEDULER_PRIORITY
}process_scheduler_t;


typedef struct
{
	char *name;
	address_space_t *as;
	process_scheduler_t scheduler_type;
	size_t priority;
	thread_list_node_t* threads;

}process_t;

/**
 * initialize the Process Manager
 */
void pm_init(void);

/**
 * create a process with the given parameters but no threads and does not ready for scheduling
 * @param name		the process name
 * @param as		the address space
 * @param scheduler	the desired thread scheduler
 * @param priority 	the process priority
 */
process_t *pm_process_create(char *name, address_space_t *as, process_scheduler_t scheduler, size_t priority);


/**
 * add a process to the scheduler
 * @param prc		the process
 */
void pm_process_schedule(process_t *prc);



/**
 * create a thread and add it to the specified process
 * note: if this process is scheduled, this thread will be immediately schedulable 
 * @param name			this thred's name
 * @param prc			the parent process
 * @param entry			the entry point
 * @param argc			the argument count
 * @param argv			the argument array
 * @param stack_size	the thread stack size
 * @param priority		the thread priority (unused in some process schedulers )
 */
void pm_thread_create(char *name, process_t *prc, void *entry, int argc, char **argv, size_t stack_size, size_t priority);



/*---------------------------------------------------------- */
/* architecture implemented calls below here				 */
/*---------------------------------------------------------- */

/**
 * architecture specific call to populate the task stack with the arguments and set the entry point
 */
void pm_arch_thread_stack_populate(process_t *prc, thread_t *thr);
#endif
