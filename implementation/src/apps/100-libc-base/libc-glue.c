#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/reent.h>
#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/userspace/schedule.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libc-glue.h>

#define THREAD_INFO_MUTEX_NAME		APP_NAME"-tinfo-mutex"

#define THREAD_INFO_MUTEX_UNLOCK(d)	syscall_ipc_pipe_write(THREAD_INFO_LIST_MUTEX_ID, &d, sizeof(size_t));

#define THREAD_INFO_MUTEX_LOCK(d)	while(syscall_ipc_pipe_write(THREAD_INFO_LIST_MUTEX_ID, &d, sizeof(size_t)) != 0);


static size_t HEAP_BASE_ADDR = 0;
static size_t HEAP_SIZE = 0;

size_t THREAD_INFO_LIST_MUTEX_ID = 0;

static libc_thread_info_node_t *THREAD_INFO_LIST = NULL;
static libc_thread_info_node_t root_thread_info;

static void thread_info_list_append(libc_thread_info_node_t* thinfo);

static void libc_thread_entry(void *arg);


void libc_init(void)
{
	void *lHeap = NULL;
	HEAP_SIZE = LIBC_HEAP_SIZE;
	syscall_memory_alloc(LIBC_HEAP_SIZE, 0, &lHeap);
	HEAP_BASE_ADDR = (size_t)lHeap;
	/* init the root thread reent structure */
	root_thread_info.reent = (struct _reent)_REENT_INIT(root_thread_info.reent);
	syscall_schedule_thread_id_get(&root_thread_info.thread_id);
	root_thread_info.next = NULL;
	
	syscall_ipc_pipe_create(THREAD_INFO_MUTEX_NAME, sizeof(size_t), 1, 0);
	syscall_ipc_pipe_id_get(THREAD_INFO_MUTEX_NAME, &THREAD_INFO_LIST_MUTEX_ID);
	syscall_ipc_pipe_write(THREAD_INFO_LIST_MUTEX_ID, &THREAD_INFO_LIST_MUTEX_ID, sizeof(size_t)); /* give the initial token */
	
	
	THREAD_INFO_LIST = &root_thread_info;

}

void thread_info_list_append(libc_thread_info_node_t *thinfo)
{
	size_t token;
	THREAD_INFO_MUTEX_LOCK(token);
	libc_thread_info_node_t **pTln = &THREAD_INFO_LIST;
	while(*pTln != NULL)
	{
		pTln = &(*pTln)->next;
	}
	
	*pTln = thinfo;
	THREAD_INFO_MUTEX_UNLOCK(token);
}


void libc_thread_start(thread_info_t* params)
{
	thread_info_t *shim_info = (thread_info_t*)malloc(sizeof(thread_info_t));
	shim_info->entry = libc_thread_entry;
	shim_info->arg = (void*) params;
	shim_info->name = params->name;
	shim_info->stack_size = params->stack_size;
	shim_info->priority = params->priority;
	
	syscall_schedule_thread_create(shim_info);
}

void libc_thread_entry(void *arg)
{
	/* common entry point for all libc spawned threads */
	thread_info_t *info = (thread_info_t*)arg;
	libc_thread_info_node_t *tin = (libc_thread_info_node_t*)malloc(sizeof(libc_thread_info_node_t));
	tin->reent = (struct _reent)_REENT_INIT(tin->reent);
	syscall_schedule_thread_id_get(&tin->thread_id);
	tin->next = NULL;
	thread_info_list_append(tin);
	info->entry(info->arg);
}

struct _reent * __getreent (void)
{
	size_t token, id;
	THREAD_INFO_MUTEX_LOCK(token);
	libc_thread_info_node_t *pTln = THREAD_INFO_LIST;
	syscall_schedule_thread_id_get(&id);
	while(pTln != NULL)
	{
		if(id == pTln->thread_id)
		{
			THREAD_INFO_MUTEX_UNLOCK(token);
			return &pTln->reent;
		}
		pTln = pTln->next;
	}
	
	THREAD_INFO_MUTEX_UNLOCK(token);
	return NULL;

}



static char *array[] = {
	"OS=LuKOS"
};
/* pointer to array of char * strings that define the current environment variables */
char **environ = array;


void _exit()
{

}

/* close a file */
int _close(int file)
{
	return -1;

}

int _execve(char *name, char **argv, char **env)
{
return -1;

}
int _fork()
{
return -1;

}
int _fstat(int file, struct stat *st)
{	
	return -1;
}
int _getpid()
{
	return -1;
}
int _isatty(int file)
{
	return -1;
}
int _kill(int pid, int sig)
{
	return -1;
}
int _link(char *old, char *new)
{
	return -1;
}
int _lseek(int file, int ptr, int dir)
{
	return -1;
}

int _open(const char *name, int flags, ...)
{
	return -1;

}
int _read(int file, char *ptr, int len)
{
	 return -1;
}
caddr_t _sbrk(int incr)
{	
	
	void *return_ptr = NULL;
	if( (HEAP_BASE_ADDR + incr) < (HEAP_BASE_ADDR + HEAP_SIZE))
	{
		return_ptr = (void*)HEAP_BASE_ADDR;
		HEAP_BASE_ADDR += incr;
	}
	
	
	return return_ptr;
	
}
int _stat(const char *file, struct stat *st)
{
	return -1;

}
clock_t _times(struct tms *buf)
{
	return -1;

}
int _unlink(char *name)
{
	return -1;

}
int _wait(int *status)
{
	return -1;

}
int _write(int file, char *ptr, int len)
{
	return -1;

}
int _gettimeofday (struct timeval *__restrict __p,
			  void *__restrict __tz)
{
	return -1;
}

