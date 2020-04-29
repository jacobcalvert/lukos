/**
 * @file boot.S
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date 12Jun2019
 * This file contains the initial boot symbol for the kernel bootup and any SMP init
 *
 */


.section ".text.boot" /* stick this in the .text.boot section */


/*
 * we assume a few things
 *
 * 1) we start in EL1 (we try to handle it if we don't)
 * 2) we only have one core running at boot, we'll enable the others later
 * 3) we need to copy stuff into ram
 */


#define EL3_BITS				0x0C 	//1100
#define EL2_BITS				0x08 	//1000
#define EL1_BITS				0X04 	//0100


/* secure cfg register for el3
 * byte 0 = 0xB1 => NS = 1, SMD = 1
 * byte 1 = 0x05 => HVC =1, RW = 1 
 * 0101_1011_0001
 */
#define SCR_EL3_DEFAULT			0x05B1


/* 
 * sys ctrl reg el3
 * C = 1 (global cache enable)
 */
#define SCTLR_EL3_DEFAULT		0x04


/*
 * hyp config reg 
 *
 * RW = 1 (EL1 is AARCH64)
 * 
 */
#define HCR_EL2_DEFAULT			 0x80000000


/* saved program status reg
 * 
 * 0001_1100_1001
 * 1C9 => 
		M = 1001 = EL2h
		F, I = 11 = Masked IRQ/FIQ
		A = 1 = Serror masked
 * 
 */

#define SPSR_EL3_DEFAULT		0x01C9



/* saved program status reg
 * 0001_1100_0101
 * 1C5 => 
 * 		M = 0101 = EL1h
 *		F, I = 11 = Masked IRQ/FIQ
 *		A = 1 = Masked SError
 */
#define SPSR_EL2_DEFAULT		0x01C5

/*
 * sys ctrl reg for EL1
 * 0001_0000_0000_0100
 * 1004 => 
 *		C = 1, cache on
 * 		I = 1, cache on
 */

#define SCTLR_EL1_DEFAULT		0x0

/*
 * Architectural Feature Access Control Register for EL1
 * (1<<20) => FPEN= 0b01 = El0 SIMD is trapped but not El1
 */
#define CPACR_EL1_DEFAULT		(1<<20)



#define TCR_ELx_T0SZ(n)         ((n))
#define TCR_ELx_IRGN0(n)        ((n)<<8)
#define TCR_ELx_ORGN0(n)        ((n)<<10)
#define TCR_ELx_SH0(n)          ((n)<<12)
#define TCR_ELx_TG0(n)          ((n)<<14)
#define TCR_ELx_T1SZ(n)         ((n)<<16)
#define TCR_ELx_A1(n)           ((n)<<22)
#define TCR_ELx_EPD1(n)         ((n)<<23)
#define TCR_ELx_IRGN1(n)        ((n)<<24)
#define TCR_ELx_ORGN1(n)        ((n)<<26)
#define TCR_ELx_SH1(n)          ((n)<<28)
#define TCR_ELx_TG1(n)          ((n)<<30)
#define TCR_ELx_IPS(n)          ((n)<<32)
#define TCR_ELx_AS(n)           ((n)<<36)
#define TCR_ELx_TBI0(n)         ((n)<<37)
#define TCR_ELx_TBI1(n)         ((n)<<38)			


#define TBL_TYPE_TABLE_DESC         (0b011)
#define TBL_TYPE_BLOCK_L12          (0b001)
#define TBL_TYPE_BLOCK_L3           (0b011)
#define TBL_TYPE_INVALID            (0b000)

#define TBL_LOWER_ATTR_MAIR_IDX(n)  ((n)<<2)
#define TBL_LOWER_ATTR_NS_BIT(n)    ((n)<<5)
#define TBL_LOWER_ATTR_AP(n)        ((n)<<6)
#define TBL_LOWER_ATTR_SH(n)        ((n)<<8)
#define TBL_LOWER_ATTR_AF(n)        ((n)<<10)


#define TBL_UPPER_ATTR_PXN(n)       ((n)<<53)
#define TBL_UPPER_ATTR_UXN(n)       ((n)<<54) 
#define TBL_UPPER_ATTR_SWRESV(n)    ((n)<<55)


#define TBL_MAKE_INVALID(desc)      ((desc) &= ~TBL_TYPE_INVALID)


