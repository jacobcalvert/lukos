#include <stddef.h>
#include <stdint.h>

#include <libraries/fdt/fdtlib.h>
#include <libraries/mem/memlib.h>
#include <mmu/mmu.h>
#include <string.h>

#define SIZE_4K				(1<<12)
#define SIZE_2M				(1<<21)
#define UPALIGN_4K(n)		( ( ((uint64_t)n & 0xFFFFFFFFFFFFF000) == (uint64_t)n )?(uint64_t)n:(((uint64_t)n & (uint64_t)0xFFFFFFFFFFFFF000) + (uint64_t)0x1000))

#define IS_ALIGNED_TO(addr, mask)		( (	(size_t)addr & (size_t)mask	) == (size_t)addr ) 

#define KERNEL_TABLE_RESERVED_SPACE		(16*1024*1024)

#define NUM_KERNEL_STACK_PAGES			4
#define KERNEL_STACK_BASE(n)			(0xFFFFFFFFFFFFFFFF - ((n+1)*NUM_KERNEL_STACK_PAGES*PAGE_SIZE_4K) + 1)

static int fdt_memory_node_finder_cb(char *path, void *arg);

static size_t KERNEL_TABLE_BASE = 0;

void *KERNEL_LOW_ADDR_MAP = NULL;
void *KERNEL_HIGH_ADDR_MAP = NULL;
void *KERNEL_LOW_EXE_ADDR_MAP = NULL;


extern char __end_paddr;
extern char __text_start_paddr;
extern char __text_start_vaddr;
extern char __text_size;
extern char __rodata_start_paddr;
extern char __rodata_start_vaddr;
extern char __rodata_size;
extern char __data_start_paddr;
extern char __data_start_vaddr;
extern char __data_size;
extern char __bss_start_paddr;
extern char __bss_start_vaddr;
extern char __bss_size;

static size_t lma_end_addr = (size_t)&__end_paddr;
static size_t lma_text_base = (size_t)&__text_start_paddr;
static size_t vma_text_base = (size_t)&__text_start_vaddr;
static size_t text_size	= (size_t)&__text_size;
static size_t lma_rodata_base = (size_t)&__rodata_start_paddr;
static size_t vma_rodata_base = (size_t)&__rodata_start_vaddr;
static size_t rodata_size	= (size_t)&__rodata_size;
static size_t lma_data_base = (size_t)&__data_start_paddr;
static size_t vma_data_base = (size_t)&__data_start_vaddr;
static size_t data_size	= (size_t)&__data_size;
static size_t lma_bss_base = (size_t)&__bss_start_paddr;
static size_t vma_bss_base = (size_t)&__bss_start_vaddr;
static size_t bss_size	= (size_t)&__bss_size;

