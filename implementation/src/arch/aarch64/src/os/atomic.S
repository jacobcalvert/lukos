
/*
 * int atomic32_set(void *ptr, uint32_t value);
 * perform a test-and-set on the memory at [ptr]
 *  
 * ptr will be in x0 and return will go into x0
 */

.global atomic32_set

atomic32_set:
	ldxr w2, [x0]			/* tag the memory for exclusive access */
	stxr w2, w1, [x0]		/* try to set it, result in w1, 0 = success */
	cbz	w2, atomic32_set_success
atomic32_set_fail:
	clrex
	mov x0, #-1
	ret
atomic32_set_success:
	mov x0, #0
	ret

.global atomic32_inc
atomic32_inc:
	ldaxr w2, [x0]
	add w2, w2, #1
	stxr w1, w2, [x0]
	mov x0, x1
	ret


.global atomic32_dec
atomic32_dec:
	ldaxr w2, [x0]
	add w2, w2, #-1
	stxr w1, w2, [x0]
	mov x0, x1
	ret


/* void atomic32_spinlock_acquire(uint32_t *p); */
.global atomic32_spinlock_acquire
atomic32_spinlock_acquire:
	mov w2, #1
	sevl
atomic32_spinlock_acquire_loop1:
	wfe
atomic32_spinlock_acquire_loop2:
	ldaxr w1, [x0]
	cbnz w1, atomic32_spinlock_acquire_loop1
	stxr w1, w2, [x0]
	cbnz w1, atomic32_spinlock_acquire_loop2
	ret
	

/* void atomic32_spinlock_release(uint32_t *p); */
.global atomic32_spinlock_release
atomic32_spinlock_release:
	stlr wzr, [x0]
	ret

