#include <libraries/fdt/fdtlib.h>
#include <libraries/mem/memlib.h>
#include <mmu/mmu.h>
#include <exceptions/exceptions.h>
#include <string.h>

#include <stdint.h>

#define AARCH64_AUX_CPU_START_TYPE_PSCI			(1<<0)
#define AARCH64_AUX_CPU_START_TYPE_SPINTABLE	(1<<1)


static void aarch64_init_aux_cpus(void);
static void aarch64_aux_cpu_start(size_t cpuno, size_t type);
static int aarch64_find_aux_cpus_callback(char *path, void *arg);
static size_t CPU_COUNT = 1; /* we have at least one */
static void (*aux_cpu_jump_point)(size_t cpuno) = NULL;
static int cpu_online[16] = {0};


static void hvc4args(uint64_t a, uint64_t b, uint64_t c, uint64_t d);

void aarch64_init(void *dtb_addr)
{
	if(fdtlib_init(dtb_addr) != FDT_OK)
	{
		while(1);
	}
	aarch64_mmu_init();
	aarch64_exceptions_init();
	aarch64_init_aux_cpus();
	aarch64_aux_cpu_entry(0);
}


void aarch64_init_aux_cpus(void)
{
	fdtlib_find_by_prop("device_type", "cpu", aarch64_find_aux_cpus_callback, NULL);
}

int aarch64_find_aux_cpus_callback(char *path, void *arg)
{
	/*
	 * get reg number, skip if 0
	 * get enable method
	 * allocate stack
	 * call init routine for cpu
	 */ 
	uint32_t cpuno = fdtlib_conv_u32(fdtlib_get_prop(path, "reg"));
	size_t type = 0;
	if(cpuno == 0)
	{
		return 1;
	}
	
	char *enable_method = (char *)fdtlib_get_prop(path, "enable-method");
	if(strcmp(enable_method, "psci") == 0)
	{
		type = AARCH64_AUX_CPU_START_TYPE_PSCI;
	}
	else if(strcmp(enable_method, "spintable") == 0)
	{
		type = AARCH64_AUX_CPU_START_TYPE_SPINTABLE;
	}
	
	aarch64_aux_cpu_start((size_t)cpuno, type);
	return 1; /* keep finding */
}

void aarch64_aux_cpu_start(size_t cpuno, size_t type)
{
	++CPU_COUNT;
	switch(type)
	{
		case AARCH64_AUX_CPU_START_TYPE_PSCI:
		{
		
			extern char aarch64_psci_init;
			extern char aarch64_switch_as;
			extern void *KERNEL_HIGH_ADDR_MAP;
			extern void *KERNEL_LOW_ADDR_MAP;
			extern void *KERNEL_LOW_EXE_ADDR_MAP;
			struct psci_info
			{
				void *sp;
				void *ttbr0_rw;
				void *ttbr0_rx;
				void *ttbr1;
				uint64_t mair;
				uint64_t tcr;
				void (*next_vaddr)(void);
			};
			
			struct psci_info ctx;
			ctx.sp = (void*)((uint64_t)aarch64_mmu_stack_create(cpuno) + 0x4000); /* HACK: offset to stack end */
			ctx.ttbr0_rw = KERNEL_LOW_ADDR_MAP;
			ctx.ttbr0_rx = KERNEL_LOW_EXE_ADDR_MAP;
			ctx.ttbr1 = KERNEL_HIGH_ADDR_MAP;
			ctx.mair = MAIR_EL1_DEFAULT;
			ctx.tcr = TCR_EL1_DEFAULT;
			ctx.next_vaddr = &aarch64_switch_as;
			register uint64_t command = 0xC4000003;
			register uint64_t target = cpuno;
			register uint64_t entry = (uint64_t)aarch64_v2p(KERNEL_HIGH_ADDR_MAP, &aarch64_psci_init);
			register uint64_t context = (uint64_t)aarch64_v2p(KERNEL_HIGH_ADDR_MAP, &ctx);
			
			hvc4args(command, target, entry, context);
			
			while(cpu_online[cpuno] == 0)
			{
				/* wait */			
			}
			
			break;
		}
		case AARCH64_AUX_CPU_START_TYPE_SPINTABLE:
		{
			break;
		}
	
	}

}
void hvc4args(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
{
	__asm__ __volatile__("hvc #0");
}

void aarch64_aux_cpu_entry(size_t cpuno)
{
	cpu_online[cpuno] = 1;
	while(aux_cpu_jump_point == NULL)
	{
	
	}

}
