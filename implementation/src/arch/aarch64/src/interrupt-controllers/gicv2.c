#include <libraries/fdt/fdtlib.h>
#include <exceptions/exceptions.h>
#include <interrupt-controllers/interrupt-controller.h>
#include <mmu/mmu.h>
#include <stddef.h>
#include <string.h>

#define BIT(n)		(1<<(n))

#define GICD_CTLR_ENABLE	BIT(0)
#define GICD_CTLR_ENGRP0	BIT(0)
#define GICD_CTLR_ENGRP1	BIT(1)


#define GICD_TYPER_ITLINES_MASK 	(0x0000001F)
#define GICD_TYPER_CPUNUM_MASK		(BIT(5) | BIT(6) | BIT(7))
#define GICD_TYPER_CPUNUM_SHIFT		(5)


#define GICC_CTLR_ENGRP0		BIT(0)
#define GICC_CTLR_ENGRP1		BIT(1)

static int gicv2_enable(void *ctx, size_t intno, size_t cpuno, aarch64_int_handler handler);
static int gicv2_disable(void *ctx, size_t intno, size_t cpuno);
static int gicv2_pri_set(void *ctx, size_t intno, size_t cpuno, size_t pri);
static size_t gicv2_get(void*ctx);
static void gicv2_complete(void *ctx, size_t intno);
static void gicv2_percore_init(void* ctx);
static void gicv2_generate(void *ctx, size_t intno);
static void gicv2_ipi_generate(void *ctx, size_t cpu, size_t intno);
static int gicv2_enable_by_dtb(void *ctx, void *properties, size_t pri,  size_t cpuno, aarch64_int_handler handler);
typedef struct __attribute__((packed))
{
	uint32_t CTLR;
	uint32_t TYPER;
	uint32_t IIDR;
	uint32_t resvd1[29];
	uint32_t IGROUPR[32];
	uint32_t ISENABLER[32];
	uint32_t ICENABLER[32];
	uint32_t ISPENDR[32];
	uint32_t ICPENDR[32];
	uint32_t ISACTIVER[32];
	uint32_t ICACTIVER[32];
	uint32_t IPRIORITYR[255];
	uint32_t resvd2;
	uint32_t ITARGETSR[255];
	uint32_t resvd3;
	uint32_t ICFGR[64];
	uint32_t resvd4[64];
	uint32_t NSACR[64];
	uint32_t SGIR;
	uint32_t resvd5[4];
	uint32_t CPENDSGIR[4];
	uint32_t SPENDSGIR[4];
}GICD_t;


typedef struct __attribute__((packed))
{ 
	uint32_t CTLR;
	uint32_t PMR;
	uint32_t BPR;
	uint32_t IAR;
	uint32_t EOIR;
	uint32_t RPR;
	uint32_t HPPIR;
	uint32_t ABPR;
	uint32_t AIAR;
	uint32_t AEOIR;
	uint32_t AHPPIR;
}GICC_t;

typedef struct 
{
	volatile GICD_t *GICD;
	volatile GICC_t *GICC;

	uint32_t max_irqs;
	uint32_t max_cpus;
	uint32_t uuid;

}GIC_V2_t;

static GIC_V2_t GIC_V2;

static aarch64_intc_t GICV2_INTC_IMPL = {
	.context = (void*)&GIC_V2, 
	.enable = gicv2_enable,
	.disable = gicv2_disable,
	.priority_set = gicv2_pri_set,
	.current_get = gicv2_get, 
	.complete = gicv2_complete,
	.generate = gicv2_generate,
	.ipi_generate = gicv2_ipi_generate,
	.init = gicv2_percore_init,
	.enable_by_dtb = gicv2_enable_by_dtb
};

