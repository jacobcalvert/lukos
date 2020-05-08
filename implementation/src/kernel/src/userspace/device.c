#include <interfaces/userspace/device.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>
#include <interfaces/platform/platform_data.h>

extern platform_data_t PLATFORM_DATA;

int syscall_device_alloc_kernel_handler(thread_t *thr, void *base, size_t len, void **ptr)
{
	/* unusally, ptr here is pointing to the address in the destination VA */
	address_space_t *as = thr->parent->as;
	void *va = NULL;
	
	for(size_t i = 0; i < PLATFORM_DATA.num_restricted_ranges; i++)
	{
		size_t rq_base = (size_t)base;
		size_t rq_end = rq_base + len;
		
		size_t restr_base = (size_t)PLATFORM_DATA.restricted_ranges[i].phys_addr;
		size_t restr_end = restr_base + PLATFORM_DATA.restricted_ranges[i].len;
		
		/* three cases to check, fourth case (both ends inside no-go) is implicit */
		if(rq_base >= restr_base && rq_base < restr_end)
		{
			/* requested base is inside no-go region */
			return -1;
		}
		if(rq_end >= restr_base && rq_end < restr_end)
		{
			/* requested end is inside no-go region */
			return -1;
		}
		if(rq_base <= restr_base && rq_end >= restr_end)
		{
			/* requested region is superset of no-go */
			return -1;
		}	
	}
	
	/* we are OK to map */
	
	va = vmm_arch_get_free_va_range(as->arch_context, len);
	if(vmm_arch_map(as->arch_context, AS_REGION_RW, va, base, len)!= 0)
	{
		return -2;
	}
	
	/* copy from the ADDRESS of the address, to the destination VA in AS */
	vmm_address_space_copy_in(as, (void*)&va, ptr, sizeof(void*));
	
	
	return 0;
}
