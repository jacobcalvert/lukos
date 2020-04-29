#include <libraries/fdt/fdtlib.h>
#include <libraries/mem/memlib.h>
#include <mmu/mmu.h>
#include <exceptions/exceptions.h>
#include <interrupt-controllers/interrupt-controller.h>
#include <interfaces/platform/platform_data.h>
#include <string.h>

#include <stdint.h>

#define AARCH64_AUX_CPU_START_TYPE_PSCI			(1<<0)
#define AARCH64_AUX_CPU_START_TYPE_SPINTABLE	(1<<1)
void aarch64_aux_cpu_entry(size_t cpuno);

static void aarch64_init_aux_cpus(void);
static void aarch64_aux_cpu_start(size_t cpuno, size_t type);
static int aarch64_find_aux_cpus_callback(char *path, void *arg);
static int aarch64_find_core_timers_callback(char *path, void *arg);
static void aarch64_core_timer_init(size_t cpuno);

static void hvc4args(uint64_t a, uint64_t b, uint64_t c, uint64_t d);


static size_t CPU_COUNT = 1; /* we have at least one */
static void (*aux_cpu_jump_point)(size_t cpuno) = NULL;
static int cpu_online[16] = {0};

extern platform_data_t PLATFORM_DATA;

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
			ctx.next_vaddr = (void*)&aarch64_switch_as;
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
	aarch64_intc_init();
	/*****
	
	HACKHACKHACK
	
	*****/
	
	void *sp0 = memlib_malloc(0x1000);
	sp0 = (void*)((size_t)sp0 & 0xFFFFFFFFFFFFFF00);
	__asm__ __volatile__("msr SPSel, #0\r\nmov sp, %0\r\nmsr SPSel, #1" : "=r" (sp0) : : );
	/*****
	
	HACKHACKHACK
	
	*****/
	
	cpu_online[cpuno] = 1;
	aarch64_core_timer_init(cpuno);
	while(aux_cpu_jump_point == NULL)
	{
	
		uint64_t current;
		uint64_t comp;
		uint64_t enable;
		__asm__ __volatile__("mrs %0, cntpct_el0" : "=r" (current) : : );
		__asm__ __volatile__("mrs %0, cntp_cval_el0" : "=r" (comp) : : );
		__asm__ __volatile__("mrs %0, cntp_ctl_el0" : "=r" (enable) : : );
	}

}
static void temp(size_t cpuno, size_t intno)
{
	uint64_t freq = 0;
	uint64_t comp = 0;
	uint64_t current = 0;
	__asm__ __volatile__("mrs %0, cntfrq_el0" : "=r" (freq) : : );
	comp = freq/PLATFORM_DATA.scheduling_freq;
	__asm__ __volatile__("mrs %0, cntpct_el0" : "=r" (current) : : );
	comp+=current;
	__asm__ __volatile__("msr cntp_cval_el0, %0" : "=r" (comp) : : );
}
int aarch64_find_core_timers_callback(char *path, void *arg)
{
	void *intlist = fdtlib_get_prop(path, "interrupts");
	size_t *cpuno = (size_t*)arg;
	if(intlist)
	{
		/*
		 * the interrupts are represented in an array of 4 u32s, but the array lengths can be 1 or more
		 * each array represents the interrupt config for:
		 * secure mode
		 * non-secure mode (we want to use)
		 * virtual
		 * hypervisor
		 * so we want to use offset 1
		 */
		uint64_t freq = 0;
		uint64_t comp = 0;
		uint64_t current = 0;
		size_t proplen = fdtlib_get_prop_len(path, "interrupts");
		proplen/=16; /* div4 for convert to u32 div 4 again to breakout the number of u32s in each array */
		uint32_t *props = (uint32_t*) intlist;
		
		aarch64_intc_int_enable_by_properties(&props[1*proplen], 128, *cpuno, temp);
		__asm__ __volatile__("mrs %0, cntfrq_el0" : "=r" (freq) : : );
		comp = freq-1;
		__asm__ __volatile__("mrs %0, cntpct_el0" : "=r" (current) : : );
		comp+=current;
		__asm__ __volatile__("msr cntp_cval_el0, %0" : "=r" (comp) : : );
		__asm__ __volatile("mov x0, #1");
		__asm__ __volatile("msr cntp_ctl_el0, x0");
		__asm__ __volatile("msr daifclr, #7");
		return 0;
	}
	return 1;
}
void aarch64_core_timer_init(size_t cpuno)
{	
		fdtlib_find_by_prop("compatible", "arm,armv8-timer", aarch64_find_core_timers_callback, &cpuno );
		
}
