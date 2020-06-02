
/*
 * size_t cpu_current_get(void);
 */
 
.global cpu_current_get
cpu_current_get:
	mrs x0, mpidr_el1
	and x0, x0, #0x0F
	ret
