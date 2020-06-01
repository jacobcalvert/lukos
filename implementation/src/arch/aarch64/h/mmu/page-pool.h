#ifndef __PAGE_POOL_H__
#define __PAGE_POOL_H__

#include <stdint.h>
#include <stddef.h>

typedef struct page_pool_item
{	
	struct page_pool_item *next;
	void *base;
	
}page_pool_item_t;

typedef struct
{
	size_t page_size;
	void *base;
	size_t len;
	
	void *next_page;
	page_pool_item_t* free;
	
	uint32_t lock;
	
}page_pool_t;


page_pool_t *page_pool_create(size_t page_size, void *base, size_t len);


int page_pool_alloc(page_pool_t *pool, void **ptr);

void page_pool_free(page_pool_t *pool, void *ptr);


#endif
