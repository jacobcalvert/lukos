#include <managers/process-manager.h>
#include <managers/interrupt-manager.h>
#include <managers/vmm_arch.h>
#include <libraries/mem/memlib.h>
#include <managers/virtual-memory-manager.h>
#include <interrupt-controllers/interrupt-controller.h>
#include <exceptions/exceptions.h>
#include <string.h>
#include <mmu/mmu.h>

#define SPSR_FRAME_OFFSET	(64U)
#define ELR_FRAME_OFFSET	(65U)
#define X30_FRAME_OFFSET	(66U)
#define X0_FRAME_OFFSET		(96U)
#define X1_FRAME_OFFSET		(97U)

#define FRAME_SIZE_BYTES	(784U)

#define DEFAULT_SPSR		0x0

static void aarch64_intm_handler(size_t cpu, size_t irqno);
void aarch64_scheduling_stop(void);

/**
 * populate the interrupt stack
 * @param reg		the registration
 */
void intm_arch_stack_populate(process_interrupt_registration_t *reg)
{

	address_space_t *as = reg->prc->as;
	void *base = (void*)reg->stack;
	
	size_t frame_base_off_words = (reg->stack_size - ((size_t)FRAME_SIZE_BYTES))/sizeof(uint64_t);
	
	uint64_t *frame_base = &((uint64_t*)vmm_arch_v2p(as->arch_context, (void*)base))[frame_base_off_words];
	
	reg->stack_pointer = (void*)((size_t)base + (frame_base_off_words*sizeof(uint64_t))); /* VA frame base */
	
	memset(frame_base, 0, FRAME_SIZE_BYTES);
	frame_base[SPSR_FRAME_OFFSET] = (uint64_t)DEFAULT_SPSR;
	frame_base[ELR_FRAME_OFFSET] = (uint64_t)reg->entry;
}

/**
 * instruct the interrupt to be routed to the INTM for further processing
 * @param cpuno			the cpu number
 * @param irqno			the IRQ number
 */
void intm_arch_interrupt_attach(size_t cpuno, size_t irqno)
{
	aarch64_intc_int_pri_set(irqno, cpuno, 250);
	aarch64_intc_int_enable(irqno, cpuno, aarch64_intm_handler);
}



void aarch64_intm_handler(size_t cpuno, size_t irq)
{
	/* this works like a context switch */
	void *sp0 =NULL;
	void *ttbr0 = NULL;
	process_interrupt_registration_t *reg = NULL;
	aarch64_scheduling_stop(); /* stop scheduling on this core until we're done ! */
	
	reg = intm_interrupt_handle(cpuno, irq);
	if(reg != NULL)
	{
		thread_t *thr = pm_thread_current_get(cpuno);
		/* could be null if first thread ever */
		if(thr != NULL)
		{
			__asm__ __volatile__("msr SPSel, #0\r\nmov %0, sp\r\nmsr SPSel, #1" : "=r" (sp0) : : );
			thr->stack_pointer = sp0;
		
		}
		
		sp0 = reg->stack_pointer;
		__asm__ __volatile__("msr SPSel, #0\r\nmov sp, %0\r\nmsr SPSel, #1" : "=r" (sp0) : : );
		ttbr0 = (void*)((aarch64_vmm_context_t*)thr->parent->as->arch_context)->translation_table;
		__asm ("msr ttbr0_el1, %[ttbr0]\r\nisb" : : [ttbr0] "r" (ttbr0));
	
	}
	
}
