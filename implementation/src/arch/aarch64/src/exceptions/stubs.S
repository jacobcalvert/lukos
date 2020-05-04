/**
 *
 */

#define ESR_EC_MASK			(0x00000000FC000000)
#define ESR_EC_SHIFT_BITS	(26)

exception_halt:
	b exception_halt

/**
 * vector table entry macro
 *
 * simply branches to a handler, makes a clean table
 *
 * useage:
 * vtable_entry <handler_label>
 *
 */
 

.macro	vtable_entry	label
.align	7
	b	label
.endm

/**
 * a handler for unhandled vectors
 *
 * usage: vtable_error_handler	label_name idx
 */
.macro vtable_error_handler		label, index
.balign (1<<7)
\label:
	/* store off context */
	stp	x0, x1, [sp, #-256]!
	stp	x2, x3, [sp, #16]
	stp	x4, x5, [sp, #32]
	stp	x6, x7, [sp, #48]
	stp	x8, x9, [sp, #64]
	stp	x10, x11, [sp, #80]
	stp	x12, x13, [sp, #96]
	stp	x14, x15, [sp, #112]
	stp	x16, x17, [sp, #128]
	stp	x18, x19, [sp, #144]
	stp	x20, x21, [sp, #160]
	stp	x22, x23, [sp, #176]
	stp	x24, x25, [sp, #192]
	stp	x26, x27, [sp, #208]
	stp	x28, x29, [sp, #224]
	str	x30, [sp, #240]
	mrs x0, elr_el1
	mov x0, sp
	mov x1, #\index
	bl exception_halt
	/* unroll the context */
	ldp	x2, x3, [sp, #16]
	ldp	x4, x5, [sp, #32]
	ldp	x6, x7, [sp, #48]
	ldp	x8, x9, [sp, #64]
	ldp	x10, x11, [sp, #80]
	ldp	x12, x13, [sp, #96]
	ldp	x14, x15, [sp, #112]
	ldp	x16, x17, [sp, #128]
	ldp	x18, x19, [sp, #144]
	/* 20 and up are not my responsibility */
	ldr	x30, [sp, #240]
	ldp	x0, x1, [sp], #256
	/* go back */
	eret
.endm

/**
 * a handler for irq vectors
 *
 * usage: vtable_irq_handler	label_name, idx, elx
 */
.macro vtable_irq_handler	label, index, elx
.balign (1<<7)
\label:
	b irq_stub
.endm


/**
 * a handler for sync vectors
 *
 * usage: vtable_sync_handler	label_name, idx, elx
 */
.macro vtable_sync_handler	label, index, elx
.balign (1<<7)
\label:
	stp x0, x1, [sp, #-16]!
	mrs x0, esr_\elx 			/* read the ESR */
	mov x1, #ESR_EC_MASK		
	and x0, x1, x0				/* get only the EC bits */
	lsr x0, x0, #ESR_EC_SHIFT_BITS
	cmp x0, #0x15				/* check if SVC */
	ldp x0, x1, [sp], #16		/* replace x0,x1 */
	beq svc_stub
	b bad_sync_stub
.endm


bad_sync_stub:
	b exception_halt


svc_stub:
	msr SPsel, #0
	isb
	stp x0, x1, [sp, #-16]!
	stp x2, x3, [sp, #-16]!
	stp x4, x5, [sp, #-16]!
	stp x6, x7, [sp, #-16]!
	stp x8, x9, [sp, #-16]!
	stp x10, x11, [sp, #-16]!
	stp x12, x13, [sp, #-16]!
	stp x14, x15, [sp, #-16]!
	stp x16, x17, [sp, #-16]!
	stp x18, x19, [sp, #-16]!
	stp x20, x21, [sp, #-16]!
	stp x22, x23, [sp, #-16]!
	stp x24, x25, [sp, #-16]!
	stp x26, x27, [sp, #-16]!
	stp x28, x29, [sp, #-16]!
	stp x30, xzr, [sp, #-16]!
	mrs x0, spsr_el1
	mrs x1, elr_el1
	stp x0, x1, [sp, #-16]! 
	stp q0, q1, [sp, #-32]!					
	stp q2, q3, [sp, #-32]!					
	stp q4, q5, [sp, #-32]!					
	stp q6, q7, [sp, #-32]!					
	stp q8, q9, [sp, #-32]!					
	stp q10, q11, [sp, #-32]!					
	stp q12, q13, [sp, #-32]!					
	stp q14, q15, [sp, #-32]!					
	stp q16, q17, [sp, #-32]!					
	stp q18, q19, [sp, #-32]!					
	stp q20, q21, [sp, #-32]!					
	stp q22, q23, [sp, #-32]!					
	stp q24, q25, [sp, #-32]!					
	stp q26, q27, [sp, #-32]!					
	stp q28, q29, [sp, #-32]!					
	stp q30, q31, [sp, #-32]!
	mov x1, sp /* get SP for this thread */
	/* we have stored off context, select SPx and handle SVC */
	msr SPsel, #1
	ldr x0, =KERNEL_LOW_ADDR_MAP
	ldr x0, [x0]
	msr ttbr0_el1, x0
	isb
	mrs x0, mpidr_el1 
	and x0, x0, #0x0FF
	bl aarch64_svc_handle
	msr SPsel, #0
	isb
	ldp q30, q31, [sp], #32					
	ldp q28, q29, [sp], #32					
	ldp q26, q27, [sp], #32					
	ldp q24, q25, [sp], #32					
	ldp q22, q23, [sp], #32					
	ldp q20, q21, [sp], #32					
	ldp q18, q19, [sp], #32					
	ldp q16, q17, [sp], #32					
	ldp q14, q15, [sp], #32					
	ldp q12, q13, [sp], #32					
	ldp q10, q11, [sp], #32					
	ldp q8, q9, [sp], #32						
	ldp q6, q7, [sp], #32						
	ldp q4, q5, [sp], #32						
	ldp q2, q3, [sp], #32						
	ldp q0, q1, [sp], #32						
	ldp x0, x1, [sp], #16						
	msr spsr_el1, x0							
	msr elr_el1, x1							
	ldp x30, xzr, [sp], #16					
	ldp x28, x29, [sp], #16					
	ldp x26, x27, [sp], #16					
	ldp x24, x25, [sp], #16					
	ldp x22, x23, [sp], #16					
	ldp x20, x21, [sp], #16					
	ldp x18, x19, [sp], #16					
	ldp x16, x17, [sp], #16					
	ldp x14, x15, [sp], #16					
	ldp x12, x13, [sp], #16					
	ldp x10, x11, [sp], #16					
	ldp x8, x9, [sp], #16						
	ldp x6, x7, [sp], #16						
	ldp x4, x5, [sp], #16						
	ldp x2, x3, [sp], #16						
	ldp x0, x1, [sp], #16			
	eret
	
	

irq_stub:
	msr SPsel, #0
	isb
	stp x0, x1, [sp, #-16]!
	stp x2, x3, [sp, #-16]!
	stp x4, x5, [sp, #-16]!
	stp x6, x7, [sp, #-16]!
	stp x8, x9, [sp, #-16]!
	stp x10, x11, [sp, #-16]!
	stp x12, x13, [sp, #-16]!
	stp x14, x15, [sp, #-16]!
	stp x16, x17, [sp, #-16]!
	stp x18, x19, [sp, #-16]!
	stp x20, x21, [sp, #-16]!
	stp x22, x23, [sp, #-16]!
	stp x24, x25, [sp, #-16]!
	stp x26, x27, [sp, #-16]!
	stp x28, x29, [sp, #-16]!
	stp x30, xzr, [sp, #-16]!
	mrs x0, spsr_el1
	mrs x1, elr_el1
	stp x0, x1, [sp, #-16]! 
	stp q0, q1, [sp, #-32]!					
	stp q2, q3, [sp, #-32]!					
	stp q4, q5, [sp, #-32]!					
	stp q6, q7, [sp, #-32]!					
	stp q8, q9, [sp, #-32]!					
	stp q10, q11, [sp, #-32]!					
	stp q12, q13, [sp, #-32]!					
	stp q14, q15, [sp, #-32]!					
	stp q16, q17, [sp, #-32]!					
	stp q18, q19, [sp, #-32]!					
	stp q20, q21, [sp, #-32]!					
	stp q22, q23, [sp, #-32]!					
	stp q24, q25, [sp, #-32]!					
	stp q26, q27, [sp, #-32]!					
	stp q28, q29, [sp, #-32]!					
	stp q30, q31, [sp, #-32]!
	/* we have stored off context, select SPx and handle IRQ */
	msr SPsel, #1
	ldr x0, =KERNEL_LOW_ADDR_MAP
	ldr x0, [x0]
	msr ttbr0_el1, x0
	isb
	mrs x0, mpidr_el1 
	and x0, x0, #0x0FF
	bl aarch64_exceptions_handle
	msr SPsel, #0
	isb
	ldp q30, q31, [sp], #32					
	ldp q28, q29, [sp], #32					
	ldp q26, q27, [sp], #32					
	ldp q24, q25, [sp], #32					
	ldp q22, q23, [sp], #32					
	ldp q20, q21, [sp], #32					
	ldp q18, q19, [sp], #32					
	ldp q16, q17, [sp], #32					
	ldp q14, q15, [sp], #32					
	ldp q12, q13, [sp], #32					
	ldp q10, q11, [sp], #32					
	ldp q8, q9, [sp], #32						
	ldp q6, q7, [sp], #32						
	ldp q4, q5, [sp], #32						
	ldp q2, q3, [sp], #32						
	ldp q0, q1, [sp], #32						
	ldp x0, x1, [sp], #16						
	msr spsr_el1, x0							
	msr elr_el1, x1							
	ldp x30, xzr, [sp], #16					
	ldp x28, x29, [sp], #16					
	ldp x26, x27, [sp], #16					
	ldp x24, x25, [sp], #16					
	ldp x22, x23, [sp], #16					
	ldp x20, x21, [sp], #16					
	ldp x18, x19, [sp], #16					
	ldp x16, x17, [sp], #16					
	ldp x14, x15, [sp], #16					
	ldp x12, x13, [sp], #16					
	ldp x10, x11, [sp], #16					
	ldp x8, x9, [sp], #16						
	ldp x6, x7, [sp], #16						
	ldp x4, x5, [sp], #16						
	ldp x2, x3, [sp], #16						
	ldp x0, x1, [sp], #16			
	eret


.global _default_el1_vtable
/* el1 vtable */
.balign (1<<11)
_default_el1_vtable:
	vtable_sync_handler 	sync_el1_sp0, 		 0, 	el1
	vtable_irq_handler 		irq_el1_sp0, 		 1, 	el1
	vtable_error_handler 	fiq_el1_sp0, 	 	 2		
	vtable_error_handler 	serror_el1_sp0, 	 3
	vtable_sync_handler 	sync_el1_spx, 	 	 4, 	el1
	vtable_irq_handler 		irq_el1_spx, 		 5, 	el1
	vtable_error_handler 	fiq_el1_spx,	 	 6
	vtable_error_handler 	serror_el1_spx, 	 7
	vtable_sync_handler 	sync_el0_a64, 		 8, 	el1
	vtable_irq_handler 		irq_el0_a64, 		 9, 	el1
	vtable_error_handler 	fiq_el0_a64, 		10
	vtable_error_handler 	serror_el0_a64, 	11
	vtable_error_handler 	sync_el0_a32, 		12
	vtable_error_handler 	irq_el0_a32, 		13
	vtable_error_handler 	fiq_el0_a32, 		14
	vtable_error_handler 	serror_el0_a32, 	15




	
	
	

