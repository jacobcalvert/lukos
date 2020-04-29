/**
 * @file memlib.c
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date February 07, 2020
 * @brief This file implements the public interface defined at memlib.h
 *
 * This implementation deals with the abstraction created in
 * memlib.h. Since the global heap and any private heaps can
 * have different allocation schema via the memlib_opt_t type,
 * this layer of memlib handles this abstraction.
 *
 *
 *
 * Copyright (c) 2020 Jacob Calvert
 * All rights reserved.
 *
 * This file is subject to the terms and conditions
 * defined in 'LICENSE.txt' provided with this source
 * code package.
 *
 */
#include <libraries/mem/memlib.h>
#include <string.h>

static struct memlib_priv_heap GLOBAL_HEAP;

#define GLOBAL_HEAP_FLAG	1
#define PRIV_HEAP_FLAG		0


struct memlib_priv_heap
{
	void *context;
	memlib_ops_t *ops;
};


void memlib_init(memlib_ops_t *impl)
{
	GLOBAL_HEAP.ops = impl;
	GLOBAL_HEAP.context = GLOBAL_HEAP.ops->create_context(GLOBAL_HEAP_FLAG);
}

int memlib_heap_add(void *base, size_t len)
{
	if(GLOBAL_HEAP.ops && GLOBAL_HEAP.ops->heap_add)
	{
		return GLOBAL_HEAP.ops->heap_add(GLOBAL_HEAP.context, base, len);
	}
	return MEMLIB_NO_OPERATION;
}

void *memlib_malloc(size_t n)
{
	if(GLOBAL_HEAP.ops && GLOBAL_HEAP.ops->malloc)
	{
		return GLOBAL_HEAP.ops->malloc(GLOBAL_HEAP.context, n);
	}
	return NULL;
}

void memlib_free(void *ptr)
{
	if(GLOBAL_HEAP.ops && GLOBAL_HEAP.ops->free)
	{
		GLOBAL_HEAP.ops->free(GLOBAL_HEAP.context, ptr);
	}
}
void memlib_stats_get(memlib_stats_t *stats)
{
	if(GLOBAL_HEAP.ops && GLOBAL_HEAP.ops->stats_get)
	{
		GLOBAL_HEAP.ops->stats_get(GLOBAL_HEAP.context, stats);
	}
}

memlib_priv_heap_t memlib_priv_heap_create(size_t n, memlib_ops_t *ops)
{
	memlib_priv_heap_t pheap = (memlib_priv_heap_t)memlib_malloc(sizeof(struct memlib_priv_heap));
	void * heap = memlib_malloc(n);
	memset(heap, 0, n);
	pheap->ops = ops;
	pheap->context = pheap->ops->create_context(PRIV_HEAP_FLAG);
	pheap->ops->heap_add(pheap->context, heap, n);
	return pheap;
}

void memlib_priv_heap_destroy(memlib_priv_heap_t pheap)
{
	if(pheap)
	{
		pheap->ops->destroy_context(pheap->context);
	}
}

void *memlib_priv_heap_malloc(memlib_priv_heap_t pheap, size_t n)
{
	if(pheap)
	{
		return pheap->ops->malloc(pheap->context, n);
	}
	return NULL;
}


void memlib_priv_heap_free(memlib_priv_heap_t pheap, void *ptr)
{
	if(pheap)
	{
		pheap->ops->free(pheap->context, ptr);
	}
}

void memlib_priv_heap_stats_get(memlib_priv_heap_t pheap, memlib_stats_t *stats)
{
	if(pheap)
	{
		pheap->ops->stats_get(pheap->context, stats);
	}
}

