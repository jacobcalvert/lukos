#include <userspace/syscall_numbers.h>


/**
 * create a pipe
 * @param name		the string name of the pipe for named pipes
 * @param msg_size	the message size of a single message in the pipe
 * @param max_msgs	the max number of messages this pipe should handle
 * @param flags		the configuration of this pipe
 */
.global syscall_ipc_pipe_create
syscall_ipc_pipe_create:
	mov x4, x3
	mov x3, x2
	mov x2, x1
	mov x1, x0
	mov x0, #SYSCALL_IPC_PIPE_CREATE
	svc #0
	ret


/**
 * get the pipe's id by name
 * @param name		the pipe name
 * @param id		out - the retrieved ID
 */

.global syscall_ipc_pipe_id_get
syscall_ipc_pipe_id_get:
	mov x2, x1
	mov x1, x0
	mov x0, #SYSCALL_IPC_PIPE_ID_GET
	svc #0
	ret
	

/**
 * send some data to this pipe
 * @param id		the pipe's id
 * @param msg		the message to data to write
 * @param len		the length of data in msg
 */
.global syscall_ipc_pipe_write
syscall_ipc_pipe_write:
	mov x3, x2
	mov x2, x1
	mov x1, x0
	mov x0, #SYSCALL_IPC_PIPE_WRITE
	svc #0
	ret
	
.global syscall_ipc_pipe_read
syscall_ipc_pipe_read:
	mov x3, x2
	mov x2, x1
	mov x1, x0
	mov x0, #SYSCALL_IPC_PIPE_READ
	svc #0
	ret
