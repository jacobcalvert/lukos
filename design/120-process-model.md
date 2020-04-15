# LuKOS Kernel Functions - Process Management
The process management model defines what a process is, and how it interacts with the kernel. This process manager is implemented as a Process Manager (PM).

## Process Model
A process is simply a single thread or collection of threads sharing an address space. The individual threads in a process will be scheduled according to the currently implemented scheduler policy regardless of process relation. 

### Process Creation/Deletion
Processes can be spawned from the root process on system startup, and from other running processes. Additional threads of execution in the same process can be spawned from the initial thread in the process. A process can only delete itself in userland, but the kernel can kill a process (if it is behaving badly). If all threads in a process are deleted, the kernel will remove the process identifier as well. 

## Process Management
### Kernel Calls
```
/**
 * create a new process
 */	
process_t pm_process_create(thread_func_t entry, size_t stack_size);

/**
 * delete this process, removes all threads associated with this process
 */
 void pm_process_delete(void);
```

## Thread Management
### Kernel Calls
```
/**
 * create a new thread in the current process
 */
 thread_t pm_thread_create(thread_func_t entry, size_t stack_size, void *arg);
 
/**
 * deletes a given thread in this process
 */
void pm_thread_delete(thread_t thread, int *rc);
```
