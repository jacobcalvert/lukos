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
	
extern char __io_server;
extern char __idle;
	
static elf_entry_t myelves[] = {
	{
		.name = "io-server",
		.addr = &__io_server,
		.priority = 50,
		.stack_size = 0x8000

	},
	{
		.name = "idle",
		.addr = &__idle,
		.priority = (size_t)-2,
		.stack_size = 0x1000

	}
};

#define NO_ELVES 	sizeof(myelves)/sizeof(myelves[0])
platform_data_t PLATFORM_DATA = {
	.name = 			"QEMU ARM Virt - Cortex-A53 SMP",
	.version =			"v0.0.1",
	.max_cpus =			4,
	.scheduling_freq =	10,
	.restricted_ranges = ranges,
	.elves = myelves,
	.num_elves = NO_ELVES

};
