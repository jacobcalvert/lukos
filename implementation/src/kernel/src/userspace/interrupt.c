#include <interfaces/userspace/interrupt.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>
#include <managers/interrupt-manager.h>
#include <interfaces/os/cpu.h>
int syscall_interrupt_attach_kernel_handler(thread_t *thread, size_t irqno, void (*handler)(size_t irqno))
{
	size_t flags = 0;
	return intm_interrupt_attach(thread->parent, cpu_current_get(), irqno, flags, handler)==0?SYSCALL_RESULT_OK:SYSCALL_RESULT_ERROR;
}