void aarch64_mmu_init(void)
{
	/* in this routine we need to:
	 * 1) get our physical memory layout from FDT
	 * 2) allocate some RAM for page tables from it 
	 * 3) remap TTBR0/1 to these page tables and save them as kernel context view
	 *
	 *
	 *
	 */ 	 
	 size_t ram_base_phys = 0;
	 size_t ram_size = 0;
	 char memory_node_path[64];
	 extern const memlib_ops_t MEMLIB_IMPL_BASIC_OPS;
	 
	 fdtlib_find_by_prop("device_type", "memory", fdt_memory_node_finder_cb, (void*)memory_node_path);
	 
	 uint32_t *memory_regs = (uint32_t*)fdtlib_get_prop(memory_node_path, "reg");
	  	 
 	 uint32_t no_size_cells = fdtlib_conv_u32(fdtlib_get_prop("/", "#size-cells"));
  	 uint32_t no_addr_cells = fdtlib_conv_u32(fdtlib_get_prop("/", "#address-cells"));
  	 
  	 /* we assume that there's only one pair... spec doesn't confirm nor deny this */
  	 if(no_addr_cells == 2)
  	 {
  	 	ram_base_phys = (size_t)fdtlib_conv_u64(&memory_regs[0]);
  	 }
  	 else
  	 {
  	 	ram_base_phys = (size_t)fdtlib_conv_u32(&memory_regs[0]);
  	 }
  	 
  	 if(no_size_cells == 2)
  	 { 
	  	 ram_size = (size_t)fdtlib_conv_u64(&memory_regs[2]);
  	 }
  	 else
  	 {
  	 	ram_size = (size_t)fdtlib_conv_u32(&memory_regs[1]);
  	 }
  	 
  	 size_t page_table_base_phys = (size_t)lma_end_addr;
  	 
  	 page_table_base_phys = (size_t)UPALIGN_4K(page_table_base_phys);
  	 KERNEL_TABLE_BASE = page_table_base_phys;
  	
  	 
  	 KERNEL_LOW_ADDR_MAP = aarch64_mmu_allocate_kernel_table(NULL);
   	 KERNEL_LOW_EXE_ADDR_MAP = aarch64_mmu_allocate_kernel_table(NULL);
   	 KERNEL_HIGH_ADDR_MAP = aarch64_mmu_allocate_kernel_table(NULL);
   	 
   	 /* let's identity map 16G of the low addresses as R/W*/ 
   	 aarch64_mmu_map_space(KERNEL_LOW_ADDR_MAP, (void*)0, (void*)0, (size_t)16*PAGE_SIZE_1G, TBL_LOWER_ATTR_KERNEL_MEM_RW, aarch64_mmu_allocate_kernel_table, NULL);
 	/* lets create a RX mapping for secondary boot code*/
	 aarch64_mmu_map_space(KERNEL_LOW_EXE_ADDR_MAP, (void*)0, (void*)0, (size_t)16*PAGE_SIZE_1G, TBL_LOWER_ATTR_KERNEL_MEM_RX, aarch64_mmu_allocate_kernel_table, NULL);
  	 
	 __asm ("msr ttbr0_el1, %[KERNEL_LOW_ADDR_MAP]" : : [KERNEL_LOW_ADDR_MAP] "r" (KERNEL_LOW_ADDR_MAP));
	 __asm ("isb");
	 
	 /* map our kernel spaces and current SP*/
	aarch64_mmu_map_space(KERNEL_HIGH_ADDR_MAP, (void*)vma_text_base, (void*)lma_text_base, text_size, TBL_LOWER_ATTR_KERNEL_MEM_RX, aarch64_mmu_allocate_kernel_table, NULL);
 	aarch64_mmu_map_space(KERNEL_HIGH_ADDR_MAP, (void*)vma_rodata_base, (void*)lma_rodata_base, rodata_size, TBL_LOWER_ATTR_KERNEL_MEM_RX, aarch64_mmu_allocate_kernel_table, NULL);
 	aarch64_mmu_map_space(KERNEL_HIGH_ADDR_MAP, (void*)vma_data_base, (void*)lma_data_base, data_size, TBL_LOWER_ATTR_KERNEL_MEM_RW, aarch64_mmu_allocate_kernel_table, NULL);
 	aarch64_mmu_map_space(KERNEL_HIGH_ADDR_MAP, (void*)vma_bss_base, (void*)lma_bss_base, bss_size, TBL_LOWER_ATTR_KERNEL_MEM_RW, aarch64_mmu_allocate_kernel_table, NULL);
 	
 	size_t sp = 0;
 	
 	__asm__ __volatile__("mov %0, sp" : "=r" (sp) : : );
 	void* ttbr1_el1 = 0;
	__asm__ __volatile__("mrs %0 ,ttbr1_el1"    : "=r" (ttbr1_el1) : : );
 	size_t sp_phys = (size_t)aarch64_mmu_v2p(ttbr1_el1, (void*)sp);
 	
 	
 	aarch64_mmu_map_space(KERNEL_HIGH_ADDR_MAP, (void*)( (size_t)sp & 0xFFFFFFFFFFFFF000 ), (void*) ( (size_t)sp_phys & 0xFFFFFFFFFFFFF000 ), PAGE_SIZE_4K, TBL_LOWER_ATTR_KERNEL_MEM_RW, aarch64_mmu_allocate_kernel_table, NULL);
 	__asm ("msr ttbr1_el1, %[KERNEL_HIGH_ADDR_MAP]" : : [KERNEL_HIGH_ADDR_MAP] "r" (KERNEL_HIGH_ADDR_MAP));
 	__asm ("isb");	
 	
 	/* there may be a region below our kernel load address (boot code for example) which we can reclaim */
 	size_t heap_base = ram_base_phys;
 	size_t heap_size = lma_text_base - heap_base;
 	
 	memlib_init((memlib_ops_t*)&MEMLIB_IMPL_BASIC_OPS); /* init the heap */
 	
 	/* only fool with it if it is at least a couple pages */
 	if(heap_size > 4*PAGE_SIZE_4K)
 	{
 		memlib_heap_add((void*)heap_base, heap_size);	
 	}
 	
 	heap_base = page_table_base_phys + KERNEL_TABLE_RESERVED_SPACE;
 	heap_size = (ram_base_phys + ram_size) - heap_base;
 	memlib_heap_add((void*)heap_base, heap_size);	
 	
 	void *kernel_stack = aarch64_mmu_stack_create(0);
 	/* copy stack contents so we can return to the right spot */
 	memcpy((void*)kernel_stack, (void*)(sp & 0xFFFFFFFFFFFFF000), (PAGE_SIZE_4K));
 	
 	sp = (size_t)kernel_stack + (sp - (sp & 0xFFFFFFFFFFFFF000));
 	__asm ("mov sp, %[sp]" : : [sp] "r" (sp));

}

