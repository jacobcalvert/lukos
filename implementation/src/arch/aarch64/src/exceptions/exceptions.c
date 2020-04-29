#include <exceptions/exceptions.h>
#include <interrupt-controllers/interrupt-controller.h>

#define MAX_CPUS		8
#define MAX_INTS		256



static aarch64_int_handler EXCEPTION_HANDLER_TABLE[MAX_CPUS][MAX_INTS];

void aarch64_exceptions_init(void)
{
	extern char _default_el1_vtable;
	void *vbar_el1 = &_default_el1_vtable;
	 __asm ("msr vbar_el1, %[vbar_el1]" : : [vbar_el1] "r" (vbar_el1));
}


void aarch64_exceptions_handler_register(size_t cpuno, size_t intno, aarch64_int_handler handler)
{
	if(cpuno < MAX_CPUS  && intno < MAX_INTS)
	{
		EXCEPTION_HANDLER_TABLE[cpuno][intno] = handler;
	}
}

void aarch64_exceptions_handle(size_t cpuno)
{
	size_t intno = aarch64_intc_current_int_get();//
	if(cpuno < MAX_CPUS  && intno < MAX_INTS)
	{
		if(EXCEPTION_HANDLER_TABLE[cpuno][intno])
		{
			EXCEPTION_HANDLER_TABLE[cpuno][intno](cpuno, intno);
		}
	}
	
	aarch64_intc_int_complete(intno);

}
