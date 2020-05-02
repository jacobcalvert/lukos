#include <managers/process-manager.h>
#include <libraries/mem/memlib.h>
#include <managers/virtual-memory-manager.h>
#include <string.h>

#define SPSR_FRAME_OFFSET	(64U)
#define ELR_FRAME_OFFSET	(65U)
#define X30_FRAME_OFFSET	(66U)
#define X0_FRAME_OFFSET		(96U)
#define X1_FRAME_OFFSET		(97U)

#define FRAME_SIZE_BYTES	(784U)

#define DEFAULT_SPSR		0x4




void pm_arch_thread_stack_populate(process_t *prc, thread_t *thr)
{
	address_space_t *as = prc->as;
	void *base = (void*)thr->stack_base;
	uint64_t spsr = DEFAULT_SPSR;
	
	size_t frame_base_off_words = (thr->stack_size - ((size_t)FRAME_SIZE_BYTES))/sizeof(uint64_t);
	
	void*frame_base = (void*)((size_t)base + frame_base_off_words);
	
	thr->stack_pointer = frame_base;
	
	/*memset(frame_base, 0, FRAME_SIZE_BYTES);*/
	vmm_address_space_copy_in(as, &thr->argc, (void*)((size_t)frame_base + X0_FRAME_OFFSET), sizeof(uint64_t));
	vmm_address_space_copy_in(as, &thr->argv, (void*)((size_t)frame_base + X1_FRAME_OFFSET), sizeof(uint64_t));
	vmm_address_space_copy_in(as, &spsr, (void*)((size_t)frame_base + SPSR_FRAME_OFFSET), sizeof(uint64_t));
	vmm_address_space_copy_in(as, &thr->entry, (void*)((size_t)frame_base + ELR_FRAME_OFFSET), sizeof(uint64_t));
	vmm_address_space_copy_in(as, &thr->entry, (void*)((size_t)frame_base + X30_FRAME_OFFSET), sizeof(uint64_t));
	/*
	frame_base[X0_FRAME_OFFSET] = (uint64_t)argc;
	frame_base[X1_FRAME_OFFSET] = (uint64_t)argv;
	frame_base[SPSR_FRAME_OFFSET] = DEFAULT_SPSR;
	frame_base[ELR_FRAME_OFFSET] = (uint64_t) entry;
	frame_base[X30_FRAME_OFFSET] = (uint64_t) on_exit;
	*/

}
