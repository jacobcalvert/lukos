
/**
 * @file platform_data.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date April 29, 2020
 * @brief This file defines the structure for defining platform data.
 * 
 * This platform data is used by the kernel and architecture
 * specific code to initialize the system properly. It also
 * provides some human-readable data for identification of the
 * running platform.
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

#ifndef __LUKOS_PLATFORM_DATA__
#define __LUKOS_PLATFORM_DATA__

#include <stdint.h>
#include <stddef.h>

#define INTERRUPT_RESTRICT_CPU(N)		(1<<N)
#define INTERRUPT_RESTRICT_CPU_ALL		(size_t)-1

typedef struct
{
	char *name;
	void *phys_addr;
	size_t len;

}platform_restrict_range_t;

typedef struct
{
	char *name;
	size_t irqno;
	size_t cpus;

}platform_restrict_interrupt_t;

typedef struct platform_data
{
	/* these first few properties are meaningless to the system, they are only for human consumption */
	char *name;					/**< the platform name as a string, e.g., "raspberry-pi-3B" */
	char *version;				/**< the platform version as a string, e.g., "v0.0.1" */

	uint32_t max_cpus;			/**< number of cpus to use, if max_cpus < number in system the extras will be idled */
	uint32_t scheduling_freq; 	/**< scheduling frequency in Hz */
	
	platform_restrict_range_t *restricted_ranges;	/**< MMIO restricted ranges */
	size_t num_restricted_ranges;
	
	platform_restrict_interrupt_t *restricted_interrupts;	/**< interrupt numbers the processes aren't allowed to attach */
	size_t num_restricted_interrupts;
	
	

}platform_data_t;

#endif
