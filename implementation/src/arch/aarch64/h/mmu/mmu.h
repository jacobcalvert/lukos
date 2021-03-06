#ifndef __AARCH64_MMU__
#define __AARCH64_MMU__

#define KERNEL_HEAP_SIZE_BYTES	(32*1024*1024) /* 32MiB */

#define TCR_ELx_T0SZ(n)         ((uint64_t)(n))
#define TCR_ELx_IRGN0(n)        ((uint64_t)(n)<<8)
#define TCR_ELx_ORGN0(n)        ((uint64_t)(n)<<10)
#define TCR_ELx_SH0(n)          ((uint64_t)(n)<<12)
#define TCR_ELx_TG0(n)          ((uint64_t)(n)<<14)
#define TCR_ELx_T1SZ(n)         ((uint64_t)(n)<<16)
#define TCR_ELx_A1(n)           ((uint64_t)(n)<<22)
#define TCR_ELx_EPD1(n)         ((uint64_t)(n)<<23)
#define TCR_ELx_IRGN1(n)        ((uint64_t)(n)<<24)
#define TCR_ELx_ORGN1(n)        ((uint64_t)(n)<<26)
#define TCR_ELx_SH1(n)          ((uint64_t)(n)<<28)
#define TCR_ELx_TG1(n)          ((uint64_t)(n)<<30)
#define TCR_ELx_IPS(n)          ((uint64_t)(n)<<32)
#define TCR_ELx_AS(n)           ((uint64_t)(n)<<36)
#define TCR_ELx_TBI0(n)         ((uint64_t)(n)<<37)
#define TCR_ELx_TBI1(n)         ((uint64_t)(n)<<38)			


#define TBL_TYPE_TABLE_DESC         (0b011)
#define TBL_TYPE_BLOCK_L12          (0b001)
#define TBL_TYPE_BLOCK_L3           (0b011)
#define TBL_TYPE_INVALID            (0b000)

#define TBL_LOWER_ATTR_MAIR_IDX(n)  ((uint64_t)(n)<<2)
#define TBL_LOWER_ATTR_NS_BIT(n)    ((uint64_t)(n)<<5)
#define TBL_LOWER_ATTR_AP(n)        ((uint64_t)(n)<<6)
#define TBL_LOWER_ATTR_SH(n)        ((uint64_t)(n)<<8)
#define TBL_LOWER_ATTR_AF(n)        ((uint64_t)(n)<<10)


#define TBL_LOWER_ATTR_KERNEL_MEM_RW	(TBL_LOWER_ATTR_SH(2) | TBL_LOWER_ATTR_AF(1) | TBL_LOWER_ATTR_AP(0) | TBL_LOWER_ATTR_MAIR_IDX(MAIR_IDX_RAM_NONCACHEABLE))
#define TBL_LOWER_ATTR_KERNEL_MEM_RX	(TBL_LOWER_ATTR_SH(2) | TBL_LOWER_ATTR_AF(1) | TBL_LOWER_ATTR_AP(2) | TBL_LOWER_ATTR_MAIR_IDX(MAIR_IDX_RAM_NONCACHEABLE))
#define TBL_LOWER_ATTR_USER_MEM_RW		(TBL_LOWER_ATTR_SH(2) | TBL_LOWER_ATTR_AF(1) | TBL_LOWER_ATTR_AP(1) | TBL_LOWER_ATTR_MAIR_IDX(MAIR_IDX_RAM_NONCACHEABLE))	
#define TBL_LOWER_ATTR_USER_MEM_RX		(TBL_LOWER_ATTR_SH(2) | TBL_LOWER_ATTR_AF(1) | TBL_LOWER_ATTR_AP(3) | TBL_LOWER_ATTR_MAIR_IDX(MAIR_IDX_RAM_NONCACHEABLE))		
#define TBL_LOWER_ATTR_KERNEL_DEVICE	(TBL_LOWER_ATTR_SH(2) | TBL_LOWER_ATTR_AF(1) | TBL_LOWER_ATTR_AP(0) | TBL_LOWER_ATTR_MAIR_IDX(MAIR_IDX_DEVICE))
#define TBL_LOWER_ATTR_USER_DEVICE		(TBL_LOWER_ATTR_SH(2) | TBL_LOWER_ATTR_AF(1) | TBL_LOWER_ATTR_AP(0) | TBL_LOWER_ATTR_MAIR_IDX(MAIR_IDX_DEVICE))

#define TBL_UPPER_ATTR_PXN(n)       ((uint64_t)(n)<<53)
#define TBL_UPPER_ATTR_UXN(n)       ((uint64_t)(n)<<54) 
#define TBL_UPPER_ATTR_SWRESV(n)    ((uint64_t)(n)<<55)


#define TBL_MAKE_INVALID(desc)      ((desc) &= ~TBL_TYPE_INVALID)


#define NUM_VIRTADDR_BITS			39
#define VIRTADDR_UPPER_MASK			0xFFFFFF8000000000
#define VIRTADDR_BASE				0xFFFFFF8000000000
#define VIRTADDR_LOWER_MASK			(~VIRTADDR_UPPER_MASK)

#define PAGE_SIZE_4K				(0x0000000000001000)
#define PAGE_MASK_4K				(0xFFFFFFFFFFFFF000)

#define PAGE_SIZE_2M				(0x0000000000200000)
#define PAGE_MASK_2M				(0xFFFFFFFFFFE00000)

#define PAGE_SIZE_1G				(0x0000000040000000)
#define PAGE_MASK_1G				(0xFFFFFFFFC0000000)

