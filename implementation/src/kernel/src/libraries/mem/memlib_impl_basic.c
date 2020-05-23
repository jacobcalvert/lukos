/**
 * @file memlib_impl_basic.c
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date February 07, 2020
 * @brief This is a basic implementation of the functions required by memlib.
 *
 * This heap allocator and memory manager functions by created
 * a linked list of all allocated blocks. If a request is
 * made, the first fitting block is used. If there are no
 * fitting blocks, a compaction scheme to consolidate adjacent
 * free blocks is used.
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
#include <interfaces/os/atomic.h>
#include <string.h>

#include <stdint.h>

#define NUM_HEAP_REGIONS		4

/* mib -> memlib_impl_basic */



#define MARK_USED(flags)			((flags)| 0x00000001)
#define MARK_FREE(flags)			((flags)& 0xFFFFFFFE)
#define IS_USED(flags)				((flags)& 0x00000001)
#define SET_SIZE(flags, size)		((flags) | ((size)<<1))
#define GET_SIZE(flags)				((flags)>>1)

typedef struct mib_marker_block
{
	uint32_t flags;
	struct mib_marker_block *next;
}mib_marker_block_t;

#define MARKER_SIZE			sizeof(mib_marker_block_t)
#define REQD_SIZE(n)		(MARKER_SIZE + n)
typedef struct
{
	void *base;
	void *free_base;
	size_t len;
	size_t free;
	mib_marker_block_t *blocks;

}mib_heap_region_t;


typedef struct
{
	mib_heap_region_t regions[NUM_HEAP_REGIONS];
	uint32_t lock;

}mib_heap_context_t;


static mib_heap_context_t GLOBAL_HEAP_CONTEXT;


static void *memlib_impl_basic_create_context(int is_global);
static void memlib_impl_basic_destroy_context(void* ctx);
static int memlib_impl_basic_heap_add(void* ctx, void *base, size_t n);
static void memlib_impl_basic_free(void* ctx, void *ptr);

static void *memlib_impl_basic_malloc(void* ctx, size_t n);



static mib_marker_block_t *find_existing_block(mib_heap_context_t *context, size_t n);
static mib_marker_block_t *allocate_block(mib_heap_context_t *context, size_t n);
static mib_marker_block_t *compact_list(mib_heap_context_t *context, size_t n);


const memlib_ops_t MEMLIB_IMPL_BASIC_OPS =
{
		.create_context = memlib_impl_basic_create_context,
		.destroy_context = memlib_impl_basic_destroy_context,
		.malloc = memlib_impl_basic_malloc,
		.heap_add = memlib_impl_basic_heap_add,
		.free = memlib_impl_basic_free
};

void *memlib_impl_basic_create_context(int is_global)
{
	mib_heap_context_t *context = NULL;
	if(is_global)
	{
		context = &GLOBAL_HEAP_CONTEXT;
	}
	else
	{
		/* allocate from global heap */
		context = (mib_heap_context_t*) memlib_malloc(sizeof(mib_heap_context_t));
	}
	
	context->lock = 0;

	for(int i = 0; i < NUM_HEAP_REGIONS; i++)
	{
		context->regions[i].base = NULL;
		context->regions[i].len = 0;
		context->regions[i].free = 0;
		context->regions[i].free_base = NULL;
		context->regions[i].blocks = NULL;
	}

	return (void*) context;
}
void memlib_impl_basic_destroy_context(void* ctx)
{
	if(ctx != (void*)&GLOBAL_HEAP_CONTEXT)
	{
		memlib_free(ctx);
	}
}

int memlib_impl_basic_heap_add(void* ctx, void *base, size_t n)
{
	if(ctx)
	{
		mib_heap_context_t *context = (mib_heap_context_t*)ctx;
		atomic32_spinlock_acquire(&context->lock);
		for(int i = 0; i < NUM_HEAP_REGIONS; i++)
		{
			if(context->regions[i].base == NULL)
			{
				context->regions[i].base = base;
				context->regions[i].free_base = base;
				context->regions[i].len = n;
				context->regions[i].free = n;
				atomic32_spinlock_release(&context->lock);
				return MEMLIB_OK;
			}
		}
		atomic32_spinlock_release(&context->lock);
		return MEMLIB_OUT_OF_MEMORY;
	}
	return MEMLIB_BAD_CONTEXT;
}