void *aarch64_mmu_stack_create(size_t cpuno)
{
	/* allocate a kernel stack and set it */
 	/* +1 here so we can upalign the base */
 	void *sp_base = memlib_malloc((NUM_KERNEL_STACK_PAGES+1)*PAGE_SIZE_4K);
 	sp_base = (void*) UPALIGN_4K(sp_base);
 	
 	aarch64_mmu_map_space(KERNEL_HIGH_ADDR_MAP, (void*)KERNEL_STACK_BASE(cpuno), (void*)sp_base, (NUM_KERNEL_STACK_PAGES*PAGE_SIZE_4K), TBL_LOWER_ATTR_KERNEL_MEM_RW, aarch64_mmu_allocate_kernel_table, NULL);

	return (void*)KERNEL_STACK_BASE(cpuno);
}

int aarch64_mmu_device_map(void * table, void *pa, size_t size, aarch64_table_allocator_t alloc, void *arg, void **vaout)
{
	size_t pa_addr_offset = (size_t)pa & VIRTADDR_LOWER_MASK;
	size_t va = (pa_addr_offset + VIRTADDR_BASE);
	*vaout = (void*)va;
	return aarch64_mmu_map_space(table, (void*)va, (void*) pa, (size_t)size, TBL_LOWER_ATTR_KERNEL_DEVICE, alloc, arg);
}

int aarch64_mmu_map_space(void *table, void *va_start, void*pa_start, size_t len, size_t attr, aarch64_table_allocator_t alloc, void *arg)
{
	/*
	 * 1) figure out what mix of pages will be used
	 * 2) map it
	 */
	 /*						1G, 2M, 4K */
	 size_t no_pages[3] = 	{0, 0, 	0};
	 size_t va = (size_t) va_start;
	 size_t pa = (size_t) pa_start;
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
	 
	 /* check alignement */
	 if(!IS_ALIGNED_TO(va_start, PAGE_MASK_4K) || !IS_ALIGNED_TO(pa_start, PAGE_MASK_4K))
	 {
	 	while(1); /*TODO */
	 } 

	 
	 for(size_t i = 0; i < no_pages[0]; i++)
	 {
	 	if(aarch64_mmu_map_1G(table, (void*)va,(void*)pa, attr, alloc, arg) != 0)
	 	{
	 		while(1); /* TODO */
	 	}
	 	va += PAGE_SIZE_1G;
	 	pa += PAGE_SIZE_1G;
	 }
	 
	 	 
	 for(size_t i = 0; i < no_pages[1]; i++)
	 {
	 	if(aarch64_mmu_map_2M(table, (void*)va,(void*)pa, attr, alloc, arg) != 0)
	 	{
	 		while(1); /* TODO */
	 	}
	 	va += PAGE_SIZE_2M;
	 	pa += PAGE_SIZE_2M;
	 }
	 
	 	 	 
	 for(size_t i = 0; i < no_pages[2]; i++)
	 {
	 	if(aarch64_mmu_map_4K(table, (void*)va,(void*)pa, attr, alloc, arg) != 0)
	 	{
	 		while(1); /* TODO */
	 	}
	 	va += PAGE_SIZE_4K;
	 	pa += PAGE_SIZE_4K;
	 }
	 

	 return 0;
	 
}

