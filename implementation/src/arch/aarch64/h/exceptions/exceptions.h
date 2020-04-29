#ifndef __AARCH64_EXCEPTIONS__
#define __AARCH64_EXCEPTIONS__

#include <interrupt-controllers/interrupt-controller.h>
#include <stddef.h>


void aarch64_exceptions_init(void);

void aarch64_exceptions_handler_register(size_t cpuno, size_t intno, aarch64_int_handler handler);
#endif
