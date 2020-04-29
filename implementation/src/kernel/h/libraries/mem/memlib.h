/**
 * @file memlib.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date February 07, 2020
 * @brief This file defines the public interface to the memlib component.
 *
 * The memory library (memlib) interface is defined here. There
 * are two types of memory interactions: global heap and
 * private heap. The global heap must be initialized by
 * calling memlib_init and then heap regions must be added to
 * it by calling memlib_heap_add. Once the global heap has
 * been initialized, calls to memlib_heap_alloc/free and any
 * private heap functions can be used as well.
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


#ifndef _MEMLIB_H_
#define _MEMLIB_H_

#include <stddef.h>
#include <stdint.h>

#define MEMLIB_OK				0
#define MEMLIB_OUT_OF_MEMORY	-1
#define MEMLIB_NO_OPERATION		-2
#define MEMLIB_BAD_CONTEXT		-3


struct memlib_priv_heap;
typedef struct memlib_priv_heap* memlib_priv_heap_t;

typedef struct memlib_stats
{
	size_t total;
	size_t allocated;
	size_t free;
}memlib_stats_t;


typedef struct
{
	/**
	 * create a memory allocator context with memory area 'base'
	 * @param is_global		set to 1 if it is the global heap
	 * @return the context pointer
	 */
	void *(*create_context)(int is_global);

	/**
	 * destroy the context created
	 * @param ctx		the context
	 */
	void (*destroy_context)(void *ctx);

	/**
	 * allocate a block of memory >= n bytes
	 * @param ctx	the implementation's context
	 * @param n		number of bytes requested
	 * @return a pointer to the block or NULL on failure
	 */
	void* (*malloc)(void *ctx, size_t n);

	/**
	 * free an allocated memory block
	 * @param ctx	the implementation's context
	 * @param ptr		the block ptr
	 */
	void (*free)(void *ctx, void *ptr);

	/**
	 * add a heap area to the heap
	 * @param ctx	the implementation's context
	 * @param base		pointer to the base
	 * @param len		the number of bytes available
	 * @return 0 if OK, <0 on error
	 */
	int (*heap_add)(void *ctx, void * base, size_t len);

	/**
	 * retrieve memory allocator statistics
	 * @param ctx	the implementations' context
	 * @param stats 	out the stats
	 */
	void (*stats_get)(void *ctx, memlib_stats_t *stats);

}memlib_ops_t;


/**
 * initialize the base memory library for global heap
 */
void memlib_init(memlib_ops_t *impl);


/**
 * add a heap area to the global heap
 * @param base		pointer to the base
 * @param len		the number of bytes available
 * @return 0 if OK, <0 on error
 */

int memlib_heap_add(void *base, size_t len);

/**
 * allocate memory from the global heap
 * @param n		number of bytes requested
 * @return pointer to memblock or NULL
 */
void *memlib_malloc(size_t n);

/**
 * free a block of memory in the global heap
 * @param ptr	pointer to block
 */
void memlib_free(void *ptr);

/**
 * retrieve stats for the global heap
 * @param stats out the stats
 */
void memlib_stats_get(memlib_stats_t *stats);

/**
 * create a private heap
 * @param n		requested size of private heap
 * @param ops	the implementation to use on the private heap
 * @return a private heap handle
 */
memlib_priv_heap_t memlib_priv_heap_create(size_t n, memlib_ops_t *ops);


/**
 * destroy a private heap
 * @param pheap		the private heap handle
 *
 * @note this will free all memory malloc'd from this heap! Beware!
 */
void memlib_priv_heap_destroy(memlib_priv_heap_t pheap);

/**
 * allocate some memory from a private heap
 * @param pheap		the private heap handle
 * @param n			the number of requested bytes
 * @return a pointer to the block or NULL
 */
void *memlib_priv_heap_malloc(memlib_priv_heap_t pheap, size_t n);

/**
 * free memory allocated from a private heap
 * @param pheap		the private heap handle
 * @param ptr		the pointer to memory to be freed
 */
void memlib_priv_heap_free(memlib_priv_heap_t pheap, void *ptr);

/**
 * retrieve stats for the global heap
 * @param pheap		the private heap handle
 * @param stats out the stats
 */
void memlib_priv_heap_stats_get(memlib_priv_heap_t pheap, memlib_stats_t *stats);

#endif /* _MEMLIB_H_ */
