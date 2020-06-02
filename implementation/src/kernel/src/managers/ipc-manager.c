#include <managers/process-manager.h>
#include <managers/virtual-memory-manager.h>
#include <managers/ipc-manager.h>
#include <libraries/mem/memlib.h>
#include <interfaces/os/atomic.h>

#include <string.h>


#define PIPE_LIST_LOCK()			atomic32_spinlock_acquire(&PIPE_LIST_LOCK_VAR)

#define PIPE_LIST_UNLOCK()			atomic32_spinlock_release(&PIPE_LIST_LOCK_VAR)


#define PIPE_LOCK(p)				atomic32_spinlock_acquire(&p->lock);
#define PIPE_UNLOCK(p)				atomic32_spinlock_release(&p->lock);

#define PIPE_RW_LIST_LOCK(l)		atomic32_spinlock_acquire(&l->lock);
#define PIPE_RW_LIST_UNLOCK(l)		atomic32_spinlock_release(&l->lock);

#define THREAD_LOCK(t)				atomic32_spinlock_acquire(&t->lock);
#define THREAD_UNLOCK(t)			atomic32_spinlock_release(&t->lock);



#define PIPE_IDX_INC(p, idx)		( (idx + 1) % p->max_messages)

#define PIPE_IS_EMPTY(p)			((p->max_messages > 1)?(p->head == p->tail):( p->messages[0].len == 0 ))
#define PIPE_IS_FULL(p)				( (p->max_messages > 1)?(PIPE_IDX_INC(p, p->head) == p->tail): (p->messages[0].len != 0) )



typedef struct ipcm_pipe_list_node
{
	struct ipcm_pipe_list_node *next;
	ipc_pipe_t *pipe;

}ipcm_pipe_list_node_t;


static ipcm_pipe_list_node_t *PIPE_LIST = NULL;
static uint32_t PIPE_LIST_LOCK_VAR = 0;
static size_t PIPE_ID = 1000;

static void pipe_list_append(ipc_pipe_t *pipe);
static void rw_list_append(ipc_pipe_rw_list_t *lst, thread_t *thr);
static void rw_list_notify(ipc_pipe_rw_list_t *lst);
static void rw_list_recurse_notify(ipc_pipe_rw_list_node_t *n);
void ipcm_init(void)
{

}

ipc_pipe_t * ipcm_pipe_create(char *name, size_t msg_size, size_t max_msgs, size_t flags)
{
	ipc_pipe_t *pipe = (ipc_pipe_t*)memlib_malloc(sizeof(ipc_pipe_t));
	
	if(name != NULL)
	{
		pipe->name = (char*)memlib_malloc(strlen(name)+1);
		memset(pipe->name, 0, strlen(name)+1);
		strncpy(pipe->name, name, strlen(name));
	
	}
	
	pipe->max_messages = max_msgs;
	pipe->message_size = msg_size;
	pipe->readers.list = NULL;
	pipe->readers.lock = 0;
	pipe->writers.list = NULL;
	pipe->writers.lock = 0;
	pipe->head = 0;
	pipe->tail = 0;
	pipe->lock = 0;
	pipe->flags = flags;
	
	pipe->messages = (ipc_pipe_message_t*)memlib_malloc(sizeof(ipc_pipe_message_t)*max_msgs);
	for(size_t msg = 0; msg < max_msgs; msg++)
	{
		pipe->messages[msg].message = memlib_malloc(msg_size);
		pipe->messages[msg].len = 0;
	}
	
	PIPE_LIST_LOCK();
	pipe->id = PIPE_ID++;
	PIPE_LIST_UNLOCK();
	
	pipe_list_append(pipe);
	
	return pipe;

}

ipc_pipe_t * ipcm_pipe_lookup_by_id(size_t id)
{
	PIPE_LIST_LOCK();
	ipcm_pipe_list_node_t *pPln = PIPE_LIST;
	while(pPln != NULL)
	{
		if(pPln->pipe->id == id)
		{
			PIPE_LIST_UNLOCK();
			return pPln->pipe;
		}
		pPln = pPln->next;
	}
	PIPE_LIST_UNLOCK();
	return NULL;
}

ipc_pipe_t * ipcm_pipe_lookup_by_name(char *name)
{

	PIPE_LIST_LOCK();
	ipcm_pipe_list_node_t *pPln = PIPE_LIST;
	while(pPln != NULL)
	{
		if(strcmp(pPln->pipe->name, name) == 0)
		{
			PIPE_LIST_UNLOCK();
			return pPln->pipe;
		}
		pPln = pPln->next;
	}
	PIPE_LIST_UNLOCK();
	return NULL;
	
}

