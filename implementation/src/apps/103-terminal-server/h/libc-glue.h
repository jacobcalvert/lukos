#ifndef __LIBC_GLUE_H__
#define __LIBC_GLUE_H__
#include <sys/reent.h>
#include <stddef.h>
#include <interfaces/userspace/schedule.h>

#define KiB	1024
#define MiB	KiB*1024

#ifndef THREAD_INFO_MAX_SIZE
#define THREAD_INFO_MAX_SIZE	64
#endif

#ifndef LIBC_HEAP_SIZE
#define LIBC_HEAP_SIZE			16*MiB
#endif

typedef struct libc_thread_info_node
{
	struct libc_thread_info_node *next;
	size_t thread_id;
	struct _reent reent;
	
}libc_thread_info_node_t;

typedef struct
{
	void (*entry)(void *arg);
	void *arg;
}libc_thread_params_t;


void libc_init(void);

void libc_thread_start(thread_info_t* params);


#endif