#define TCR_T0SZ					(64-NUM_VIRTADDR_BITS)
#define TCR_T1SZ					(TCR_T0SZ)
#define TCR_IRGN					(0)
#define TCR_ORGN					(0)
#define TCR_SH						(2)
#define TCR_TG						(0)
#define TCR_IPS						(1)

#define MAIR_EL1_ATTR0                       0x00        /* device, nGnRnE */
#define MAIR_EL1_ATTR1                      (0x04<<8)   /* device, nGnRnE */
#define MAIR_EL1_ATTR2                      (0x44<<16)  /* normal non-cacheable */
#define MAIR_EL1_ATTR3                      (0xFF<<24)  /* normal, cacheable */

#define MAIR_IDX_DEVICE						0
#define MAIR_IDX_RAM_NONCACHEABLE			2
#define MAIR_IDX_RAM_CACHEABLE				3

#define TCR_EL1_DEFAULT 			TCR_ELx_T0SZ(TCR_T0SZ) | \
		TCR_ELx_IRGN0(TCR_IRGN)    	| \
		TCR_ELx_ORGN0(TCR_ORGN)    	| \
		TCR_ELx_SH0(TCR_SH)      	| \
		TCR_ELx_T1SZ(TCR_T1SZ) 		| \
		TCR_ELx_IRGN1(TCR_IRGN)    	| \
		TCR_ELx_ORGN1(TCR_ORGN)    	| \
		TCR_ELx_SH1(TCR_SH)			| \
		TCR_ELx_TG0(TCR_TG)			| \
		TCR_ELx_TG1(TCR_TG)			| \
		TCR_ELx_IPS(TCR_IPS)

#define MAIR_EL1_DEFAULT	(MAIR_EL1_ATTR3 | MAIR_EL1_ATTR2 | MAIR_EL1_ATTR1 | MAIR_EL1_ATTR0)
	
	


#define NUM_TRANSLATION_LEVELS				3

#define TRANSLATION_ENTRY_SIZE_BYTES		8

#define L1_TABLE_RESOLVE_BITS				9
#define L1_TABLE_SIZE_BYTES					((1<<L1_TABLE_RESOLVE_BITS)*TRANSLATION_ENTRY_SIZE_BYTES)
#define L1_ENTRY_STRIDE_BYTES				(PAGE_SIZE_1G)
#define NUM_L1_ENTRIES						(1<<L1_TABLE_RESOLVE_BITS)


#define L2_TABLE_RESOLVE_BITS				9
#define L2_TABLE_SIZE_BYTES					((1<<L2_TABLE_RESOLVE_BITS)*TRANSLATION_ENTRY_SIZE_BYTES)
#define L2_ENTRY_STRIDE_BYTES				(PAGE_SIZE_2M)
#define NUM_L2_ENTRIES						(1<<L2_TABLE_RESOLVE_BITS)


#define L3_TABLE_RESOLVE_BITS				9
#define L3_TABLE_SIZE_BYTES					((1<<L3_TABLE_RESOLVE_BITS)*TRANSLATION_ENTRY_SIZE_BYTES)
#define L3_ENTRY_STRIDE_BYTES				(PAGE_SIZE_4K)
#define NUM_L3_ENTRIES						(1<<L3_TABLE_RESOLVE_BITS)

#ifndef __ASSEMBLER__


#include <interfaces/os/atomic.h>
#include <stdint.h>

#define TABLE_LOCK(tbl)					(atomic32_spinlock_acquire(&tbl->lock))
#define TABLE_UNLOCK(tbl)					(atomic32_spinlock_release(&tbl->lock))

typedef struct
{
	uint64_t *translation_table;
	
	void * table_space;
	size_t table_space_len;
	
	uint32_t lock;
	
}aarch64_vmm_context_t;


extern aarch64_vmm_context_t KERNEL_LOW_ADDR_MAP;
extern aarch64_vmm_context_t KERNEL_LOW_EXE_ADDR_MAP;
extern aarch64_vmm_context_t KERNEL_HIGH_ADDR_MAP;

typedef void *(*aarch64_table_allocator_t)(void* arg);
;
/**
 * initialize the MMU and setup the kernel heap in virtual space
 */
void aarch64_mmu_init(void);

/**
 * 
 */
void *aarch64_mmu_stack_create(size_t cpuno);

/**
 * translate a VA to a PA with the given table
 * @param table		the TTBRx
 * @param va		the VA
 * @return the PA
 */
void *aarch64_mmu_v2p(aarch64_vmm_context_t *table, void *va);

/**
 * map a device (auto selects the VA)
 */
int aarch64_mmu_device_map(aarch64_vmm_context_t * table, void *pa, size_t size, aarch64_table_allocator_t alloc, void *arg, void **vaout);

/**
 * 
 */
int aarch64_mmu_map_space(aarch64_vmm_context_t *table, void *va_start, void*pa_start, size_t len, size_t attr, aarch64_table_allocator_t alloc, void *arg);
int aarch64_mmu_map_4K(aarch64_vmm_context_t *table, void *va, void *pa, uint64_t attr, aarch64_table_allocator_t alloc, void *arg);
int aarch64_mmu_map_2M(aarch64_vmm_context_t *table, void *va, void *pa, uint64_t attr, aarch64_table_allocator_t alloc, void *arg);
int aarch64_mmu_map_1G(aarch64_vmm_context_t *table, void *va, void *pa, uint64_t attr, aarch64_table_allocator_t alloc, void *arg);

void aarch64_mmu_trans_table_indicies_get(void *virtaddr, size_t *l1, size_t *l2, size_t *l3);

void *aarch64_mmu_trans_table_get_next(uint64_t entry); 

void *aarch64_mmu_allocate_kernel_table(void * arg);



#endif

#endif
