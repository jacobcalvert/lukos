#include <managers/virtual-memory-manager.h>
#include <libraries/mem/memlib.h>
#include <mmu/mmu.h>
#include <string.h>


#define KB									1024
#define MB									KB*KB

#define TRANSLATION_TABLE_ALLOCATION		(2*MB)
#define IS_ALIGNED_TO(addr, mask)		( (	(size_t)addr & (size_t)mask	) == (size_t)addr ) 

#define UPALIGN_BY_MASK(addr, mask, size)		( ( ((uint64_t)addr & mask) == (uint64_t)addr )?(uint64_t)addr:(((uint64_t)addr & (uint64_t)mask) + (uint64_t)size))

typedef struct
{
	uint64_t *translation_table;
	
	void * table_space;
	size_t table_space_len;
	
}aarch64_vmm_context_t;


static void* vmm_arch_next_table(void* ctx);



void *vmm_arch_context_create(address_space_t *as)
{
	aarch64_vmm_context_t *context = (aarch64_vmm_context_t*)memlib_malloc(sizeof(aarch64_vmm_context_t));
	context->table_space = (void*)UPALIGN_BY_MASK(memlib_malloc(TRANSLATION_TABLE_ALLOCATION + PAGE_SIZE_4K), PAGE_MASK_4K, PAGE_SIZE_4K);
	context->table_space_len = TRANSLATION_TABLE_ALLOCATION;
	context->translation_table = (uint64_t*)vmm_arch_next_table(context);
	return (void*)context;
}

void *vmm_arch_get_free_va_range(void *ctx, size_t len)
{
	/* probe in blocks */
	size_t page_size = PAGE_SIZE_4K;
	if(len > PAGE_SIZE_2M)
	{
		page_size = PAGE_SIZE_2M;
		
		if(len > PAGE_SIZE_1G)
		{
			page_size = PAGE_SIZE_1G;
		}
	}
	
	size_t no_pages = (len/page_size);
	if( (len % page_size) != 0)
	{
		no_pages ++;
	}
	
	size_t address = 0;
	size_t potential = 0;
	size_t free_pages = 0;
	while(address < VIRTADDR_BASE)
	{
		while(vmm_arch_v2p(ctx, (void*)address) != NULL)
		{
			address += page_size;
		}
		free_pages = 0;
		potential = address;
		while(vmm_arch_v2p(ctx, (void*)address) == NULL)
		{
			free_pages++;
			address += page_size;
			if(free_pages >= no_pages)
			{
				goto vmm_arch_get_free_range_return_success;
			}
		}
	
	}
	return NULL;
vmm_arch_get_free_range_return_success:
	return (void*)potential;	
}	


void* vmm_arch_alloc_pa_range(size_t len)
{
	size_t page_size = PAGE_SIZE_4K;
	size_t page_mask = PAGE_MASK_4K;
	if(len > PAGE_SIZE_2M)
	{
		page_size = PAGE_SIZE_2M;
		page_mask = PAGE_MASK_2M;
		
		if(len > PAGE_SIZE_1G)
		{
			page_size = PAGE_SIZE_1G;
			page_mask = PAGE_MASK_1G;
		}
	}
	
	size_t no_pages = (len/page_size);
	if( (len % page_size) != 0)
	{
		no_pages ++;
	}
	
	void *pa = memlib_malloc( (no_pages+1)*page_size);
	pa = (void*)UPALIGN_BY_MASK(pa, page_mask, page_size);
	return pa;
}


int vmm_arch_map(void *ctx, address_space_region_prop_t props,  void *va, void *pa, size_t len)
{
	size_t attr = 0;
	switch(props)
	{
		case AS_REGION_RW: attr = TBL_LOWER_ATTR_USER_MEM_RW;break;
		case AS_REGION_RX: attr = TBL_LOWER_ATTR_USER_MEM_RX;break;
		case AS_REGION_RO: attr = TBL_LOWER_ATTR_USER_MEM_RX;break;
		case AS_REGION_NO_ACCESS: attr = TBL_LOWER_ATTR_KERNEL_MEM_RW;break;
		default:attr = TBL_LOWER_ATTR_KERNEL_MEM_RW;break;
	}
	
	return aarch64_mmu_map_space((void*)((aarch64_vmm_context_t*)ctx)->translation_table, va, pa, len, attr, vmm_arch_next_table, (void*) ctx);
	
}


void *vmm_arch_v2p(void *ctx, void *va)
{
	aarch64_vmm_context_t *context = (aarch64_vmm_context_t*) ctx;
	return aarch64_mmu_v2p((void*)context->translation_table, va);
}
int vmm_arch_align_check(void *va, size_t len)
{

	size_t page_mask = PAGE_MASK_4K;
	if(len > PAGE_SIZE_2M)
	{

		page_mask = PAGE_MASK_2M;
		
		if(len > PAGE_SIZE_1G)
		{

			page_mask = PAGE_MASK_1G;
		}
	}
	
	return IS_ALIGNED_TO(va, page_mask)?0:-1;

}
void* vmm_arch_next_table(void* arg)
{
	void*next = NULL;
	aarch64_vmm_context_t *ctx = (aarch64_vmm_context_t*)arg;
	if(ctx->table_space_len >= PAGE_SIZE_4K)
	{
		next = ctx->table_space;
		ctx->table_space += PAGE_SIZE_4K;
		ctx->table_space_len -= PAGE_SIZE_4K;
		memset(next, 0, PAGE_SIZE_4K);
	}
	return next;
}