int ipcm_pipe_write(thread_t *from, size_t id, void *vamsg, size_t len)
{
	ipc_pipe_t* pipe = ipcm_pipe_lookup_by_id(id);

	void *source_pa = (from == NULL)?vamsg:vmm_arch_v2p(from->parent->as->arch_context, vamsg); /** allows the kernel to use this mechanism too */
	if(pipe == NULL)
	{
		return IPC_RESULT_PIPE_NOT_FOUND;
	}
	
	if(len > pipe->message_size)
	{
		return IPC_RESULT_MESSAGE_TOO_LARGE;
	}
	
	
	PIPE_LOCK(pipe);
	
	if(PIPE_IS_FULL(pipe))
	{
		if(pipe->flags & IPC_PIPE_FLAGS_PURGE_ON_FULL)
		{
			/* make room! */
			pipe->tail =  PIPE_IDX_INC(pipe, pipe->tail);
		}
		else
		{
			rw_list_append(&pipe->writers, from); /* to notify him when we are ready */
			THREAD_LOCK(from);
			from->blocked_by = pipe;
			THREAD_UNLOCK(from);
			PIPE_UNLOCK(pipe);
			return IPC_RESULT_PIPE_FULL;
		}
	}
	
	memcpy(pipe->messages[pipe->head].message, source_pa, len);
	pipe->messages[pipe->head].len = len;
	pipe->head = PIPE_IDX_INC(pipe, pipe->head);
	/* notify readers */
	rw_list_notify(&pipe->readers);
	PIPE_UNLOCK(pipe);
	return 0;
}

int ipcm_pipe_read(thread_t *to, size_t id, void *vamsg, size_t *valen)
{

	ipc_pipe_t* pipe = ipcm_pipe_lookup_by_id(id);
	void *dest_msg_pa = (to == NULL)?vamsg:vmm_arch_v2p(to->parent->as->arch_context, vamsg);
	size_t *dest_len_pa = (to == NULL)?valen:vmm_arch_v2p(to->parent->as->arch_context, valen);
	if(pipe == NULL)
	{
		return IPC_RESULT_PIPE_NOT_FOUND;
	}
	
	PIPE_LOCK(pipe);
	
	if(PIPE_IS_EMPTY(pipe))
	{
		rw_list_append(&pipe->readers, to); /* to notify him when we are ready */
		THREAD_LOCK(to);
		to->blocked_by = pipe;
		THREAD_UNLOCK(to);
		PIPE_UNLOCK(pipe);
		return IPC_RESULT_PIPE_EMPTY;
	}
	
	
	
	memcpy(dest_msg_pa, pipe->messages[pipe->tail].message, pipe->messages[pipe->tail].len);
	*dest_len_pa = pipe->messages[pipe->tail].len;
	pipe->messages[pipe->tail].len = 0;
	
	pipe->tail = PIPE_IDX_INC(pipe, pipe->tail);
	
	/* notify writers if we have any */
	rw_list_notify(&pipe->writers);
	
	PIPE_UNLOCK(pipe);
	return 0;
}
void rw_list_recurse_notify(ipc_pipe_rw_list_node_t *n)
{
	if(n != NULL)
	{
		rw_list_recurse_notify(n->next);
		THREAD_LOCK((n->thread));
		n->thread->blocked_by = NULL;
		THREAD_UNLOCK((n->thread));
		memlib_free(n);
		n = NULL;
	}
}
void rw_list_notify(ipc_pipe_rw_list_t *list)
{
	PIPE_RW_LIST_LOCK(list);
	rw_list_recurse_notify(list->list);
	list->list = NULL;
	PIPE_RW_LIST_UNLOCK(list);

}

void rw_list_append(ipc_pipe_rw_list_t *list, thread_t *thr)
{
	PIPE_RW_LIST_LOCK(list);
	ipc_pipe_rw_list_node_t ** pRWln = &list->list;
	while(*pRWln != NULL)
	{
			pRWln = &(*pRWln)->next;
	}
	
	*pRWln = (ipc_pipe_rw_list_node_t*)memlib_malloc(sizeof(ipc_pipe_rw_list_node_t));
	(*pRWln)->thread = thr;
	(*pRWln)->next = NULL;
	
	PIPE_RW_LIST_UNLOCK(list);
}


void pipe_list_append(ipc_pipe_t *pipe)
{
	ipcm_pipe_list_node_t **pPln = &PIPE_LIST;
	PIPE_LIST_LOCK();
	while(*pPln != NULL)
	{
			pPln = &(*pPln)->next;
	}
	*pPln = (ipcm_pipe_list_node_t*)memlib_malloc(sizeof(ipcm_pipe_list_node_t));
	(*pPln)->pipe = pipe;
	(*pPln)->next = NULL;
	
	PIPE_LIST_UNLOCK();
	
}
