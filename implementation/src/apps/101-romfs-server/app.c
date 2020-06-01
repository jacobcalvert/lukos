#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/userspace/schedule.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libc-glue.h>
#include <romfs.h>

void server_start(romfs_hdr_t *hdr);

extern const char *__romfs_data;

static int initial_iterator(char *name, void *base, uint32_t length);

int main(void *arg)
{
	/* initialize the C library glue code */
	libc_init();
	printf("romfs-server: application loaded\r\n");
	romfs_hdr_t *hdr = romfs_load((void*)&__romfs_data);
	if(hdr != NULL)
	{
		server_start(hdr);
	}
	
	
	while(1);
}

int initial_iterator(char *name, void *base, uint32_t length)
{
	printf("romfs-server: found file %s @ %p (%.2fkiB)\r\n", name, base, (float)length/1024.0);
	return 1;
}	



void server_start(romfs_hdr_t *hdr)
{
	
	romfs_iterate_files(hdr, initial_iterator);
	while(1)
	{
	
		
	}
}
