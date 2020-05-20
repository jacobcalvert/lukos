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



int main(void *arg)
{
	/* initialize the C library glue code */
	libc_init();
	romfs_hdr_t *hdr = romfs_load((void*)__romfs_data);
	if(hdr != NULL)
	{
		server_start(hdr);
	}
	
	
	while(1);
}



/**
 * the server handles requests via a request pipe, and responds over a response pipe specified by the requestor
 */ 
 
typedef struct
{
	size_t request_type;
	size_t response_pipe;
	size_t request_id;
}rs_request_t;

typedef struct 
{
	size_t request_type;
	

}rs_response_t;

#define RS_RQ_LIST_CONTENTS		0
#define RS_RQ_GET				1

void server_start(romfs_hdr_t *hdr)
{
	
	rs_request_t rq;
	size_t rq_pipe = 0;
	syscall_ipc_pipe_create("romfs-server/requests", sizeof(rs_request_t), 64, 0);
	syscall_ipc_pipe_id_get("romfs-server/requests", &rq_pipe);
	
	while(1)
	{
		while(syscall_ipc_pipe_read(rq_pipe, &rq, sizeof(rq)) == SYSCALL_RESULT_PIPE_EMPTY); /* wait until we have a request */
		
		switch(rq.request_type)
		{
			case RS_RQ_LIST_CONTENTS:
			{
				
			
			break;
			};
			
			case RS_RQ_GET:
			{
				
			break;
			};
			
			default:break;
		}
		
	}
}
