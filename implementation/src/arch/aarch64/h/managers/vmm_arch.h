#ifndef __AARCH64_VMM_ARCH__
#define __AARCH64_VMM_ARCH__


typedef struct
{
	uint64_t *translation_table;
	
	void * table_space;
	size_t table_space_len;
	
}aarch64_vmm_context_t;


#endif