static int gicv2_find_match_callback(char *path, void *arg)
{	
	void *prop_addr = NULL;
	aarch64_intc_t *impl = (aarch64_intc_t*)arg;
	GIC_V2_t *gic = (GIC_V2_t*)impl->context;
	if((prop_addr = fdtlib_get_prop(path, "#size-cells")) == NULL)
	{
		prop_addr = fdtlib_get_prop("/", "#size-cells");
		
	}
	uint32_t sz_cells = fdtlib_conv_u32(prop_addr);
	if((prop_addr = fdtlib_get_prop(path, "#address-cells")) == NULL)
	{
		prop_addr = fdtlib_get_prop("/", "#address-cells");
		
	}
	uint32_t addr_cells = fdtlib_conv_u32(prop_addr);
	
	/* get regs */
	if((prop_addr = fdtlib_get_prop(path, "reg")) != NULL)
	{
	
		/* GICD */
		if(addr_cells == 2)
		{
			gic->GICD = (volatile GICD_t*)fdtlib_conv_u64(prop_addr);
		}
		else
		{
			gic->GICD = (volatile GICD_t*)(uint64_t)fdtlib_conv_u32(prop_addr);
		}
		prop_addr = (void*)((size_t)prop_addr + (size_t)addr_cells*sizeof(uint32_t));
		uint64_t size_gicd = 0;
		if(sz_cells == 2)
		{
			size_gicd = (uint64_t)fdtlib_conv_u64(prop_addr);
		}
		else
		{
			size_gicd = (uint64_t)fdtlib_conv_u32(prop_addr);
		}
		prop_addr = (void*)((size_t)prop_addr + (size_t)sz_cells*sizeof(uint32_t));
		
		/* GICC */
		if(addr_cells == 2)
		{
			gic->GICC = (volatile GICC_t*)fdtlib_conv_u64(prop_addr);
		}
		else
		{
			gic->GICC = (volatile GICC_t*)(uint64_t)fdtlib_conv_u32(prop_addr);
		}
		prop_addr = (void*)((size_t)prop_addr + (size_t)addr_cells*sizeof(uint32_t));
		uint64_t size_gicc = 0;
		if(sz_cells == 2)
		{
			size_gicc = (uint64_t)fdtlib_conv_u64(prop_addr);
		}
		else
		{
			size_gicc = (uint64_t)fdtlib_conv_u32(prop_addr);
		}
		
		if(aarch64_mmu_device_map(&KERNEL_HIGH_ADDR_MAP, (void*)gic->GICD, size_gicd, aarch64_mmu_allocate_kernel_table, NULL, (void**)&gic->GICD) != 0)
		{
		
		}
		
		if(aarch64_mmu_device_map(&KERNEL_HIGH_ADDR_MAP, (void*)gic->GICC, size_gicc, aarch64_mmu_allocate_kernel_table, NULL, (void**)&gic->GICC) != 0)
		{
		
		}

		
		gic->max_irqs = (32*((gic->GICD->TYPER & GICD_TYPER_ITLINES_MASK) +1));
		gic->max_cpus = (1 + ((gic->GICD->TYPER & GICD_TYPER_CPUNUM_MASK)>>GICD_TYPER_CPUNUM_SHIFT));
		
		
		
	}
	if((prop_addr = fdtlib_get_prop(path, "phandle")) != NULL)
	{
		gic->uuid = fdtlib_conv_u32(prop_addr);
	}
	
	
	
	
	return 0;
}

