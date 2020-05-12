# LuKOS Userspace Model
The LuKOS userspace runs entirely in the loweset privilege level on the architecture (EL0 on ARMv8 for example), and uses a single Address Space (see [the Virtual Memory Management subsystem](110-virtual-memory.md.html) for more info on this. It uses context switches via syscalls into Kernel mode to make requests for things like resources (RAM, processing time), access (shared memory) etc. 

## Userspace Functions
The userspace functions are broken down into groups based on their functional area.

### Data Path
When a userspace application wants the kernel to something on its behalf, the userspace application will make a system call. This system call will record the arguments to the call and switch contexts to kernel mode. The architecture-specific handler will unpack these arguments and pass them to a kernel generic handler. This process in unravelled to return data to the calling application. 

### Function Groups
#### Scheduling Calls
```
	/**
	 * yield control back to the scheduler, indicating I'm done for now
	 */
	void syscall_schedule_yield(void);
	
	/**
	 * create a new thread based on tinfo
	 * @param tinfo		thread info for creation
	 */
	int syscall_schedule_thread_create(thread_info_t *tinfo);
	


```

#### Memory Calls

```
	/**
	 * allocate some memory for the user process
	 * @param sz		the length requested
	 * @param flags		flags indicated properties about the memory region
	 * @param ptr		out- the VA of the block
	 * @return 0 if OK or != on error/issue
	 */
	int syscall_memory_alloc(size_t sz, size_t flags, void **ptr);
```

#### Device Calls
```

	/**
	 * allocate a memory-mapped device 
	 * @param pa		the physical address of this device
	 * @param sz		number of bytes to map
	 * @param ptr		out - the VA of the mapped device
	 * @return 0 if OK or !=0 on error/issue
	 */
	 int syscall_device_alloc(void *pa, size_t sz, void **ptr);

```

#### IPC Calls
```
	/**
	 *
	 */

```

