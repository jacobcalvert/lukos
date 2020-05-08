#include <userspace/syscall_numbers.h>

/**
 * attach a handler to an interrupt
 * @param irqno		the irq number
 * @param handler	the routine to be called
 * int syscall_interrupt_attach(size_t irqno, void (*handler)(size_t irqno);
 */ 
.global syscall_interrupt_attach
syscall_interrupt_attach:
	mov x2, x1
	mov x1, x0
	mov x0, #SYSCALL_INTERRUPT_ATTACH
	svc #0
	ret
	
	
/**
 * indicate to the kernel that you're done processing this inteerupt 
 * and normal scheduling should be resumed 
 */ 
.global syscall_interrupt_complete
syscall_interrupt_complete:
	mov x0, #SYSCALL_INTERRUPT_COMPLETE
	svc #0
	ret /* the ret is not needed since this causes a context switch */
	