#define NUM_VIRTADDR_BITS			39
#define VIRTADDR_UPPER_MASK			0xFFFFFF8000000000
#define VIRTADDR_BASE				0xFFFFFF8000000000
#define VIRTADDR_LOWER_MASK			(~VIRTADDR_UPPER_MASK)

#define PAGE_SIZE					(0x0000000000001000)
#define PAGE_MASK					(0xFFFFFFFFFFFFF000)

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

#define NUM_TRANSLATION_LEVELS				3

#define TRANSLATION_ENTRY_SIZE_BYTES		8

#define L1_TABLE_RESOLVE_BITS				9
#define L1_TABLE_SIZE_BYTES					((1<<L1_TABLE_RESOLVE_BITS)*TRANSLATION_ENTRY_SIZE_BYTES)
#define L1_ENTRY_STRIDE_BYTES				(0x40000000)
#define NUM_L1_ENTRIES						(1<<L1_TABLE_RESOLVE_BITS)


#define L2_TABLE_RESOLVE_BITS				9
#define L2_TABLE_SIZE_BYTES					((1<<L2_TABLE_RESOLVE_BITS)*TRANSLATION_ENTRY_SIZE_BYTES)
#define L2_ENTRY_STRIDE_BYTES				(0x200000)
#define NUM_L2_ENTRIES						(1<<L2_TABLE_RESOLVE_BITS)


#define L3_TABLE_RESOLVE_BITS				9
#define L3_TABLE_SIZE_BYTES					((1<<L3_TABLE_RESOLVE_BITS)*TRANSLATION_ENTRY_SIZE_BYTES)
#define L3_ENTRY_STRIDE_BYTES				(PAGE_SIZE)
#define NUM_L3_ENTRIES						(1<<L3_TABLE_RESOLVE_BITS)

#define TBL_TYPE_TABLE_DESC         (0b011)
#define TBL_TYPE_BLOCK_L12          (0b001)
#define TBL_TYPE_BLOCK_L3           (0b011)
#define TBL_TYPE_INVALID            (0b000)

#define TT_1G_BLOCK(PA, ATTR)		.dword (PA + ATTR + TBL_TYPE_BLOCK_L12)
#define TT_2M_BLOCK(PA, ATTR)		.dword (PA + ATTR + TBL_TYPE_BLOCK_L12)
#define TT_4K_BLOCK(PA, ATTR)		.dword (PA + ATTR + TBL_TYPE_BLOCK_L12)

#define TT_NEXT_TABLE(PA)			.dword (PA + TBL_TYPE_TABLE_DESC)

#define TT_ATTR_RW					(TBL_LOWER_ATTR_AP(1) |  TBL_LOWER_ATTR_SH(2) | TBL_LOWER_ATTR_AF(1) | TBL_LOWER_ATTR_MAIR_IDX(3))
#define TT_ATTR_RX					(TBL_LOWER_ATTR_AP(2) |  TBL_LOWER_ATTR_SH(2) | TBL_LOWER_ATTR_AF(1) | TBL_LOWER_ATTR_MAIR_IDX(3))				


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
	
	

.globl _start

_start:
	mrs x0, CurrentEL					/* read the current exception level */	
	and x0, x0, #EL3_BITS				/* and off the unused bits */
	cmp x0, #EL3_BITS					/* compare the result to the EL3 bits */
	bne _el2_setup						/* if x0 != EL3, means we're not in EL3, so go to EL2 setup */
	mov x0, #SCR_EL3_DEFAULT			/* load up the default we want */
	msr scr_el3, x0						/* set it in the scr reg */

	mov x0, #SCTLR_EL3_DEFAULT			/* load up */
	msr sctlr_el3, x0					/* set it */

	mrs	x0, S3_1_C15_C2_1				/* enable SMPEN (cluster cache coherency) */
	orr	x0, x0, #0x40					/* enable SMPEN (cluster cache coherency) */
	msr	S3_1_C15_C2_1, x0				/* enable SMPEN (cluster cache coherency) */

	msr	cptr_el3, xzr					/* enable FPU */

	mov x0, #SPSR_EL3_DEFAULT			/* load up for the jump to EL2 */
	msr spsr_el3, x0					/* set it */
	adr	x0, _el2_setup					/* address of jump -> x0 */
	msr	elr_el3, x0						/* put it into the link reg */
	isb									/* sync */
	eret								/* jump*/
	/* do el3 init */