int aarch64_mmu_map_4K(void *tbl, void *va, void *pa, uint64_t attr, aarch64_table_allocator_t alloc, void *arg)
{
	size_t L1_INDEX, L2_INDEX, L3_INDEX;
	aarch64_mmu_trans_table_indicies_get(va, &L1_INDEX, &L2_INDEX, &L3_INDEX);
	uint64_t *table = (uint64_t*)tbl;
	uint64_t entry = 0;
	if(!IS_ALIGNED_TO(va, PAGE_MASK_4K) || !IS_ALIGNED_TO(pa, PAGE_MASK_4K))
	{
		return -1;
	}
	
	entry = table[L1_INDEX];
	
	if(entry != 0)
	{
		table = (uint64_t*)aarch64_mmu_trans_table_get_next(entry);
		entry = table[L2_INDEX];
		
		if( entry != 0)
		{
			table = (uint64_t*)aarch64_mmu_trans_table_get_next(entry);
			entry = table[L3_INDEX];
			if(entry != 0)
			{
				return -1;
			}
			else
			{
				entry = (uint64_t)pa | TBL_TYPE_BLOCK_L3 | attr;
				table[L3_INDEX] = entry;
				return 0;
			}
			
		}
		else
		{
			/* make L2 Table */
			void *newtable = (uint64_t*)alloc(arg);
			entry =  (uint64_t)newtable | TBL_TYPE_TABLE_DESC;
			table[L2_INDEX] = entry;
			return aarch64_mmu_map_4K(tbl, va, pa, attr, alloc, arg);
		}
		
	}
	else
	{
		/* make L1 Table */
		void *newtable = (uint64_t*)alloc(arg);
		entry =  (uint64_t)newtable | TBL_TYPE_TABLE_DESC;
		table[L1_INDEX] = entry;
		return aarch64_mmu_map_4K(tbl, va, pa, attr, alloc, arg);
	}
	
	
	return -1;
	
}

int aarch64_mmu_map_2M(void *tbl, void *va, void *pa, uint64_t attr, aarch64_table_allocator_t alloc, void *arg)
{
	size_t L1_INDEX, L2_INDEX, L3_INDEX;
	aarch64_mmu_trans_table_indicies_get(va, &L1_INDEX, &L2_INDEX, &L3_INDEX);
	uint64_t *table = (uint64_t*)tbl;
	uint64_t entry = 0;
	if(!IS_ALIGNED_TO(va, PAGE_MASK_2M) || !IS_ALIGNED_TO(pa, PAGE_MASK_2M))
	{
		return -1;
	}
	
	entry = table[L1_INDEX];
	
	if(entry != 0)
	{
		table = (uint64_t*)aarch64_mmu_trans_table_get_next(entry);
		entry = table[L2_INDEX];
		
		if( entry != 0)
		{
			return -1;
			
		}
		else
		{
			entry = (uint64_t)pa | TBL_TYPE_BLOCK_L12 | attr;
			table[L2_INDEX] = entry;
			return 0;
		}
		
	}
	else
	{
		/* make L1 Table */
		void *newtable = (uint64_t*)alloc(arg);
		entry =  (uint64_t)newtable | TBL_TYPE_TABLE_DESC;
		table[L1_INDEX] = entry;
		return aarch64_mmu_map_2M(tbl, va, pa, attr, alloc, arg);
	}
	
	
	return -1;
	
}

