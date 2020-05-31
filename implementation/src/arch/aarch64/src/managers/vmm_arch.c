#include <managers/virtual-memory-manager.h>
#include <libraries/mem/memlib.h>
#include <mmu/mmu.h>
#include <managers/vmm_arch.h>
#include <string.h>


#define KB									1024
#define MB									KB*KB

#define TRANSLATION_TABLE_ALLOCATION		(4*MB)
#define IS_ALIGNED_TO(addr, mask)		( (	(size_t)addr & (size_t)mask	) == (size_t)addr ) 

#define UPALIGN_BY_MASK(addr, mask, size)		( ( ((uint64_t)addr & mask) == (uint64_t)addr )?(uint64_t)addr:(((uint64_t)addr & (uint64_t)mask) + (uint64_t)size))

#define UPALIGN_4K(n)		( ( ((uint64_t)n & 0xFFFFFFFFFFFFF000) == (uint64_t)n )?(uint64_t)n:(((uint64_t)n & (uint64_t)0xFFFFFFFFFFFFF000) + (uint64_t)0x1000))

static void* vmm_arch_next_table(void* ctx);




void *vmm_arch_context_create(address_space_t *as)
{
	aarch64_vmm_context_t *context = (aarch64_vmm_context_t*)memlib_malloc(sizeof(aarch64_vmm_context_t));
	context->table_space = (void*) UPALIGN_4K(memlib_malloc(TRANSLATION_TABLE_ALLOCATION + PAGE_SIZE_4K));
	context->table_space_len = TRANSLATION_TABLE_ALLOCATION;
	context->translation_table = (uint64_t*)vmm_arch_next_table(context);
	context->lock = 0;
	return (void*)context;
}

void *vmm_arch_get_free_va_range(void *ctx, size_t len)
{
	size_t no_pages[3] = 	{0, 0, 	0};
	size_t pg_sizes[3] = 	{PAGE_SIZE_1G, PAGE_SIZE_2M, PAGE_SIZE_4K};
	size_t pgszi = 2;
	size_t local_len = len;
	
	while(local_len >= PAGE_SIZE_1G)
	{
	local_len -= PAGE_SIZE_1G;
	no_pages[0]++;
	}

	while(local_len >= PAGE_SIZE_2M )
	{
	local_len -= PAGE_SIZE_2M;
	no_pages[1]++;
	}

	while( local_len >= PAGE_SIZE_4K )
	{
	local_len -= PAGE_SIZE_4K;
	no_pages[2]++;
	}

	if(local_len != 0)
	{
	no_pages[2]++; /* left overs */
	}
	
	size_t address = 0;
	size_t potential = 0;
	size_t free_pages = 0;
	size_t top_page_idx = 2;
	while(no_pages[top_page_idx] == 0) top_page_idx--;
	TABLE_LOCK(((aarch64_vmm_context_t*)ctx));
	while(address < VIRTADDR_BASE)
	{
skiploop_start:
		/* skip over the used pages */
		while(vmm_arch_v2p(ctx, (void*)address) != NULL)
		{
			address += pg_sizes[top_page_idx];
		}
		/* work our way down in a contiguous block */
		potential = address;
		pgszi = top_page_idx;
		while(pgszi)
		{
			size_t npgs = no_pages[pgszi];
			while(npgs--)
			{
				size_t page_size = pg_sizes[pgszi];
				if(vmm_arch_v2p(ctx, (void*)address) != NULL)
				{
					goto skiploop_start;
				}
				
				address += page_size;
				
			
			
				
			}
			pgszi--;
		}
		TABLE_UNLOCK(((aarch64_vmm_context_t*)ctx));
		return (void*)potential;
		
	
	}
	TABLE_UNLOCK(((aarch64_vmm_context_t*)ctx));
	return NULL;
}	


void* vmm_arch_alloc_pa_range(size_t len)
{
	size_t page_size = PAGE_SIZE_4K;
	size_t page_mask = PAGE_MASK_4K;
	
	size_t no_pages = (len/page_size);
	if( (len % page_size) != 0)
	{
		no_pages ++;
	}
	
	void *pa = memlib_malloc( (no_pages+1)*PAGE_SIZE_4K);
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
	
	return aarch64_mmu_map_space((aarch64_vmm_context_t*)ctx, va, pa, len, attr, vmm_arch_next_table, (void*) ctx);
	
}


void *vmm_arch_v2p(void *ctx, void *va)
{
	aarch64_vmm_context_t *context = (aarch64_vmm_context_t*) ctx;
	return aarch64_mmu_v2p((aarch64_vmm_context_t*)context, va);
}
int vmm_arch_align_check(void *va, size_t len)
{

	size_t page_mask = PAGE_MASK_4K;
	
	return IS_ALIGNED_TO(va, page_mask)?0:-1;

}
void* vmm_arch_next_table(void* arg)
{
	void*next = NULL;
	aarch64_vmm_context_t *ctx = (aarch64_vmm_context_t*)arg;
	if(ctx->table_space_len >= PAGE_SIZE_4K)
	{
		next = ctx->table_space;
		ctx->table_space = (void*)((size_t)ctx->table_space + (size_t)PAGE_SIZE_4K);
		ctx->table_space_len -= PAGE_SIZE_4K;
		memset(next, 0, PAGE_SIZE_4K);
	}
	else
	{
		while(1);
	}
	return next;
}
