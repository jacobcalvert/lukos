#include <userspace/syscall_numbers.h>
/**
 * allocate a device region to this process
 * @param base		the base address (PA)
 * @param len		the len of the register set
 * @param ptr 		out - the mapped pointer in this AS
 * @return 0 if OK or != on error/issue
 */
int syscall_device_alloc(void *base, size_t len, void **ptr)
{

	return -1;
}
