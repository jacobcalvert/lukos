#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/userspace/schedule.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
	size_t x;
	size_t y;

}dummy_t;

void thread(void *arg);


thread_info_t thread_info = {

	.name = "new-thread",
	.entry = thread,
	.arg = NULL,
	.stack_size = 0x8000,
	.priority = 254
};


void thread(void *arg)
{
	size_t pipe = (size_t) arg;
	size_t len;
	dummy_t data;
	while(1)
	{
		syscall_ipc_pipe_read(pipe, (void*)&data, &len);
	}
	
}


int main(void)
{
	dummy_t data = {93, 94};
	syscall_ipc_pipe_create("dummy-pipe", sizeof(dummy_t), 60, 0);
	
	
	size_t dummy_pipe = 0;
	
	if(syscall_ipc_pipe_id_get("dummy-pipe", &dummy_pipe) != 0)
	{
		while(1);
	
	}
	thread_info.arg = (void*)dummy_pipe;
	syscall_schedule_thread_create(&thread_info);
	while(1)
	{
	
		syscall_ipc_pipe_write(dummy_pipe, (void*)&data, sizeof(dummy_t));
		data.x++;
	}
	
	while(1);

}
