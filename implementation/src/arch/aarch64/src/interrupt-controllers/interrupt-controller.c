#include <interrupt-controllers/interrupt-controller.h>

static aarch64_intc_t* INTC = NULL;

aarch64_intc_t* gicv2_register();

void aarch64_intc_register()
{

	INTC = gicv2_register();
	if(INTC != NULL)return;

}

void aarch64_intc_init(void)
{
	if(INTC != NULL)
	{
		INTC->init(INTC->context);
	}
}

size_t aarch64_intc_current_int_get(void)
{
	if(INTC != NULL)
	{
		return INTC->current_get(INTC->context);
	}
	return (size_t)-1;
}

void aarch64_intc_int_complete(size_t intno)
{
	if(INTC != NULL)
	{
		return INTC->complete(INTC->context, intno);
	}	
}

int aarch64_intc_int_enable_by_properties(void *prop, size_t pri, size_t cpuno, aarch64_int_handler handler)
{
	if(INTC != NULL)
	{
		return INTC->enable_by_dtb(INTC->context, prop, pri, cpuno, handler);
	}
	return -1;
}


