#ifndef __LUKOS_USERSPACE_INTERRUPTS__
#define __LUKOS_USERSPACE_INTERRUPTS__

#include <stddef.h>
#include <interfaces/userspace/macros.h>


/**
 * attach a handler to an interrupt
 * @param irqno		the irq number
 * @param handler	the routine to be called
 */ 
KERNEL_SYSCALL2(syscall_interrupt_attach, size_t irqno, void (*handler)(size_t irqno));

/**
 * indicate to the kernel that you're done processing this inteerupt 
 * and normal scheduling should be resumed 
 */ 
KERNEL_SYSCALL0(syscall_interrupt_complete);
	
	
	
#endif
