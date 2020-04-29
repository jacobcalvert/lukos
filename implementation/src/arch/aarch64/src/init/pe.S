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
.global aarch64_psci_init
aarch64_psci_init:
	mov x4, x0
	ldr x5, [x4], #8					/* sp */
	ldr x6, [x4], #8					/* ttbr0_rw */
	ldr x7, [x4], #8					/* ttbr0_rx */
	ldr x8, [x4], #8					/* ttbr1 */
	ldr x9, [x4], #8					/* mair */
	ldr x10, [x4], #8						/* tcr */
	ldr x11, [x4]						/* vaddr of next jump */
	b aarch64_pe_init
.global aarch64_pe_init
aarch64_pe_init:
	mov x4, x0
	mrs x0, CurrentEL					/* read the current exception level */	
	and x0, x0, #EL3_BITS				/* and off the unused bits */
	cmp x0, #EL3_BITS					/* compare the result to the EL3 bits */
	bne __aarch64_pe_EL2_init			/* if x0 != EL3, means we're not in EL3, so go to EL2 setup */
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
	adr	x0, __aarch64_pe_EL2_init		/* address of jump -> x0 */
	msr	elr_el3, x0						/* put it into the link reg */
	isb									/* sync */
	eret								/* jump*/

__aarch64_pe_EL2_init:
	mrs x0, CurrentEL					/* read the current exception level */	
	and x0, x0, #EL2_BITS
	cmp x0, #EL2_BITS					/* compare the result to the EL3 bits */
	bne __aarch64_pe_EL1_init			/* if x0 != EL2, means we're not in EL2, so go to EL1 setup */
	mov x0, #HCR_EL2_DEFAULT			/* load up */
	msr hcr_el2, x0						/* set it */
	mov x0, #SPSR_EL2_DEFAULT			/* setup EL2 SPSR */
	msr spsr_el2, x0
	msr	cptr_el2, xzr					/* enable FPU */
	adr x0, __aarch64_pe_EL1_init		/* set the EL1 return address */
	msr elr_el2, x0		
	isb
	eret								/* jump */


__aarch64_pe_EL1_init:
	mov	x0, #CPACR_EL1_DEFAULT			/* don't trap SIMD, SVE instructions in EL1 */
	mov sp, x5
	msr	cpacr_el1, x0
	msr ttbr0_el1, x7					/* use ttbr0_rx for the switch, then jump to high code */
	msr ttbr1_el1, x8
	msr mair_el1, x9
	msr tcr_el1, x10
	ldr x0, =_default_el1_vtable
	msr vbar_el1, x0
	isb
	mov x0, #SCTLR_EL1_DEFAULT			/* setup control regs */
	orr x0, x0, #1
	msr sctlr_el1, x0
	isb
	
	br x11
	
.global aarch64_switch_as	
aarch64_switch_as:
	msr ttbr0_el1, x6
	isb
	mrs x0, mpidr_el1
	and x0, x0, #0x0F
	bl aarch64_aux_cpu_entry

