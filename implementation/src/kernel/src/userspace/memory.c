#include <interfaces/userspace/memory.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>

int syscall_memory_alloc_kernel_handler(thread_t *thr, size_t sz, size_t flags, void **ptr)
{
	/* unusally, ptr here is pointing to the address in the destination VA */
	address_space_t *as = thr->parent->as;
	void *va = NULL;
	if(vmm_address_space_region_create_auto(as, sz, AS_REGION_RW, &va) != 0)
	{
		return SYSCALL_RESULT_ERROR;
	}	
	
	/* copy from the ADDRESS of the address, to the destination VA in AS */
	vmm_address_space_copy_in(as, (void*)&va, ptr, sizeof(void*));

		
	return SYSCALL_RESULT_OK;
}
