#include <exceptions/exceptions.h>
#include <interrupt-controllers/interrupt-controller.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>
#include <managers/interrupt-manager.h>
#include <managers/ipc-manager.h>
#include <managers/vmm_arch.h>
#include <userspace/syscall_numbers.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/schedule.h>
#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/platform/platform_data.h>


#define MAX_CPUS		8
#define MAX_INTS		256

#define X4_FRAME_OFFSET		92U
#define X5_FRAME_OFFSET		93U
#define X2_FRAME_OFFSET		94U
#define X3_FRAME_OFFSET		95U
#define X0_FRAME_OFFSET		96U
#define X1_FRAME_OFFSET		97U


extern platform_data_t PLATFORM_DATA;
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
			((uint64_t*)frame)[X0_FRAME_OFFSET] = SYSCALL_RESULT_OK;
			load_ttbr0 = 0;
			break;
		};
		case SYSCALL_SCHEDULING_SLEEP:
		{
			((uint64_t*)frame)[X0_FRAME_OFFSET] = SYSCALL_RESULT_OK;
			break;
		};
		case SYSCALL_SCHEDULING_THREAD_CREATE:
		{
			thread_info_t *info = (thread_info_t*)vmm_arch_v2p(as->arch_context,(void*)((size_t*)frame)[X1_FRAME_OFFSET]);
			((uint64_t*)frame)[X0_FRAME_OFFSET] = pm_thread_create((char*)vmm_arch_v2p(as->arch_context,(void*)info->name), thread->parent, info->entry, info->arg, info->stack_size,info->priority)?SYSCALL_RESULT_OK:SYSCALL_RESULT_ERROR; 
			break;	
		}
		case SYSCALL_INTERRUPT_ATTACH:
		{
			size_t irqno = ((size_t*)frame)[X1_FRAME_OFFSET];
			void *entry = (void*)((size_t*)frame)[X2_FRAME_OFFSET];
			((uint64_t*)frame)[X0_FRAME_OFFSET]  = (intm_interrupt_attach(thread->parent, cpuno, irqno, 0, entry) ==  0)?SYSCALL_RESULT_OK:SYSCALL_RESULT_BAD_PARAM;
			break;
		};
		case SYSCALL_INTERRUPT_COMPLETE:
		{
			/* discard the incoming static pointer, we don't care */
			/* get new stack pointer for next to be scheduled */
			/* set it up and turn scheduling back on */
			uint64_t freq = 0;
			uint64_t comp = 0;
			uint64_t current = 0;
			void *sp0 =NULL;
			void *ttbr0 = NULL;
			__asm__ __volatile__("mrs %0, cntfrq_el0" : "=r" (freq) : : );
			comp = freq/PLATFORM_DATA.scheduling_freq;
			__asm__ __volatile__("mrs %0, cntpct_el0" : "=r" (current) : : );
			comp+=current;
			__asm__ __volatile__("msr cntp_cval_el0, %0" : "=r" (comp) : : );
			
			thread_t *thr = pm_thread_next_get(cpuno);
			sp0 = thr->stack_pointer;
			__asm__ __volatile__("msr SPSel, #0\r\nmov sp, %0\r\nmsr SPSel, #1" : "=r" (sp0) : : );
			ttbr0 = (void*)((aarch64_vmm_context_t*)thr->parent->as->arch_context)->translation_table;
			 __asm ("msr ttbr0_el1, %[ttbr0]\r\nisb" : : [ttbr0] "r" (ttbr0));
			__asm__ __volatile("mov x0, #1");
			__asm__ __volatile("msr cntp_ctl_el0, x0");
			__asm__ __volatile("msr daifclr, #7");
			load_ttbr0 = 0;
			((uint64_t*)frame)[X0_FRAME_OFFSET] = SYSCALL_RESULT_OK;
			break;
		};
		case SYSCALL_MEMORY_ALLOC:
		{
			size_t size = ((size_t*)frame)[X1_FRAME_OFFSET];
			size_t flags = ((size_t*)frame)[X2_FRAME_OFFSET];
			void **ptr = (void**)((size_t*)frame)[X3_FRAME_OFFSET];
			((uint64_t*)frame)[X0_FRAME_OFFSET] = (syscall_memory_alloc_kernel_handler(thread, size, flags, ptr) == 0)?SYSCALL_RESULT_OK:SYSCALL_RESULT_BAD_PARAM;
			break;
		};
		
		case SYSCALL_DEVICE_ALLOC:
		{
		
			void* base = (void*)((size_t*)frame)[X1_FRAME_OFFSET];
			size_t len = ((size_t*)frame)[X2_FRAME_OFFSET];
			void **ptr = (void**)((size_t*)frame)[X3_FRAME_OFFSET];
			((uint64_t*)frame)[X0_FRAME_OFFSET] = (syscall_device_alloc_kernel_handler(thread, base, len, ptr) == 0)?SYSCALL_RESULT_OK:SYSCALL_RESULT_BAD_PARAM;
			break;
		};
	
	
		case SYSCALL_IPC_PIPE_CREATE:
		{
			char *name = (char*)vmm_arch_v2p(as->arch_context,(void*)((size_t*)frame)[X1_FRAME_OFFSET]);
			size_t msg_size = ((size_t*)frame)[X2_FRAME_OFFSET];
			size_t max_msgs = ((size_t*)frame)[X3_FRAME_OFFSET];
			size_t flags = ((size_t*)frame)[X4_FRAME_OFFSET];
			
			((uint64_t*)frame)[X0_FRAME_OFFSET]  = ipcm_pipe_create(name, msg_size, max_msgs, flags)?SYSCALL_RESULT_OK:SYSCALL_RESULT_ERROR;
		
			break;
		};
		
		case SYSCALL_IPC_PIPE_ID_GET:
		{
			char *name = (char*)vmm_arch_v2p(as->arch_context,(void*)((size_t*)frame)[X1_FRAME_OFFSET]);
			size_t* id = (size_t*)vmm_arch_v2p(as->arch_context,(void*)((size_t*)frame)[X2_FRAME_OFFSET]);
			
			ipc_pipe_t *pipe = ipcm_pipe_lookup_by_name(name);
			*id = pipe->id;
			
			((uint64_t*)frame)[X0_FRAME_OFFSET]  = (pipe)?SYSCALL_RESULT_OK:SYSCALL_RESULT_NOT_FOUND;
		
			break;
		};
		
		case SYSCALL_IPC_PIPE_WRITE:
		{
			size_t id = ((size_t*)frame)[X1_FRAME_OFFSET];
			void *vamsg = (void*)((size_t*)frame)[X2_FRAME_OFFSET];
			size_t len = ((size_t*)frame)[X3_FRAME_OFFSET];
			int result = ipcm_pipe_write(thread, id, vamsg, len);
			((uint64_t*)frame)[X0_FRAME_OFFSET] = SYSCALL_RESULT_OK;
			if(result == -4)
			{
				/* causes a context switch */
				((uint64_t*)frame)[X0_FRAME_OFFSET] = SYSCALL_RESULT_PIPE_FULL;
				aarch64_scheduling_interrupt(cpuno, 0);
				load_ttbr0 = 0;
			}
			break;
		};
	
		case SYSCALL_IPC_PIPE_READ:
		{
			size_t id = ((size_t*)frame)[X1_FRAME_OFFSET];
			void *vamsg = (void*)((size_t*)frame)[X2_FRAME_OFFSET];
			void *valen = (void*)((size_t*)frame)[X3_FRAME_OFFSET];
			int result = ipcm_pipe_read(thread, id, vamsg, valen);
			((uint64_t*)frame)[X0_FRAME_OFFSET] = SYSCALL_RESULT_OK;
			if(result == -4)
			{
				/* causes a context switch */
				((uint64_t*)frame)[X0_FRAME_OFFSET] = SYSCALL_RESULT_PIPE_EMPTY;
				aarch64_scheduling_interrupt(cpuno, 0);
				load_ttbr0 = 0;
			}
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
	//aarch64_intc_ipi_send(0, 2);
	if(cpuno < MAX_CPUS  && intno < MAX_INTS)
	{
		if(EXCEPTION_HANDLER_TABLE[cpuno][intno])
		{
			EXCEPTION_HANDLER_TABLE[cpuno][intno](cpuno, intno);
		}
	}
	
	aarch64_intc_int_complete(intno);

}
