/**
 * @file atomic.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date May 02, 2020
 * @brief This file defines the atomic functions required by the OS.
 * 
 * Atomic functions are used to implement various
 * synchronization mechanisms.
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

#ifndef __LUKOS_ATOMIC__
#define __LUKOS_ATOMIC__


/**
 * atomically set the value at ptr to value
 * @param ptr		the pointer
 * @param value		the value
 * @return ATOMIC_OK or ATOMIC_ERROR
 */
int atomic32_set(void *ptr, uint32_t value);


/**
 * atomically increment the value at ptr to value
 * @param ptr		the pointer
 * @return ATOMIC_OK or ATOMIC_ERROR
 */
int atomic32_inc(uint32_t *ptr);

/**
 * atomically decrement the value at ptr to value
 * @param ptr		the pointer
 * @return ATOMIC_OK or ATOMIC_ERROR
 */
int atomic32_dec(uint32_t *ptr);

/**
 * acquire a spinlock
 */
void atomic32_spinlock_acquire(uint32_t *p);


/**
 * release a spinlock
 */
 
void atomic32_spinlock_release(uint32_t *p);



#endif 
