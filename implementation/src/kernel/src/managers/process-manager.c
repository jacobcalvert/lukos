#include <managers/process-manager.h>
#include <managers/virtual-memory-manager.h>
#include <libraries/mem/memlib.h>
#include <interfaces/os/atomic.h>

#include <string.h>

#define PROCESS_LOCK(prc)			atomic32_spinlock_acquire(&prc->lock)
#define PROCESS_UNLOCK(prc)			atomic32_spinlock_release(&prc->lock)

#define PROCESS_LIST_UNLOCK()		atomic32_spinlock_release(&PROCESS_LIST_LOCK)
#define PROCESS_LIST_LOCK()			atomic32_spinlock_acquire(&PROCESS_LIST_LOCK)

typedef struct process_list
{
	struct process_list *next;
	process_t* process;

}process_list_node_t;

static process_list_node_t *PROCESS_LIST = NULL;
static uint32_t PROCESS_LIST_LOCK = 0;

static size_t MAX_CPUS = 0;

static thread_t **CURRENT_THREAD = NULL;

void idle(void *arg);
static void thread_list_append(process_t *prc, thread_t *thr);
static void process_list_append(process_t *prc);

static thread_t* process_scheduler_run(process_t *p, size_t cpuno);

void pm_init(size_t max_cpus)
{
	MAX_CPUS = max_cpus;
	CURRENT_THREAD = (thread_t **)memlib_malloc(sizeof(thread_t*)*MAX_CPUS);
	size_t i = 0;
	while(i < MAX_CPUS)
	{
		CURRENT_THREAD[i++] = NULL;
	}	
}

process_t *pm_process_create(char *name, address_space_t *as, process_scheduler_t scheduler, size_t priority)
{
	process_t *prc = (process_t*)memlib_malloc(sizeof(process_t));
	
	prc->as = as;
	prc->priority = priority;
	prc->threads = NULL;
	prc->scheduler_type = scheduler;
	prc->lock = 0;
	
	prc->name = (char*)memlib_malloc(strlen(name)+1);
	memset(prc->name, 0, strlen(name)+1);
	strncpy(prc->name, name, strlen(name));	
	
	return prc;
}

thread_t* pm_thread_create(char *name, process_t *prc, void *entry, void *arg, size_t stack_size, size_t priority)
{
	thread_t *thread = (thread_t *)memlib_malloc(sizeof(thread_t));
	
	thread->priority = priority;
	thread->stack_size = stack_size;
	thread->parent = prc;
	thread->affinity = PM_THREAD_AFF_NONE;
	thread->entry = entry;
	
	
	thread->name = (char*)memlib_malloc(strlen(name)+1);
	memset(thread->name, 0, strlen(name)+1);
	strncpy(thread->name, name, strlen(name));
	
	thread->arg = arg;
	
	thread->flags = PM_THREAD_FLAGS_READY;
	
	/* we need to 
		1) create the stack in the destination AS
		2) populate the stack in destination AS
	
	
	*/
	
	vmm_address_space_region_create_auto(prc->as, stack_size, AS_REGION_RW, &thread->stack_base);
	
	pm_arch_thread_stack_populate(prc, thread);
	
	thread_list_append(prc, thread);
	
	return thread;
	

}

void pm_thread_affinity_set(thread_t *thr, size_t aff)
{
	if(thr)
	{
		thr->affinity = aff;
	}

}

thread_t *pm_thread_next_get(size_t cpuno)
{
	/*
	 * iterate our processes
	 *		for each process, iterate threads
	 * 			call thread scheduler
	 */
	thread_t *thread = NULL;
	thread_t *selected = NULL;
	size_t max = (size_t) -1;
 	PROCESS_LIST_LOCK();
 	CURRENT_THREAD[cpuno]->flags &= ~PM_THREAD_FLAGS_RUNNING;
 	process_list_node_t *pPln = PROCESS_LIST;
 	while(pPln != NULL)
 	{
 		process_t *prc = pPln->process;
 		thread = process_scheduler_run(prc, cpuno);
 		if(thread != NULL && (prc->priority < max))
 		{
 			selected = thread;
 			max = prc->priority;
 		}
 		
 		pPln =  pPln->next;
 	}
 
 	
 	CURRENT_THREAD[cpuno] = selected;
 	selected->flags |= PM_THREAD_FLAGS_RUNNING;
 	
	PROCESS_LIST_UNLOCK();
	
	return selected;

}

thread_t* process_scheduler_run(process_t *prc, size_t cpuno)
{
	PROCESS_LOCK(prc);
	thread_list_node_t *pTln = prc->threads;
	thread_t *selected = NULL;
	switch(prc->scheduler_type)
	{
		case PM_SCHEDULER_ROUND_ROBIN:
		{
			
			break;
		
		};
		case PM_SCHEDULER_PRIORITY:
		{
			size_t max = (size_t)-1;
			while(pTln != NULL)
			{
				
				if( (pTln->thread->affinity == PM_THREAD_AFF_NONE) || (pTln->thread->affinity & PM_THREAD_AFF_CORE(cpuno)) )
				{
					pTln->thread->flags |= PM_THREAD_FLAGS_READY;
					
					if(pTln->thread->blockers != 0)
					{
						pTln->thread->flags &= ~PM_THREAD_FLAGS_READY;
					}
					
					if(pTln->thread->flags & PM_THREAD_FLAGS_RUNNING)
					{
						pTln->thread->flags &= ~PM_THREAD_FLAGS_READY;
					}
					
					if(pTln->thread->flags & PM_THREAD_FLAGS_READY)
					{
						if(pTln->thread->priority < max)
						{
							selected = pTln->thread;
							max = pTln->thread->priority;
						}
					}
				}
				pTln = pTln->next;
			
			}
			
		};
		default:break;
	
	}
	
	PROCESS_UNLOCK(prc);
	return selected;
}

thread_t *pm_thread_current_get(size_t cpuno)
{
	if(cpuno < MAX_CPUS)
	{
		return CURRENT_THREAD[cpuno];
	}
	return NULL;
}

void pm_process_schedule(process_t *prc)
{
	process_list_append(prc);
}

void process_list_append(process_t *prc)
{	
	PROCESS_LIST_LOCK();
	process_list_node_t **pPln = &PROCESS_LIST;
	while(*pPln != NULL)
	{
		pPln = &(*pPln)->next;
	}
	
	*pPln = (process_list_node_t*)memlib_malloc(sizeof(process_list_node_t));
	(*pPln)->process = prc;
	(*pPln)->next = NULL;
	PROCESS_LIST_UNLOCK();

}
void thread_list_append(process_t *prc, thread_t *thr)
{
	PROCESS_LOCK(prc);
	thread_list_node_t **pTln = &prc->threads;
	while(*pTln != NULL)
	{
		pTln = &(*pTln)->next;
	}
	
	*pTln = (thread_list_node_t*)memlib_malloc(sizeof(thread_list_node_t));
	(*pTln)->thread = thr;
	(*pTln)->next = NULL;
	PROCESS_UNLOCK(prc);
	
}
