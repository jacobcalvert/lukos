#ifndef __LUKOS_USERSPACE_MACROS__
#define __LUKOS_USERSPACE_MACROS__

#include <managers/process-manager.h>

#define KERNEL_SYSCALL0(name)					int name(void);	\
												int name##_kernel_handler(thread_t *thr)

#define KERNEL_SYSCALL1(name, arg1)				int name(arg1);	\
												int name##_kernel_handler(thread_t *thr, arg1)
												
												
#define KERNEL_SYSCALL2(name, arg1, arg2)		int name(arg1, arg2);	\
												int name##_kernel_handler(thread_t *thr, arg1, arg2)
														
														
#define KERNEL_SYSCALL3(name, arg1, arg2, arg3)	int name(arg1, arg2, arg3); \
												int name##_kernel_handler(thread_t *thr, arg1, arg2, arg3)
												
#define KERNEL_SYSCALL4(name, arg1, arg2, arg3, arg4)	int name(arg1, arg2, arg3, arg4); \
												int name##_kernel_handler(thread_t *thr, arg1, arg2, arg3, arg4)
												
												
#define KERNEL_SYSCALL5(name, arg1, arg2, arg3, arg4, arg5)	int name(arg1, arg2, arg3, arg4, arg5); \
												int name##_kernel_handler(thread_t *thr, arg1, arg2, arg3, arg4, arg5)
												
/* generic results */												
#define SYSCALL_RESULT_OK				0
#define SYSCALL_RESULT_ERROR			-1
#define SYSCALL_RESULT_BAD_PARAM		-2
#define SYSCALL_RESULT_NOT_FOUND		-3

/* specific results */
#define SYSCALL_RESULT_PIPE_EMPTY		-1000
#define SYSCALL_RESULT_PIPE_FULL		-1001




#endif
