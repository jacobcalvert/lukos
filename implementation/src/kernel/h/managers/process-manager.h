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

#define PM_THREAD_FLAGS_READY			(1<<0)
#define PM_THREAD_FLAGS_RUNNING			(1<<1)
#define PM_THREAD_FLAGS_RETURNED		(31<<0)


#define PM_THREAD_AFF_NONE				(size_t)-1
#define PM_THREAD_AFF_CORE(n)			(1<<n)

struct process;

typedef struct
{

	char *name;
	size_t stack_size;
	size_t priority;
	void *stack_base;
	void *stack_pointer;
	void *entry;
	void *arg;
	size_t flags;
	size_t affinity;
	struct process *parent;
	
	size_t blockers;
	
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


typedef struct process
{
	char *name;
	address_space_t *as;
	process_scheduler_t scheduler_type;
	size_t priority;
	thread_list_node_t* threads;
	uint32_t lock;

}process_t;

/**
 * initialize the Process Manager
 * @param maxcpus 		the maximum number of CPUs expected in this systems
 */
void pm_init(size_t maxcpus);

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
 * @param arg			the argument
 * @param stack_size	the thread stack size
 * @param priority		the thread priority (unused in some process schedulers )
 */
thread_t* pm_thread_create(char *name, process_t *prc, void *entry, void*arg, size_t stack_size, size_t priority);


/**
 * get the next schedulable thread for this cpu
 * @param cpuno		the cpu number
 * @return a pointer to the next thread
 */
thread_t *pm_thread_next_get(size_t cpuno);



/**
 * get current thread for this CPU
 * @param cpuno		the cpu noumber
 * @return a pointer to the current thread
 */
thread_t *pm_thread_current_get(size_t cpuno);

/**
 * set the core affinity of this thread
 */
void pm_thread_affinity_set(thread_t *thr, size_t aff);


/*---------------------------------------------------------- */
/* architecture implemented calls below here				 */
/*---------------------------------------------------------- */

/**
 * architecture specific call to populate the task stack with the arguments and set the entry point
 */
void pm_arch_thread_stack_populate(process_t *prc, thread_t *thr);
#endif
