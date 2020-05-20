#include <romfs.h>
#include <string.h>

#define UPALIGN_16B(n) ( ( ((size_t)n & 0xFFFFFFFFFFFFFFF0) == (size_t)n )?(size_t)n:(((size_t)n & (size_t)0xFFFFFFFFFFFFFFF0) + (size_t)0x10))

#define ENDIAN_BIG				0
#define ENDIAN_LITTLE			1

static int ENDIAN_SETTING = -1;

static void set_endian(void);
static uint32_t be32(uint32_t in);

romfs_hdr_t *romfs_load(void *base)
{
	romfs_hdr_t *hdr = (romfs_hdr_t*)base;
	if(strncmp(ROMFS_MAGIC_STRING, hdr->magic, 8) != 0)
	{
		return NULL;
	}
	
	
	return hdr;

}


void romfs_iterate_files(romfs_hdr_t *hdr, romfs_file_iter_t iter)
{
	char *end_hdr = hdr->name;
	while(*end_hdr != '\0') end_hdr++;
	romfs_file_hdr_t *fhdr = (romfs_file_hdr_t*) UPALIGN_16B(end_hdr);
	
	while(fhdr->next_off)
	{
		char *name = fhdr->name;
		uint32_t size = be32(fhdr->size);
		char *end_name = name;
		while(*end_name != '\0')end_name++;
		
		void *base = (void*)UPALIGN_16B(end_name);
		iter(name, base, size);
		
		fhdr = (romfs_file_hdr_t*)((size_t)fhdr + be32(fhdr->next_off & 0xFFFFFFF0));
		
	}
	
	char *name = fhdr->name;
	uint32_t size = be32(fhdr->size);
	char *end_name = name;
	while(*end_name != '\0')end_name++;

	void *base = (void*)UPALIGN_16B(end_name);
	iter(name, base, size);

	

}

void set_endian(void)
{
	short int test_val = 0x0001;
	char *test_byte = (char*)&test_val;
	ENDIAN_SETTING = test_byte[0]?ENDIAN_LITTLE:ENDIAN_BIG;
}

uint32_t be32(uint32_t in)
{
	if(ENDIAN_SETTING == -1)
	{
		set_endian();
		return be32(in);
	}
	else if(ENDIAN_SETTING == ENDIAN_LITTLE)
	{
		return ((in>>24)&0x000000ff) |
		                ((in<<8)&0x00ff0000) |
		                ((in>>8)&0x0000ff00) |
		                ((in<<24)&0xff000000);

	}
	return in;
}
