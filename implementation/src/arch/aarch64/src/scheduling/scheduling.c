#include <libraries/fdt/fdtlib.h>
#include <libraries/mem/memlib.h>
#include <libraries/elf/elflib.h>
#include <mmu/mmu.h>
#include <exceptions/exceptions.h>
#include <interrupt-controllers/interrupt-controller.h>
#include <interfaces/platform/platform_data.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>
#include <managers/vmm_arch.h>
#include <string.h>
#include <interfaces/os/atomic.h>


#include <stdint.h>

static uint32_t CPU_WAIT_LOCK = 1;

extern platform_data_t PLATFORM_DATA;
static int aarch64_find_core_timers_callback(char *path, void *arg);
static void aarch64_core_timer_init(size_t cpuno);
void aarch64_scheduling_interrupt(size_t cpuno, size_t intno);

void aarch64_scheduling_init(size_t cpuno)
{
	/* setup idle process */
	char name[6] = "app-n\0";
	char name2[6] = "idle0\0";
	extern char __app;
	extern char __idle;
	if(cpuno == 0)
	{
		address_space_t *app_as = vmm_address_space_create();
		void *entry = NULL;
		elflib_binary_load((void*)&__app, app_as, &entry);
		process_t *app = pm_process_create("app", app_as, PM_SCHEDULER_PRIORITY, 255);
		pm_thread_create(name, app, entry, (void*)cpuno, 0x1000, 255);
		pm_process_schedule(app);
		
		address_space_t *idle_as = vmm_address_space_create();
		elflib_binary_load((void*)&__idle, idle_as, &entry);
		process_t *idle = pm_process_create("idle", idle_as, PM_SCHEDULER_PRIORITY, (size_t)-2);
		for(size_t i = 0; i < PLATFORM_DATA.max_cpus; i++)
		{
			name2[4] = '0' + (char)i;
			pm_thread_affinity_set(pm_thread_create(name2, idle, entry, (void*)i, 0x1000, (size_t)-2), PM_THREAD_AFF_CORE(i));
		}
		pm_process_schedule(idle);
		
		CPU_WAIT_LOCK = 0;
	}
	
	/* */
	while(CPU_WAIT_LOCK);
	void *sp0 = memlib_malloc(1024);
	sp0 = (void*)((size_t)sp0 + 1024);
	__asm__ __volatile__("msr SPSel, #0\r\nmov sp, %0\r\nmsr SPSel, #1" : "=r" (sp0) : : );
	aarch64_core_timer_init(cpuno);
	while(1);
	
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
		
		aarch64_intc_int_enable_by_properties(&props[1*proplen], 128, *cpuno, aarch64_scheduling_interrupt);
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

void aarch64_scheduling_stop(void)
{
	__asm__ __volatile("mov x0, #0");
	__asm__ __volatile("msr cntp_ctl_el0, x0");
}

void aarch64_scheduling_interrupt(size_t cpuno, size_t intno)
{
	uint64_t freq = 0;
	uint64_t comp = 0;
	uint64_t current = 0;
	void *sp0 =NULL;
	void *ttbr0 = NULL;
	__asm__ __volatile__("mrs %0, cntfrq_el0" : "=r" (freq) : : );
	comp = freq/PLATFORM_DATA.scheduling_freq;
	__asm__ __volatile__("mrs %0, cntpct_el0" : "=r" (current) : : );
	comp+=current;
	__asm__ __volatile__("msr cntp_cval_el0, %0" : "=r" (comp) : : );
	
	thread_t *thr = pm_thread_current_get(cpuno);
	/* could be null if first thread ever */
	if(thr != NULL)
	{
		__asm__ __volatile__("msr SPSel, #0\r\nmov %0, sp\r\nmsr SPSel, #1" : "=r" (sp0) : : );
		thr->stack_pointer = sp0;
	
	}
	
	
	
	thr = pm_thread_next_get(cpuno);
	sp0 = thr->stack_pointer;
	__asm__ __volatile__("msr SPSel, #0\r\nmov sp, %0\r\nmsr SPSel, #1" : "=r" (sp0) : : );
	ttbr0 = (void*)((aarch64_vmm_context_t*)thr->parent->as->arch_context)->translation_table;
	 __asm ("msr ttbr0_el1, %[ttbr0]\r\nisb" : : [ttbr0] "r" (ttbr0));

	
	
	
	
	
	
}
