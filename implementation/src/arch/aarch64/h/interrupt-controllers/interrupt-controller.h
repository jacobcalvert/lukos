#ifndef __AARCH64_INTERRUPT_CONTROLLER__
#define __AARCH64_INTERRUPT_CONTROLLER__

#define AARCH64_INTC_OK		0
#define AARCH64_INTC_ERROR	-1

#include <stddef.h>

typedef void (*aarch64_int_handler)(size_t cpuno, size_t intno);
typedef struct
{
	void *context;
	void (*init)(void*ctx);
	int (*enable)(void *ctx, size_t intno, size_t cpuno, aarch64_int_handler handler);
	int (*enable_by_dtb)(void *ctx, void *properties, size_t pri, size_t cpuno, aarch64_int_handler handler);
	int (*disable)(void *ctx, size_t intno, size_t cpuno);
	int (*priority_set)(void *ctx, size_t intno, size_t priority);
	size_t (*current_get)(void* ctx);
	void (*complete)(void* ctx, size_t intno);
 	void (*generate)(void *ctx, size_t intno);
 	void (*ipi_generate)(void *ctx, size_t cpuno, size_t intno);

}aarch64_intc_t;


void aarch64_intc_register(void);

void aarch64_intc_init(void);

size_t aarch64_intc_current_int_get(void);

void aarch64_intc_int_complete(size_t intno);

int aarch64_intc_int_enable_by_properties(void *prop, size_t pri, size_t cpuno, aarch64_int_handler handler);

#endif
