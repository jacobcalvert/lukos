#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/userspace/schedule.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libc-glue.h>
#include <terminal-server.h>


int main(void *arg)
{
	/* initialize the C library glue code */
	libc_init();
	terminal_server_init();
	
	while(1); /* if we ever return */
}

