#ifndef __LUKOS_USERSPACE_IPC__
#define __LUKOS_USERSPACE_IPC__

#include <stddef.h>
#include <interfaces/userspace/macros.h>

#define SYSCALL_IPC_PIPE_FLAG_OVERWRITE_MODE		(1<<0)		/**< use this pipe in FIFO mode where the oldest info is discarded instead of blocking on write */

/**
 * create a pipe
 * @param name		the string name of the pipe for named pipes
 * @param msg_size	the message size of a single message in the pipe
 * @param max_msgs	the max number of messages this pipe should handle
 * @param flags		the configuration of this pipe
 */
KERNEL_SYSCALL4(syscall_ipc_pipe_create, char *name, size_t msg_size, size_t max_msgs, size_t flags);

/**
 * get the pipe's id by name
 * @param name		the pipe name
 * @param id		out - the retrieved ID
 */
KERNEL_SYSCALL2(syscall_ipc_pipe_id_get, char *name, size_t *id);

/**
 * get a pipe's info
 * @param id		the id of the pipe
 * @param msg_size	out - the message size of a single message in the pipe
 * @param max_msgs	out - the max number of messages this pipe should handle
 * @param flags		out - the configuration of this pipe
 */
KERNEL_SYSCALL4(syscall_ipc_pipe_info_get, size_t id, size_t *msg_size, size_t *max_msgs, size_t *flags);

/**
 * send some data to this pipe
 * @param id		the pipe's id
 * @param msg		the message to data to write
 * @param len		the length of data in msg
 */
KERNEL_SYSCALL3(syscall_ipc_pipe_write, size_t id, void *msg, size_t len);


/**
 * read some data from this pipe
 * @param id		the pipe's id
 * @param msg		out - the buffer to put read data in
 * @param len		out - the length of data in msg
 */
KERNEL_SYSCALL3(syscall_ipc_pipe_read, size_t id, void *msg, size_t *len);








	
#endif
