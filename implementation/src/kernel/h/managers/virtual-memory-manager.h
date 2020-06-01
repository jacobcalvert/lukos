/**
 * @file virtual-memory-manager.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date April 29, 2020
 * @brief This file defines the Virtual Memory Manager kernel API
 * 
 * The Virtual Memory Manager (VMM) is responsible for creating
 * the address spaces, managing memory, mapping/unmapping, and
 * sharing memory between processes. This file acts as a
 * bridge between the logical VMM functions the kernel needs
 * and the implementing architecture code.
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


#ifndef __LUKOS_VMM__
#define __LUKOS_VMM__

#include <stdint.h>
#include <stddef.h>


typedef enum
{
	AS_REGION_RW = 0,
	AS_REGION_RX = 1,
	AS_REGION_RO = 2,
	AS_REGION_NO_ACCESS = 3
}address_space_region_prop_t;

typedef struct
{
	void *arch_context; /**< this is the architecture specific context, the kernel is agnostic to this */
	size_t id;			/**< the uuid for this address space */
	size_t status;		

}address_space_t;

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
 * use this when we don't care where the VA is going to be
 * @param as		the address space
 * @param len		the len in bytes
 * @param prop		the props
 * @param vadest	the returned VA
 * @return 0 on OK != 0 on failure
 */
 
int vmm_address_space_region_create_auto(address_space_t *as, size_t len, address_space_region_prop_t prop, void **vadest);

/**
 * create a region in this address space of size (len) and properties (prop) at the specified VA
 * use this when we DO care where the VA is going to be
 * @param as		the address space
 * @param len		the len in bytes
 * @param vadest	where the base of the regions should be
 * @param prop		the props
 * @return 0 on OK != 0 on failure
 */
int vmm_address_space_region_create(address_space_t *as, void *vadest, size_t len, address_space_region_prop_t prop); 
 

/**
 * copy some data from kernel space into this AS
 * @param as			the address space
 * @param vakernel		the VA in kernel space
 * @param vadest		the VA in the AS space
 * @param len			the number of bytes
 */
void vmm_address_space_copy_in(address_space_t *as, void *vakernel, void *vadest, size_t len);



/*---------------------------------------------------------- */
/* architecture implemented calls below here				 */
/*---------------------------------------------------------- */

/**
 * create the architecture specific context (transtables, etc) and return a pointer in kernel VA space to it
 */
void *vmm_arch_context_create(address_space_t *as);

/**
 * get a range of of virtual address space in the given context
 * that will accomodate the given size
 */
void *vmm_arch_get_free_va_range(void *context, size_t len);
/* 
 *
 */
int vmm_arch_alloc_map(void*ctx, address_space_region_prop_t props, void *va_start, size_t len);
/**
 * map a specific VA to a specific PA of len
 * @return non-zero if problem
 */
int vmm_arch_map(void *ctx, address_space_region_prop_t props,  void *va, void *pa, size_t len);


/** 
 * check the alignment of the section based on length
 * @param va		the VA to check
 * @param len		the segment size
 * @return 0 if OK, non-zero otherwise
 */
int vmm_arch_align_check(void *va, size_t len);
/**
 * get the PA associated with this VA
 */
void *vmm_arch_v2p(void *ctx, void *va);

#endif
