#include <userspace/syscall_numbers.h>

/**
 * attach a handler to an interrupt
 * @param irqno		the irq number
 * @param handler	the routine to be called
 * int syscall_interrupt_attach(size_t irqno, void (*handler)(size_t irqno);
 */ 

int syscall_interrupt_attach(size_t irqno, void (*handler)(size_t irqno))
{
	return -1;
}

	
	
/**
 * indicate to the kernel that you're done processing this inteerupt 
 * and normal scheduling should be resumed 
 */ 

int syscall_interrupt_complete(void)
{
	return 0;

}


