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
	void *pa = vmm_arch_alloc_pa_range(len);
	
	
	if(vmm_arch_map(as->arch_context, prop, va, pa, len) != 0)
	{
		return -1;
	}
	*vadest = va;
	return 0;

}

int vmm_address_space_region_create(address_space_t *as, void *vadest, size_t len, address_space_region_prop_t prop)
{
	void *va = vadest;
	void *pa = vmm_arch_alloc_pa_range(len);
	if(vmm_arch_align_check(vadest, len) != 0 || vmm_arch_map(as->arch_context, prop, va, pa, len) != 0)
	{
		return -1;
	}
	return 0;
}

void vmm_address_space_copy_in(address_space_t *as, void *vakernel, void *vadest, size_t len)
{
	void *src = vakernel;
	void *dst = vmm_arch_v2p(as->arch_context, vadest);
	void *dstend = vmm_arch_v2p(as->arch_context, (void*)( (size_t)vadest + (len-1)));
	if(dstend != NULL)
	{
		memcpy(dst, src, len);
	}

}
