#include <managers/virtual-memory-manager.h>
#include <libraries/mem/memlib.h>
#include <string.h>

static size_t VMM_ASID_COUNTER = 1;
void vmm_init(void)
{

}

address_space_t *vmm_address_space_create(void)
{
	address_space_t *as = (address_space_t*)memlib_malloc(sizeof(address_space_t));
	
	/* give us an address space ID! */
	as->id = VMM_ASID_COUNTER++;
	
	/* create this context, and give the creator access to metadata (like ASID) */
	as->arch_context = vmm_arch_context_create(as);
	
	return as;
	
}

int vmm_address_space_region_create_auto(address_space_t *as, size_t len, address_space_region_prop_t prop, void **vadest)
{
	void *va = vmm_arch_get_free_va_range(as->arch_context, len);
	
	
	if(vmm_arch_alloc_map(as->arch_context, prop, va, len) != 0)
	{
		return -1;
	}
	*vadest = va;
	return 0;

}

int vmm_address_space_region_create(address_space_t *as, void *vadest, size_t len, address_space_region_prop_t prop)
{
	void *va = vadest;
	if(vmm_arch_align_check(vadest, len) != 0 || vmm_arch_alloc_map(as->arch_context, prop, va, len) != 0)
	{
		return -1;
	}
	return 0;
}

void vmm_address_space_copy_in(address_space_t *as, void *vakernel, void *vadest, size_t len)
{
	size_t src = (size_t)vakernel;
	size_t dst = (size_t)vmm_arch_v2p(as->arch_context, vadest);
	size_t idx = 0;
	while(idx < len)
	{
		dst = (size_t)vmm_arch_v2p(as->arch_context, vadest);
		memcpy((void*)dst, (void*)src, 256);
		idx += 256;
		vadest = (void*)((size_t)vadest + 256);
		src += 256;
	}
}
