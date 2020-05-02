#include <managers/process-manager.h>
#include <libraries/mem/memlib.h>
#include <managers/virtual-memory-manager.h>
#include <string.h>


static void thread_list_append(process_t *prc, thread_t *thr);


void pm_init(void)
{

}

process_t *pm_process_create(char *name, address_space_t *as, process_scheduler_t scheduler, size_t priority)
{
	process_t *prc = (process_t*)memlib_malloc(sizeof(process_t));
	
	prc->as = as;
	prc->priority = priority;
	prc->threads = NULL;
	prc->scheduler_type = scheduler;
	
	prc->name = (char*)memlib_malloc(strlen(name)+1);
	memset(prc->name, 0, strlen(name)+1);
	strncpy(prc->name, name, strlen(name));	
	
	return prc;
}

void pm_thread_create(char *name, process_t *prc, void *entry, int argc, char **argv, size_t stack_size, size_t priority)
{
	thread_t *thread = (thread_t *)memlib_malloc(sizeof(thread_t));
	
	thread->priority = priority;
	thread->stack_size = stack_size;
	
	
	thread->name = (char*)memlib_malloc(strlen(name)+1);
	memset(thread->name, 0, strlen(name)+1);
	strncpy(thread->name, name, strlen(name));
	
	thread->argv = (char **)memlib_malloc(sizeof(char*)*argc);
	
	thread->flags = 0;
	
	for(int i = 0; i < argc; i++)
	{
		thread->argv[i] = (char*)memlib_malloc(strlen(argv[i])+1);
		memset(thread->argv[i], 0, strlen(argv[i])+1);
		strncpy(thread->argv[i], argv[i], strlen(argv[i]));
	}
	
	/* we need to 
		1) create the stack in the destination AS
		2) populate the stack in destination AS
	
	
	*/
	
	vmm_address_space_region_create_auto(prc->as, stack_size, AS_REGION_RW, &thread->stack_base);
	
	pm_arch_thread_stack_populate(prc, thread);
	thread_list_append(prc, thread);
	

}

void pm_process_schedule(process_t *prc)
{


}


void thread_list_append(process_t *prc, thread_t *thr)
{
	thread_list_node_t **pTln = &prc->threads;
	while(*pTln != NULL)
	{
		*pTln = (*pTln)->next;
	}
	
	*pTln = (thread_list_node_t*)memlib_malloc(sizeof(thread_list_node_t));
	(*pTln)->thread = thr;
	(*pTln)->next = NULL;
	
}