void *memlib_impl_basic_malloc(void* ctx, size_t n)
{

	if(ctx == NULL)return NULL;
	if(n == 0)return NULL;
	mib_heap_context_t *context = (mib_heap_context_t*)ctx;
	mib_marker_block_t *block = NULL;


	atomic32_spinlock_acquire(&context->lock);
	block = find_existing_block(context, n);
	if(block == NULL)
	{
		block = allocate_block(context, n);
		if(block == NULL)
		{
			block = compact_list(context, n); /*if it's null here, we are just out of memory, period */
		}
	}
	block->flags = MARK_USED(block->flags); 
	size_t addr = ((size_t)block + MARKER_SIZE);
	atomic32_spinlock_release(&context->lock);
	return (block==NULL)?NULL:((void*)addr);
}

void memlib_impl_basic_free(void* ctx, void *ptr)
{
	if(ctx == NULL)return;
	mib_marker_block_t *block = (mib_marker_block_t*) ((size_t)ptr - MARKER_SIZE);
	if(IS_USED(block->flags))
	{
		block->flags = MARK_FREE(block->flags);
	}
}

mib_marker_block_t *find_existing_block(mib_heap_context_t *context, size_t n)
{
	for(int i = 0; i < NUM_HEAP_REGIONS; i ++)
	{
		mib_marker_block_t *p = context->regions[i].blocks;

			while(p != NULL)
			{
				if( (!IS_USED(p->flags)) && (GET_SIZE(p->flags) >= n) )
				{
					return p;
				}
				p = p->next;
			}

	}
	return NULL;
}

mib_marker_block_t *allocate_block(mib_heap_context_t *context, size_t n)
{
	size_t total_size_needed = REQD_SIZE(n) + 7;
	mib_marker_block_t *block = NULL;
	mib_marker_block_t *p;
	total_size_needed >>= 3;
	total_size_needed <<= 3;

	for(int i = 0; i < NUM_HEAP_REGIONS; i ++)
	{
		size_t end_needed = ((size_t)context->regions[i].free_base + total_size_needed);
		size_t end = (size_t)context->regions[i].base + context->regions[i].len;
		if(end_needed < end)
		{
			/* use this one! */
			block = (mib_marker_block_t*)context->regions[i].free_base;
			block->flags = 0;
			p = context->regions[i].blocks;

			if(context->regions[i].blocks == NULL)
			{
				context->regions[i].blocks = block;
			}
			else
			{
				while(p && p->next != NULL)
				{
					p = p->next;
				}

				p->next = block;
			}

			block->next = NULL;

			block->flags = MARK_USED(block->flags);
			block->flags = SET_SIZE(block->flags, n);

			context->regions[i].free_base = (void*)end_needed;
			goto allocate_block_done;
		}
	}
allocate_block_done:
	return block;
}

mib_marker_block_t *compact_list(mib_heap_context_t *context, size_t n)
{
	size_t total_size_needed = REQD_SIZE(n) + 7;
	mib_marker_block_t *p =  NULL;
	total_size_needed >>= 3;
	total_size_needed <<= 3;

	for(int i = 0; i < NUM_HEAP_REGIONS; i ++)
	{
		size_t running_total = 0;
		mib_marker_block_t *start, *end;
		p = context->regions[i].blocks;
		while(p != NULL)
		{
			running_total = 0;
			while(p != NULL && IS_USED(p->flags))p = p->next; /* skip used blocks */

			start = p;
			while(p != NULL && !IS_USED(p->flags)  && (running_total < total_size_needed)) /* find free adjacent blocks */
			{
				running_total += GET_SIZE(p->flags) + MARKER_SIZE;
				p = p->next;
			}
			end = p;

			if(running_total >= total_size_needed)
			{
				start->flags = 0;
				start->flags = SET_SIZE(start->flags, (size_t)end  - (size_t)start - MARKER_SIZE);
				start->next = end;
				start->flags = MARK_USED(start->flags);
				return start;
			}
		}

	}

	return NULL;

}
