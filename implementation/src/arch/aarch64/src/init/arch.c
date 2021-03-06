#include <libraries/fdt/fdtlib.h>
#include <libraries/mem/memlib.h>
#include <libraries/elf/elflib.h>
#include <mmu/mmu.h>
#include <exceptions/exceptions.h>
#include <interrupt-controllers/interrupt-controller.h>
#include <interfaces/platform/platform_data.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>
#include <managers/interrupt-manager.h>
#include <string.h>

#include <stdint.h>

#define AARCH64_AUX_CPU_START_TYPE_PSCI			(1<<0)
#define AARCH64_AUX_CPU_START_TYPE_SPINTABLE	(1<<1)
void aarch64_aux_cpu_entry(size_t cpuno);

static void aarch64_init_aux_cpus(void);
static void aarch64_aux_cpu_start(size_t cpuno, size_t type);
static int aarch64_find_aux_cpus_callback(char *path, void *arg);

static void hvc4args(uint64_t a, uint64_t b, uint64_t c, uint64_t d);
extern platform_data_t PLATFORM_DATA;

void aarch64_scheduling_init(size_t cpuno);


static size_t CPU_COUNT = 1; /* we have at least one */
static int cpu_online[16] = {0};

void aarch64_init(void *dtb_addr)
{
	if(fdtlib_init(dtb_addr) != FDT_OK)
	{
		while(1);
	}
	aarch64_mmu_init();
	aarch64_exceptions_init();
	aarch64_intc_register();
	aarch64_init_aux_cpus();
	vmm_init();
	pm_init(PLATFORM_DATA.max_cpus);
	intm_init(PLATFORM_DATA.max_cpus, 255);
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
			
			struct psci_info
			{
				void *sp;
				uint64_t *ttbr0_rw;
				uint64_t *ttbr0_rx;
				uint64_t *ttbr1;
				uint64_t mair;
				uint64_t tcr;
				void (*next_vaddr)(void);
			};
			
			struct psci_info ctx;
			ctx.sp = (void*)((uint64_t)aarch64_mmu_stack_create(cpuno) + 0x4000); /* HACK: offset to stack end */
			ctx.ttbr0_rw = KERNEL_LOW_ADDR_MAP.translation_table;
			ctx.ttbr0_rx = KERNEL_LOW_EXE_ADDR_MAP.translation_table;
			ctx.ttbr1 = KERNEL_HIGH_ADDR_MAP.translation_table;
			ctx.mair = MAIR_EL1_DEFAULT;
			ctx.tcr = TCR_EL1_DEFAULT;
			ctx.next_vaddr = (void*)&aarch64_switch_as;
			register uint64_t command = 0xC4000003;
			register uint64_t target = cpuno;
			register uint64_t entry = (uint64_t)aarch64_mmu_v2p(&KERNEL_HIGH_ADDR_MAP, &aarch64_psci_init);
			register uint64_t context = (uint64_t)aarch64_mmu_v2p(&KERNEL_HIGH_ADDR_MAP, &ctx);
			
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
	aarch64_intc_init();
	cpu_online[cpuno] = 1;
	aarch64_scheduling_init(cpuno);

}

