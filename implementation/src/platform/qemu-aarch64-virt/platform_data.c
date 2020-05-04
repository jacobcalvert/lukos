#include <interfaces/platform/platform_data.h>

#define ADDR(at)		(void*)at

static platform_restrict_range_t ranges[] = {
		/* 	name		physaddr	size */	
		{	
			.name = "GIC.GICD",
			.phys_addr = ADDR(0x8000000), 	
			.len = 0x10000
		},
		{	
			.name = "GIC.GICC", 
			.phys_addr = ADDR(0x8010000), 	
			.len = 0x10000
		},
		{	
			.name = "GIC.V2M",
			.phys_addr = ADDR(0x8020000),
			.len = 	0x1000
		},
		{
			.name = "RAM",
			.phys_addr = ADDR(0x40000000), 
			.len = 0x10000000
		}
		
	
	
	};

platform_data_t PLATFORM_DATA = {
	.name = 			"QEMU ARM Virt - Cortex-A53 SMP",
	.version =			"v0.0.1",
	.max_cpus =			4,
	.scheduling_freq =	10,
	.restricted_ranges = ranges

};
