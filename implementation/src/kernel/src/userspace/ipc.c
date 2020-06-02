#include <interfaces/userspace/ipc.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>
#include <managers/interrupt-manager.h>
#include <managers/ipc-manager.h>
#include <interfaces/os/cpu.h>

int syscall_ipc_pipe_create_kernel_handler(thread_t *thread, char *name, size_t msg_size, size_t max_msgs, size_t flags)
{
	size_t ipcm_flags = 0;
	if(msg_size == 0 || max_msgs == 0)
	{
		return SYSCALL_RESULT_BAD_PARAM;
	}
	if(flags & SYSCALL_IPC_PIPE_FLAG_OVERWRITE_MODE)
	{
		ipcm_flags |= IPC_PIPE_FLAGS_PURGE_ON_FULL;
	}

	return ipcm_pipe_create(name, msg_size, max_msgs, ipcm_flags)?SYSCALL_RESULT_OK:SYSCALL_RESULT_ERROR;
}

int syscall_ipc_pipe_write_kernel_handler(thread_t *thread, size_t id, void *msg, size_t len)
{	
	if(len == 0)
	{
		return SYSCALL_RESULT_BAD_PARAM;
	}
	int result = ipcm_pipe_write(thread, id, msg, len);
	switch(result)
	{
		case IPC_RESULT_PIPE_NOT_FOUND:return SYSCALL_RESULT_NOT_FOUND;
		case IPC_RESULT_MESSAGE_TOO_LARGE: return SYSCALL_RESULT_BAD_PARAM;
		case IPC_RESULT_PIPE_EMPTY: return SYSCALL_RESULT_PIPE_EMPTY;
		case IPC_RESULT_PIPE_FULL: return SYSCALL_RESULT_PIPE_FULL;
		default:return SYSCALL_RESULT_OK;
	}
}

int syscall_ipc_pipe_read_kernel_handler(thread_t *thread, size_t id, void *msg, size_t *len)
{

	int result = ipcm_pipe_read(thread, id, msg, len);
	switch(result)
	{
		case IPC_RESULT_PIPE_NOT_FOUND:return SYSCALL_RESULT_NOT_FOUND;
		case IPC_RESULT_MESSAGE_TOO_LARGE: return SYSCALL_RESULT_BAD_PARAM;
		case IPC_RESULT_PIPE_EMPTY: return SYSCALL_RESULT_PIPE_EMPTY;
		case IPC_RESULT_PIPE_FULL: return SYSCALL_RESULT_PIPE_FULL;
		default:return SYSCALL_RESULT_OK;
	}

}
