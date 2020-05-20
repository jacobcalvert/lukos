#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/userspace/schedule.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libc-glue.h>

void application_start(void);

int main(void *arg)
{
	/* initialize the C library glue code */
	libc_init();
	
	void *p = malloc(1045);
	
	application_start();
}




void application_start(void)
{

	while(1);
}
