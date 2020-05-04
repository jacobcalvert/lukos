#include <exceptions/exceptions.h>
#include <interrupt-controllers/interrupt-controller.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>
#include <managers/vmm_arch.h>
#include <userspace/syscall_numbers.h>
#include <interfaces/userspace/memory.h>


#define MAX_CPUS		8
#define MAX_INTS		256

#define X2_FRAME_OFFSET		94U
#define X3_FRAME_OFFSET		95U
#define X0_FRAME_OFFSET		96U
#define X1_FRAME_OFFSET		97U



static aarch64_int_handler EXCEPTION_HANDLER_TABLE[MAX_CPUS][MAX_INTS];

void aarch64_scheduling_interrupt(size_t cpuno, size_t intno);

void aarch64_exceptions_init(void)
{
	extern char _default_el1_vtable;
	void *vbar_el1 = &_default_el1_vtable;
	 __asm ("msr vbar_el1, %[vbar_el1]" : : [vbar_el1] "r" (vbar_el1));
}


void aarch64_exceptions_handler_register(size_t cpuno, size_t intno, aarch64_int_handler handler)
{
	if(cpuno < MAX_CPUS  && intno < MAX_INTS)
	{
		EXCEPTION_HANDLER_TABLE[cpuno][intno] = handler;
	}
}

void aarch64_svc_handle(size_t cpuno, void *sp)
{
	thread_t *thread = pm_thread_current_get(cpuno);
	address_space_t *as = thread->parent->as;
	
	void *frame = vmm_arch_v2p(as->arch_context, sp);
	uint64_t syscallno = ((uint64_t*)frame)[X0_FRAME_OFFSET]; /* syscall number at X0 */
	int load_ttbr0 = 1;
	switch(syscallno)
	{	
		case SYSCALL_SCHEDULING_YIELD:
		{
			aarch64_scheduling_interrupt(cpuno, 0);
			load_ttbr0 = 0;
			break;
		};
		case SYSCALL_SCHEDULING_SLEEP:
		{
			break;
		};
		
		case SYSCALL_MEMORY_ALLOC:
		{
			size_t size = ((size_t*)frame)[X1_FRAME_OFFSET];
			size_t flags = ((size_t*)frame)[X2_FRAME_OFFSET];
			void **ptr = (void**)((size_t*)frame)[X3_FRAME_OFFSET];
			((uint64_t*)frame)[X0_FRAME_OFFSET] = syscall_memory_alloc_kernel_handler(thread, size, flags, ptr);
			break;
		};
	
	
		default: break;
	}
	if(load_ttbr0)
	{
		void* ttbr0 = (void*)((aarch64_vmm_context_t*)thread->parent->as->arch_context)->translation_table;
		__asm ("msr ttbr0_el1, %[ttbr0]\r\nisb" : : [ttbr0] "r" (ttbr0));
	}
		

}

void aarch64_exceptions_handle(size_t cpuno)
{
	size_t intno = aarch64_intc_current_int_get();//
	if(cpuno < MAX_CPUS  && intno < MAX_INTS)
	{
		if(EXCEPTION_HANDLER_TABLE[cpuno][intno])
		{
			EXCEPTION_HANDLER_TABLE[cpuno][intno](cpuno, intno);
		}
	}
	
	aarch64_intc_int_complete(intno);

}
