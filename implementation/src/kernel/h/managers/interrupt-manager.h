/**
 * @file interrupt-manager.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date May 06, 2020
 * @brief This file defines the interface into the Interrupt Manager (INTM).
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
#ifndef __LUKOS_INTM__
#define __LUKOS_INTM__
#include <managers/process-manager.h>
#include <stddef.h>
typedef struct
{
	process_t *prc;
	void *stack;
	void *stack_pointer;
	size_t stack_size;
	void *entry;

}process_interrupt_registration_t;

/**
 * initialize the interrupt manager 
 */
void intm_init(size_t max_cpus, size_t max_irqs);

/**
 * attach to an interrupt for a particular cpu to a particular process with the flags
 * @param prc		the process to handle the interrupt
 * @param cpuno		the interrupt CPU (this may not matter for global interrupts)
 * @param irqno		the interrupt number
 * @param flags		interrupt flags (level vs edge, etc)
 * @param entry		the routine in the process's AS that will be called
 * @return 0 if OK != 0 on error/issue
 */
int intm_interrupt_attach(process_t *prc, size_t cpuno, size_t irqno, size_t flags, void *entry);

/**
 * this function will return the stack for the interrupt handler 
 * @param cpuno		the cpu number
 * @param irq no	the irq number
 * @return the interrupt registration
 */ 
process_interrupt_registration_t* intm_interrupt_handle(size_t cpuno, size_t irqno);



/*---------------------------------------------------------- */
/* architecture implemented calls below here				 */
/*---------------------------------------------------------- */

/**
 * populate the interrupt stack
 * @param reg		the registration
 */
void intm_arch_stack_populate(process_interrupt_registration_t *reg);

/**
 * instruct the interrupt to be routed to the INTM for further processing
 * @param cpuno			the cpu number
 * @param irqno			the IRQ number
 */
void intm_arch_interrupt_attach(size_t cpuno, size_t irqno);
#endif