int aarch64_mmu_map_1G(void *tbl, void *va, void *pa, uint64_t attr, aarch64_table_allocator_t alloc, void *arg)
{
	size_t L1_INDEX, L2_INDEX, L3_INDEX;
	aarch64_mmu_trans_table_indicies_get(va, &L1_INDEX, &L2_INDEX, &L3_INDEX);
	uint64_t *table = (uint64_t*)tbl;
	uint64_t entry = 0;
	if(!IS_ALIGNED_TO(va, PAGE_MASK_1G) || !IS_ALIGNED_TO(pa, PAGE_MASK_1G))
	{
		return -1;
	}
	
	entry = table[L1_INDEX];
	
	if(entry != 0)
	{
		return -1;
		
	}
	else
	{
		entry = (uint64_t)pa | TBL_TYPE_BLOCK_L12 | attr;
		table[L1_INDEX] = entry;
		return 0;
	}
	
	
	return -1;
	
}

void *aarch64_mmu_trans_table_get_next(uint64_t entry)
{
	void * next = NULL;
	if( (entry & TBL_TYPE_TABLE_DESC ) == TBL_TYPE_TABLE_DESC)
	{
		next = (void*)(entry & (uint64_t)~TBL_TYPE_TABLE_DESC); 
	}
	
	return next;
}
void *aarch64_mmu_allocate_kernel_table(void * arg)
{
	void *next = (void*)UPALIGN_4K(KERNEL_TABLE_BASE);
	KERNEL_TABLE_BASE += 0x1000;
	memset(next, 0, 0x1000);
	return next;
}
void aarch64_mmu_trans_table_indicies_get(void *virtaddr, size_t *l1, size_t *l2, size_t *l3)
{
	size_t va = (size_t)virtaddr;
	va &= (size_t)VIRTADDR_LOWER_MASK;
	va >>= 12; /* take off the 4k block at bottom */
	 /* get 9 bits */
	if(l3)*l3 = va & 0x01FF;
	va >>= 9;
	if(l2)*l2 = va & 0x01FF;
	va >>= 9;
	if(l1)*l1 = va & 0x01FF;
}
int fdt_memory_node_finder_cb(char *path, void *arg)
{
	strncpy((char*)arg, path, strlen(path));
	return 0; 
}

void *aarch64_mmu_v2p(void *pTable, void *va)
{
	void *pa = NULL;
	size_t l1, l2, l3;
	aarch64_mmu_trans_table_indicies_get(va, &l1, &l2, &l3);
	uint64_t *table = (uint64_t*) pTable;
	if(table[l1] != 0)
	{
		uint64_t entry = table[l1];
		if((entry &TBL_TYPE_TABLE_DESC) == TBL_TYPE_TABLE_DESC)
		{
			table = (uint64_t*) (entry & ~TBL_TYPE_TABLE_DESC);
			entry = table[l2];
			if(entry != 0)
			{
				if((entry &TBL_TYPE_TABLE_DESC) == TBL_TYPE_TABLE_DESC)
				{
					table = (uint64_t*) (entry & ~TBL_TYPE_TABLE_DESC);
					entry = table[l3];
					if(entry != 0)
					{
						/* block 4KB*/
						entry&= PAGE_MASK_4K;
						entry += (~PAGE_MASK_4K & (uint64_t)va);
						pa = (void*) entry;
					}
				}
				else
				{
					/* block 2MB */
					entry&= PAGE_MASK_2M;
					entry += (~PAGE_MASK_2M & (uint64_t)va);
					pa = (void*) entry;
				}
			
			}
		}
		else
		{
			/*block 1GB */
			entry&= PAGE_MASK_1G;
			entry += (~PAGE_MASK_1G & (uint64_t)va);
			pa = (void*) entry;
		
		}
	}
	
	
	
	
	return pa;

}
 


