# LuKOS Kernel Functions - Process Management
The process management model defines what a process is, and how it interacts with the kernel. This process manager is implemented as a Process Manager (PM).

## Concepts
### Processes
A process is simply a single thread or collection of threads sharing an Address Space. Processes are created with a particular priority which the kernel schedules in a priority-preemptive fashion. 
### Process-based Scheduling 
The kernel will check if a process is schedulable and if so turns scheduling control over to that process. The process will then select from its threads using whatever scheduling algorithm is applicable to that particular process. 
### Threads
A thread is a single "task" running in a process's address space. This thread has properties like priority (which may be unused if the round-robin scheduler is used by this process), core affinity, a name, etc. 

## Architecture Specific Concepts
### AARCH64 (ARMv8-A)
There are no special requirements or specificities on this architecture.

## Kernel Interface
These are the calls defined for use by the kernel. 

```

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
 * @param argc			the argument count
 * @param argv			the argument array
 * @param stack_size	the thread stack size
 * @param priority		the thread priority (unused in some process schedulers )
 */
thread_t* pm_thread_create(char *name, process_t *prc, void *entry, int argc, char **argv, size_t stack_size, size_t priority);


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

```

```

## Architecture Interface
These are the calls to be implemented by the architecture specific code. These provide the glue between the kernel which is architecture agnostic and the details of the implementation.

```
/**
 * architecture specific call to populate the task stack with the arguments and set the entry point
 */
void pm_arch_thread_stack_populate(process_t *prc, thread_t *thr);

```

