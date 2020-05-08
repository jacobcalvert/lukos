#include <interfaces/userspace/interrupt.h>
#include <stddef.h>

void interrupt_handler(size_t irqno)
{

}

void main(int argc, char **argv)
{
	syscall_interrupt_attach(2, interrupt_handler);
	
	while(1)
	{
	
	}

}


