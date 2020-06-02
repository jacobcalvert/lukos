#include <interfaces/userspace/schedule.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>
#include <interfaces/os/atomic.h>
#include <interfaces/platform/platform_data.h>

extern platform_data_t PLATFORM_DATA;


#define THREAD_LOCK(t)				atomic32_spinlock_acquire(&t->lock);
#define THREAD_UNLOCK(t)			atomic32_spinlock_release(&t->lock);


int syscall_schedule_sleep_ticks_kernel_handler(thread_t *thr, size_t ticks)
{
	
	if(ticks == 0)
	{
		return SYSCALL_RESULT_ERROR;
	}
	else
	{
		THREAD_LOCK(thr);
		thr->sleep_ticks = PLATFORM_DATA.max_cpus *(ticks+1);
		THREAD_UNLOCK(thr);
	
	}	
	
	return SYSCALL_RESULT_OK;
}

int syscall_schedule_thread_create_kernel_handler(thread_t *thr, thread_info_t *tinfo)
{
	/* we have to do this little dance because the struct could bleed over a page boundary */	
	address_space_t *as = thr->parent->as;
	void *struct_va_ptr = (void*)tinfo;
	size_t ptrsz = sizeof(void*);
	size_t sizesz = sizeof(size_t);
	size_t struct_va = *(size_t*)vmm_arch_v2p(as->arch_context, (void*)struct_va_ptr);
	char *name = (char *)vmm_arch_v2p(as->arch_context, (void*)struct_va);
	void **entry = (void*)vmm_arch_v2p(as->arch_context, (void*)(struct_va_ptr + ptrsz));
	void **arg = (void**)vmm_arch_v2p(as->arch_context, (void*)(struct_va_ptr + 2*ptrsz));
	size_t *stack_size = (size_t*)vmm_arch_v2p(as->arch_context, (void*)(struct_va_ptr + 3*ptrsz));
	size_t *pri = (size_t*)vmm_arch_v2p(as->arch_context, (void*)(struct_va_ptr + 3*ptrsz + sizesz));
	
	return pm_thread_create(name, thr->parent, *entry, *arg, *stack_size, *pri)?SYSCALL_RESULT_OK:SYSCALL_RESULT_ERROR; 

}
