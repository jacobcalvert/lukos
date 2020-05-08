#include <managers/interrupt-manager.h>
#include <interfaces/platform/platform_data.h>
#include <libraries/mem/memlib.h>
#include <interfaces/os/atomic.h>
#include <managers/process-manager.h>

#include <string.h>


static process_interrupt_registration_t **INTERRUPT_REGISTRATION;
static size_t INTERRUPT_MAX_CPUS = 0;
static size_t INTERRUPT_MAX_IRQS = 0;
extern platform_data_t PLATFORM_DATA;



void intm_init(size_t max_cpus, size_t max_irqs)
{
	INTERRUPT_REGISTRATION = (process_interrupt_registration_t**)memlib_malloc(sizeof(process_interrupt_registration_t*)*max_cpus);
	for(size_t cpu = 0; cpu < max_cpus; cpu++)
	{
		INTERRUPT_REGISTRATION[cpu] = (process_interrupt_registration_t*)memlib_malloc(sizeof(process_interrupt_registration_t)*max_irqs);
		for(size_t irq = 0; irq < max_irqs; irq++)
		{
			INTERRUPT_REGISTRATION[cpu][irq].prc = NULL;
			INTERRUPT_REGISTRATION[cpu][irq].stack_pointer = NULL;
			INTERRUPT_REGISTRATION[cpu][irq].stack = NULL;
		}
		
	}
	INTERRUPT_MAX_CPUS = max_cpus;
	INTERRUPT_MAX_IRQS = max_irqs;
}

process_interrupt_registration_t* intm_interrupt_handle(size_t cpuno, size_t irqno)
{
	if(cpuno >= INTERRUPT_MAX_CPUS || irqno >= INTERRUPT_MAX_IRQS)
	{
		return NULL;
	}
	
	if(INTERRUPT_REGISTRATION[cpuno][irqno].prc == NULL)
	{
		return NULL;
	}
	
	return &INTERRUPT_REGISTRATION[cpuno][irqno];
	
}

int intm_interrupt_attach(process_t *prc, size_t cpuno, size_t irqno, size_t flags, void *entry)
{
	size_t index = 0;
	while(index < PLATFORM_DATA.num_restricted_interrupts)
	{
		
		if( (INTERRUPT_RESTRICT_CPU(cpuno) & PLATFORM_DATA.restricted_interrupts[index].cpus) && (PLATFORM_DATA.restricted_interrupts[index].irqno == irqno))
		{
			return -1;
		}
		
		
		index++;
	}
	
	if(cpuno >= INTERRUPT_MAX_CPUS || irqno >= INTERRUPT_MAX_IRQS)
	{
		return -2;
	}
	
	if(INTERRUPT_REGISTRATION[cpuno][irqno].prc != NULL)
	{
		return -3;
	}
	
	/* we can attach */
	/* create an interrupt stack, populate it */
	
	INTERRUPT_REGISTRATION[cpuno][irqno].prc = prc;
	INTERRUPT_REGISTRATION[cpuno][irqno].stack_size = 0x4000;
	INTERRUPT_REGISTRATION[cpuno][irqno].entry = entry;
	vmm_address_space_region_create_auto(prc->as, 0x4000, AS_REGION_RW, &INTERRUPT_REGISTRATION[cpuno][irqno].stack);
	
	intm_arch_stack_populate(&INTERRUPT_REGISTRATION[cpuno][irqno]); 
	intm_arch_interrupt_attach(cpuno, irqno);

	return 0;
}


