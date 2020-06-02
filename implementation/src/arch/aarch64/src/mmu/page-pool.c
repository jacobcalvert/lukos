#include <mmu/page-pool.h>
#include <libraries/mem/memlib.h>
#include <interfaces/os/atomic.h>
#include <string.h>
#define POOL_LOCK(p)		(atomic32_spinlock_acquire(&p->lock))
#define POOL_UNLOCK(p)		(atomic32_spinlock_release(&p->lock))

page_pool_t *page_pool_create(size_t page_size, void *base, size_t len)
{
	page_pool_t *pool = (page_pool_t*)memlib_malloc(sizeof(page_pool_t));
	
	pool->lock = 0;
	pool->base = base;
	pool->len = len;
	pool->free = NULL;
	pool->next_page = base;
	pool->page_size = page_size;
	
	return pool;

}


int page_pool_alloc(page_pool_t *pool, void **ptr)
{
	POOL_LOCK(pool);
	page_pool_item_t *p = pool->free;
	if(p)
	{
		pool->free = p->next;
		*ptr = p->base;
		memlib_free(p);
	
	}
	else
	{
		if(((size_t) pool->base + pool->len  ) <= (size_t)pool->next_page)
		{
				POOL_UNLOCK(pool);
				return -1;
		}
		else
		{
			*ptr = pool->next_page;
			pool->next_page = (void *)(size_t)pool->next_page + pool->page_size;
		}
	
	}

	POOL_UNLOCK(pool);
	memset(*ptr, 0, pool->page_size);
	return 0;

}

void page_pool_free(page_pool_t *pool, void *tofree)
{
	POOL_LOCK(pool);
	
	page_pool_item_t **ptr = &pool->free;
	while(*ptr != NULL)
	{
		(*ptr) = (*ptr)->next;
	}
	(*ptr) = (page_pool_item_t*)memlib_malloc(sizeof(page_pool_item_t));
	(*ptr)->next = NULL;
	(*ptr)->base = tofree;
	
	
	POOL_UNLOCK(pool);
}

