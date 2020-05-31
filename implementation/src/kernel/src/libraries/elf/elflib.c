#include <libraries/elf/elflib.h>
#include <managers/virtual-memory-manager.h>
#include <string.h>
#include <stdint.h>

#ifdef __aarch64__
#define THIS_ARCH		0xB7
#endif

#define ENDIAN_BIG				2
#define ENDIAN_LITTLE			1




typedef struct __attribute__((packed))
{
	uint8_t magic[4];		/* 0x7F E L F */
	uint8_t class;			/* 32/64 bit = 1/2 */
	uint8_t endian;			/* little/big = 1/2 */
	uint8_t version;		/* should be 1 */
	uint8_t osabi;			/* don't care */
	uint8_t abiversion;		/* don't care for now */
	uint8_t pad1[7];		/* pad */
	uint16_t type;			/* 2 bytes, type of file */
	uint16_t machine;		/* 2 bytes, type of machine (ARM, etc) */
	uint32_t version1; 		/* should be 1 */
	uint64_t entry;			/* entry point in VA */
	uint64_t phdr_off;		/* program header offset in file */
	uint64_t shdr_off;		/* section header offset in file */
	uint32_t flags;			/* ???? */
	uint16_t hdrsz;			/* this elf header size */
	uint16_t phdrsz;		/* program header size  */
	uint16_t phdrnum;		/* count of program headers */
	uint16_t shdrsz;		/* section header size */
	uint16_t shdrnum;		/* count of section headers */
	uint16_t shdrstrings;	/* index of the section header that contains the section names */
	 

}elf_header_t;

#define ELF_SECTION_TYPE_NULL		0x0
#define ELF_SECTION_TYPE_PROGBITS	0x1
#define ELF_SECTION_TYPE_STRTAB		0x3
#define ELF_SECTION_TYPE_NOBITS		0x8

#define ELF_SECTION_FLAG_WRITEABLE  (1<<0)
#define ELF_SECTION_FLAG_ALLOC		(1<<1)
#define ELF_SECTION_FLAG_EXEC		(1<<2)


typedef struct __attribute__((packed))
{
	uint32_t name;			/* offset into the name section */
	uint32_t type;			/* type of section */
	uint64_t flags;			/* flags */
	uint64_t address;		/* VA */
	uint64_t offset;		/* offset in this file of the data */
	uint64_t size;			/* size of this section */
	uint32_t link;			/* link index (if applicable) */
	uint32_t info;			/* extra info */
	uint64_t align;			/* alignment */
	uint64_t entry_size;	/* size in bytes of each entry if fixed size */

}elf64_section_hdr_t;

#define SHDR_SIZE			sizeof(elf64_section_hdr_t)



static int get_endian(void);

static elf_header_t *elflib_load_internal(void *vakernel);

void elflib_init(void)
{
}


int elflib_binary_load(void *vakernel, address_space_t *as, void **entry)
{
	elf_header_t *elf = elflib_load_internal(vakernel);
	
	if(elf != NULL)
	{
		/* find the section string names */
	/*	elf64_section_hdr_t *section_strings = (elf64_section_hdr_t*)((size_t)elf + (elf->shdr_off) + (elf->shdrsz*elf->shdrstrings)); */
	/*	char *string_names = (char*)((size_t)elf + section_strings->offset); */
		
		/* let's get our sections and allocate them */
		
		for(uint16_t section_index = 0; section_index < elf->shdrnum; section_index++)
		{
			elf64_section_hdr_t *section = (elf64_section_hdr_t*)((size_t)elf + (elf->shdr_off) + (elf->shdrsz*section_index));
			void *data = (void*)((size_t)elf + section->offset);
			if( (section->type == ELF_SECTION_TYPE_PROGBITS) || (section->type == ELF_SECTION_TYPE_NOBITS))
			{
				/* we want to allocate for this section */
				if((section->flags & ELF_SECTION_FLAG_ALLOC) && (section->size))
				{
					address_space_region_prop_t prop = AS_REGION_RX;
					if(section->flags & ELF_SECTION_FLAG_WRITEABLE)
					{
						prop = AS_REGION_RW;
					}
					vmm_address_space_region_create(as, (void*)section->address, section->size, prop);
					vmm_address_space_copy_in(as, data, (void*)section->address, section->size);
				}		
				
				if(section->type == ELF_SECTION_TYPE_NOBITS)
				{
					/* zeroize it */
					char *ptr = (char*)vmm_arch_v2p(as->arch_context, (void*)section->address);
					uint64_t i = 0;
					while(i < section->size)
					{
						ptr[i++] = '\0';
					}
				
				}
			}

			
		
		}
		
		*entry = (void*)elf->entry;
		
		return 0;
		
		 
	}
	return -1;

}


elf_header_t *elflib_load_internal(void *vakernel)
{

	elf_header_t *hdr = (elf_header_t*)vakernel;
	
	/* do all of our sanity checks */
	char magic[4] = {0x7F, 'E', 'L', 'F'};
	
	/* check the magic */
	for(int i = 0; i < 4; i ++)
	{
		if((char)hdr->magic[i] != magic[i])
		{
			return NULL;
		}
	}
	/* must be 64-bit */
	if(hdr->class != 2)
	{
		return NULL;
	}
	
	/* must match our endianness */
	if(hdr->endian != get_endian())
	{
		return NULL;
	}
	
	/* must be ELFv1 */
	if(hdr->version != 1)
	{
		return NULL;
	}
	
	/* must match our arch */
	if(hdr->machine != THIS_ARCH)
	{
		return NULL;
	}
	
	/* OK All Good. */
	return hdr;

}
int get_endian(void)
{
	short int test_val = 0x0001;
	char *test_byte = (char*)&test_val;
	return test_byte[0]?ENDIAN_LITTLE:ENDIAN_BIG;
}

