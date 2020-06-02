#include <romfs.h>
#include <string.h>
#include <stdio.h>
#define UPALIGN_16B(n) ( ( ((size_t)n & 0xFFFFFFFFFFFFFFF0) == (size_t)n )?(size_t)n:(((size_t)n & (size_t)0xFFFFFFFFFFFFFFF0) + (size_t)0x10))

#define ENDIAN_BIG				0
#define ENDIAN_LITTLE			1

static int ENDIAN_SETTING = -1;

static void set_endian(void);
static uint32_t be32(uint32_t in);

romfs_hdr_t *romfs_load(void *base)
{
	romfs_hdr_t *hdr = (romfs_hdr_t*)base;
	char *filesystem_name = (char*)(size_t)base + 16;
	if(strncmp(ROMFS_MAGIC_STRING, hdr->magic, 8) != 0)
	{
		return NULL;
	}
	
	printf("romfs-server: romfs filesystem header validated\r\n");
	printf("romfs-server: filesystem name --> %s\r\n", filesystem_name);
	printf("romfs-server: filesystem size --> %dkiB\r\n", be32(hdr->full_size)/1024);
	printf("romfs-server: checksum --> 0x%08x\r\n", be32(hdr->checksum));
	
	return hdr;

}


void romfs_iterate_files(romfs_hdr_t *hdr, romfs_file_iter_t iter, void *arg)
{
	char *end_hdr = (char*)(size_t)hdr + 16;
	while(*end_hdr != '\0') end_hdr++;
	romfs_file_hdr_t *fhdr = (romfs_file_hdr_t*) UPALIGN_16B(end_hdr);
	int done = 0;
	while(!done)
	{
		char *name = (char*)(size_t)fhdr + 16;
		uint32_t size = be32(fhdr->size);
		char *end_name = name;
		uint32_t next_off = (be32(fhdr->next_off) & 0xFFFFFFF0);
		uint32_t attr = (be32(fhdr->next_off) & 0x0F);
		int executable = attr & (1<<3);
		
		while(*end_name != '\0')end_name++;
		
		void *base = (void*)UPALIGN_16B(end_name);
		if(size != 0)
		{
			iter(name, base, size, arg);
		}
		if(next_off == 0)
		{
			done = 1;
		}
		else
		{
			fhdr = (romfs_file_hdr_t*)((size_t)hdr + next_off );
		}
		
	}

	

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
