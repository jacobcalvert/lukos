#include <interfaces/userspace/memory.h>
#include <stddef.h>
void main(int argc, char **argv)
{

	void *mem = (void*)0;
	syscall_memory_alloc(8192, 0x93, &mem);
	while(1)
	{
		
	}


}