static void gicv2_init(aarch64_intc_t *impl)
{

	GIC_V2_t *GIC = (GIC_V2_t*)impl->context;
	
	GIC->GICD->CTLR &= ~GICD_CTLR_ENABLE; /* disable  the GIC */
	GIC->max_irqs = (32*((GIC->GICD->TYPER & GICD_TYPER_ITLINES_MASK) +1));
	GIC->max_cpus = (1 + ((GIC->GICD->TYPER & GICD_TYPER_CPUNUM_MASK)>>GICD_TYPER_CPUNUM_SHIFT));

	for(uint32_t reg = 0; reg < ((GIC->max_irqs/32) ); reg++)
	{
		GIC->GICD->IGROUPR[reg] = 0; /* set all irqs to group 0 */
		GIC->GICD->ICENABLER[reg] = 0xFFFFFFFF; /* set all irqs to disabled */
		GIC->GICD->ICPENDR[reg] = 0xFFFFFFFF; /* clear all irqs pending status */
		GIC->GICD->ICACTIVER[reg] = 0xFFFFFFFF; /* clear all irqs active status */
		
	} 

	for(uint32_t reg = 0; reg < ((GIC->max_irqs/4) ); reg++)
	{
		GIC->GICD->IPRIORITYR[reg] = 0xFFFFFFFF; /* set all priorities to lowest value */
		GIC->GICD->ITARGETSR[reg] = 0; /* set all targets to none */
	}
	
	
	GIC->GICD->CTLR |= GICD_CTLR_ENABLE; /* enable  the GIC */

}

void gicv2_percore_init(void* ctx)
{
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;
	GIC->GICC->CTLR &= ~(GICC_CTLR_ENGRP0| GICC_CTLR_ENGRP1); /* disable all interrupts */
	GIC->GICC->PMR = 0x000000FF; /* pri mask = lowest */
}

aarch64_intc_t* gicv2_register()
{

	aarch64_intc_t *impl = NULL;
	
	if(fdtlib_find_by_prop("compatible", "arm,cortex-a15-gic", gicv2_find_match_callback, (void*)&GICV2_INTC_IMPL))
	{
		impl = &GICV2_INTC_IMPL;
	}
	if(fdtlib_find_by_prop("compatible", "gicv2", gicv2_find_match_callback, (void*)&GICV2_INTC_IMPL))
	{
		impl = &GICV2_INTC_IMPL;
	}
	if(fdtlib_find_by_prop("compatible", "gic300", gicv2_find_match_callback, (void*)&GICV2_INTC_IMPL))
	{
		impl = &GICV2_INTC_IMPL;
	}
	if(impl)
	{
		gicv2_init(impl);
	}
	
	return impl;
}


int gicv2_enable(void *ctx, size_t intno, size_t cpuno, aarch64_int_handler handler)
{
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;
	if( (intno < GIC->max_irqs) && (cpuno < GIC->max_cpus))
	{
		
		uint32_t isenable_off = (intno/32);
		uint32_t isenable_bit = BIT(intno % 32);
		GIC->GICD->ISENABLER[isenable_off] |= isenable_bit;
		uint32_t itargets_off = (intno/4);
		uint32_t itargets_bit_shift = 8*(intno%4);
		uint32_t itargets_mask = BIT(cpuno) << itargets_bit_shift;
		
		aarch64_exceptions_handler_register(cpuno, intno, handler);
		GIC->GICD->ITARGETSR[itargets_off] |= itargets_mask;
	
		GIC->GICC->CTLR |= GICC_CTLR_ENGRP0;
		
		return AARCH64_INTC_OK;
	}
	return AARCH64_INTC_ERROR;
}

