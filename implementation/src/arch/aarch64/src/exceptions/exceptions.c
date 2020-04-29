#include <exceptions/exceptions.h>


void aarch64_exceptions_init(void)
{
	extern char _default_el1_vtable;
	void *vbar_el1 = &_default_el1_vtable;
	 __asm ("msr vbar_el1, %[vbar_el1]" : : [vbar_el1] "r" (vbar_el1));
}
