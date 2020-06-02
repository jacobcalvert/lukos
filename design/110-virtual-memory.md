# LuKOS Kernel Functions - Virtual Memory
The virtual memory management capability provided by the kernel is implemented as a Virtual Memory Manager (VMM). The VMM will manage the MMU translation entries for all processes and will be able to translate virtual-to-physical (V2P) addresses for the kernel and other address spaces.

## Concepts
There are several key concepts involved with managing the virtual memory of the microkernel. They are described below.

### Virtual Address
A Virtual Address (VA) is one that represents a resource in the system and may or may not be "backed" by anything.

### Physical Address
A Physical Address (PA) is an address that maps to a physical resource in the system, such as RAM or peripherals.

### Address Spaces
Address Spaces, or a singular Address Space (AS), represents a range of Virtual Addresses starting at zero (0) and going to an architecture-dependent upper-bound. Each Address Space has a unique ID associated with it, and these Adress Spaces are for use with User Processes only; i.e., the kernel does not have a numbered AS. 

### Regions
Each Address Space has one or more Regions. A Region is a mapping of a subrange of the VAs in the Address Space to a subrange of the PAs in the system of identical size. These Regions are lined up on architecture-dependent boundaries. Each Region has a set of properties defining the properties the user of the AS will have in this Region. For example, the .text area would be set to RX (read-execute), while the .data section would be RW (read-write). 

## Architecture Specific Concepts
### AARCH64 (ARMv8-A)
The Virtual Address upper bound for Address Spaces in this archtecture is 0xFFFFFF8000000000; therefore ASes in this architure have VAs in the range [0x0000000000000000, 0xFFFFFF8000000000). Note the non-inclusiveness of the upperbound. The Address Spaces in this architecture have Regions which are assigned on 4K, 2M, or 1G boundaries, depending on the requested Region size. 

## Kernel Interface
These are the calls defined for use by the kernel. 
```c

/**
 * initialize the virtual memory manager
 */
void vmm_init(void);

/**
 * create an empty address space
 * @return the new address space
 */
address_space_t *vmm_address_space_create(void);

/**
 * create a region in this address space of size (len) and properties (prop) and return the resulting VA in that address space
 * @param as	the address space
 * @param len	the len in bytes
 * @param prop	the props
 * @return the VA in the AS or NULL if failed (or won't do)
 */
void *vmm_address_space_region_create(address_space_t *as, size_t len, address_space_region_prop_t prop); 

/**
 * copy some data from kernel space into this AS
 * @param as			the address space
 * @param vakernel		the VA in kernel space
 * @param vadest		the VA in the AS space
 * @param len			the number of bytes
 */
void vmm_address_space_copy_in(address_space_t *as, void *vakernel, void *vadest, size_t len);

```

## Architecture Interface
These are the calls to be implemented by the architecture specific code. These provide the glue between the kernel which is architecture agnostic and the details of the implementation.
```c

/**
 * create the architecture specific context (transtables, etc) and return a pointer in kernel VA space to it
 */
void *vmm_arch_context_create(address_space_t *as);

/**
 * get a range of of virtual address space in the given context
 * that will accomodate the given size
 */
void *vmm_arch_get_free_range(void *context, size_t len);

/**
 * get some RAM allocated on the architecture specific boundary that is at least as big as len
 * @return the PA of the range
 */
void* vmm_arch_alloc_range(size_t len);

/**
 * map a specific VA to a specific PA of len
 * @return non-zero if problem
 */
int vmm_arch_map(void *ctx, address_space_region_prop_t props,  void *va, void *pa, size_t len);

/**
 * get the PA associated with this VA
 */
void *vmm_arch_v2p(void *ctx, void *va);
```