int gicv2_enable_by_dtb(void *ctx, void *properties, size_t pri, size_t cpuno, aarch64_int_handler handler)
{
	
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;
	/**
	 * we know the properties are in triplets of u32
	 * < spi_or_ppi   irq_no   flags >
	 * The 3rd cell is the flags, encoded as follows:
	 *		bits[3:0] trigger type and level flags.
	 *				1 = low-to-high edge triggered
	 *				2 = high-to-low edge triggered
	 *				4 = active high level-sensitive
	 *				8 = active low level-sensitive
	 *			bits[15:8] PPI interrupt cpu mask.  Each bit corresponds to each of
	 *			the 8 possible cpus attached to the GIC.  A bit set to '1' indicated
	 *			the interrupt is wired to that CPU.  Only valid for PPI interrupts.
	 */
	uint32_t *props = (uint32_t*)properties;
	size_t intno = (size_t) fdtlib_conv_u32((void*)&props[1]);
	size_t flags = (size_t) fdtlib_conv_u32((void*)&props[2]);
	if( fdtlib_conv_u32((void*)&props[0]) == 1) /* check if PPI or SPI */
	{
		intno += 16; /* PPI offset */
		if(  ( (1 << (cpuno+8)) & flags)  == 0)
		{
			return AARCH64_INTC_ERROR;
		}
	}
	else
	{
		intno += 32; /* SPI offset */
	}
	
	
	if(flags & 0x03)
	{
		/* edge */
	}
	else
	{
		/* level */
	}
	if( (intno < GIC->max_irqs) && (cpuno < GIC->max_cpus))
	{
		gicv2_pri_set(ctx, intno, cpuno, pri);
		uint32_t isenable_off = (intno/32);
		uint32_t isenable_bit = BIT(intno % 32);
		GIC->GICD->ISENABLER[isenable_off] |= isenable_bit;
		uint32_t itargets_off = (intno/4);
		uint32_t itargets_bit_shift = 8*(intno%4);
		uint32_t itargets_mask = BIT(cpuno) << itargets_bit_shift;
		
		aarch64_exceptions_handler_register(cpuno, intno, handler);
		GIC->GICD->ITARGETSR[itargets_off] |= itargets_mask;
		
	
		GIC->GICC->CTLR |= GICC_CTLR_ENGRP0;
		
		return AARCH64_INTC_OK;
	}
	
	return AARCH64_INTC_ERROR;
}

int gicv2_disable(void *ctx, size_t intno, size_t cpuno)
{
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;
	if(intno < GIC->max_irqs && cpuno < GIC->max_cpus)
	{

		uint32_t icenable_off = (intno/32);
		uint32_t icenable_bit = BIT(intno % 32);
		GIC->GICD->ICENABLER[icenable_off] |= icenable_bit;
		uint32_t itargets_off = (intno/4);
		uint32_t itargets_bit_shift = 8*(intno%4);
		uint32_t itargets_mask = BIT(cpuno) << itargets_bit_shift;

		GIC->GICD->ITARGETSR[itargets_off] &= ~itargets_mask;
		aarch64_exceptions_handler_register(cpuno, intno, NULL);

		return AARCH64_INTC_OK;
	}
	return AARCH64_INTC_ERROR;
}

int gicv2_pri_set(void *ctx, size_t intno, size_t cpuno, size_t pri)
{
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;
	
	if(intno < GIC->max_irqs)
	{
		uint32_t ipri_off = (intno/4);
		uint32_t ipri_bit_shift = 8*(intno % 4);
		uint32_t ipri_mask = (uint32_t)pri << ipri_bit_shift;
		GIC->GICD->IPRIORITYR[ipri_off] &= ~( (uint32_t)(0xFF<<ipri_bit_shift));
		GIC->GICD->IPRIORITYR[ipri_off] |= ipri_mask;
		return AARCH64_INTC_OK;
	}
	return AARCH64_INTC_ERROR;
}

void gicv2_generate(void *ctx, size_t intno)
{
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;
	
	if(intno < GIC->max_irqs)
	{
		uint32_t ispend_off = (intno/32);
		uint32_t ispend_bit = BIT(intno % 32);
		GIC->GICD->ISACTIVER[ispend_off] |= ispend_bit;
	}

}
void gicv2_ipi_generate(void *ctx, size_t cpu, size_t intno)
{
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;
	if(intno < 16 &&  cpu < GIC->max_cpus)
	{
		uint32_t target = ((1<<cpu)<<16);
		uint32_t sgi = intno;
		GIC->GICD->SGIR = (target | sgi);
	}
}

size_t gicv2_get(void*ctx)
{
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;
	size_t irq = GIC->GICC->IAR;	
	if(irq > GIC->max_irqs)
	{
		irq = (size_t)-1;
	}
	return irq;
}

void gicv2_complete(void *ctx, size_t intno)
{
	GIC_V2_t *GIC = (GIC_V2_t*)ctx;	
	if(intno < GIC->max_irqs)
	{
		GIC->GICC->EOIR = intno;
	}
}