_el2_setup:
	mrs x0, CurrentEL					/* read the current exception level */	
	and x0, x0, #EL2_BITS
	cmp x0, #EL2_BITS					/* compare the result to the EL3 bits */
	bne _el1_setup						/* if x0 != EL2, means we're not in EL2, so go to EL1 setup */
	mov x0, #HCR_EL2_DEFAULT			/* load up */
	msr hcr_el2, x0						/* set it */

	mov x0, #SPSR_EL2_DEFAULT			/* setup EL2 SPSR */
	msr spsr_el2, x0

	msr	cptr_el2, xzr					/* enable FPU */
	
	adr x0, _el1_setup					/* set the EL1 return address */
	msr elr_el2, x0		
	isb
	eret								/* jump */


_el1_setup:
	ldr x0, =__heap_end_vaddr			/* set the SP to the end of RAM */
	mov sp, x0							/* set it */
	mov x0, #SCTLR_EL1_DEFAULT			/* setup control regs */
	msr sctlr_el1, x0
	mov	x0, #CPACR_EL1_DEFAULT			/* don't trap SIMD, SVE instructions in EL1 */
	msr	cpacr_el1, x0
	
	ldr x0, =__tcr_default				/* setup the translation control register */
	ldr x0, [x0]
	msr tcr_el1, x0			
	
	ldr x0, =__mair_default				/* setup the mem attrs control register */
	ldr x0, [x0]
	msr mair_el1, x0
	
	ldr x0, =__initial_ttbr0_el1		/* setup the tt base registers */
	msr ttbr0_el1, x0
	ldr x0, =__initial_ttbr1_el1
	msr ttbr1_el1, x0
	isb
	mov x0, #SCTLR_EL1_DEFAULT
	orr x0, x0, #0x1
	msr sctlr_el1, x0
	isb
	
	/* load up dtb... it's an ugly hack but QEMU has wrong lenght in field (base + 4), just assume 8k for now*/
	mov x2, #(1<<13)
	ldr x0, = __dtb
	mov x1, #0
	bl strncpy
	ldr x1, =aarch64_init
	br x1
	
	
	
	b halt								/* never gets here hopefully */

/* in x0 out x0 */	
be32:
	rev x0, x0
	ret

halt:							
	b halt
	
__tcr_default:
	.dword TCR_EL1_DEFAULT
	
__mair_default:
	.dword MAIR_EL1_DEFAULT

	
.align 12
__initial_ttbr0_el1:
	TT_1G_BLOCK(0x0000000000000000, TT_ATTR_RW)
	TT_NEXT_TABLE(__initial_l2_user_table)
	TT_1G_BLOCK(0x0000000080000000, TT_ATTR_RW)
	TT_1G_BLOCK(0x00000000C0000000, TT_ATTR_RW)
	
	
.align 12
__initial_l2_user_table:
	TT_2M_BLOCK( (0x40000000 | (0 <<21)), TT_ATTR_RX)			/* .boot page */
	TT_2M_BLOCK( (0x40000000 | (1 <<21)), TT_ATTR_RX)			/* .text + .rodata page */
	TT_2M_BLOCK( (0x40000000 | (2 <<21)), TT_ATTR_RW)			/* .data and .bss page */
	TT_2M_BLOCK( (0x40000000 | (3 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (4 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (5 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (6 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (7 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (8 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (9 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (10<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (11<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (12<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (13<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (14<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (15<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (16<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (17<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (18<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (19<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (20<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (21<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (22<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (23<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (24<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (25<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (26<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */		
	TT_2M_BLOCK( (0x40000000 | (27<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (28<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (29<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (30<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (31<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	
.align 12
__initial_l2_kernel_table:
	TT_2M_BLOCK( (0x40000000 | (1 <<21)), TT_ATTR_RX)			/* .text + .rodata page */
	TT_2M_BLOCK( (0x40000000 | (2 <<21)), TT_ATTR_RW)			/* .data and .bss page */
	TT_2M_BLOCK( (0x40000000 | (3 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (4 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (5 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (6 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (7 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (8 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (9 <<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (10<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (11<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (12<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (13<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (14<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (15<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (16<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (17<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (18<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (19<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (20<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (21<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (22<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (23<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (24<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (25<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (26<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */		
	TT_2M_BLOCK( (0x40000000 | (27<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (28<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (29<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (30<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (31<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
	TT_2M_BLOCK( (0x40000000 | (32<<21)), TT_ATTR_RW)			/* rest of pages mapped RW */
.align 12
__initial_ttbr1_el1:
	TT_NEXT_TABLE(__initial_l2_kernel_table)




