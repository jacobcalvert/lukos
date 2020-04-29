/**
 * @file fdtlib.c
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date February 25, 2020
 * @brief This file defines the interface to the FDT library
 *
 * The FDT (flattened device tree) is a binary representation
 * of a DTB (device tree blob). This library can parse through
 * this FDT representation to provide the contents to the
 * caller.
 *
 *
 *
 * Copyright (c) 2020 Jacob Calvert
 * All rights reserved.
 *
 * This file is subject to the terms and conditions
 * defined in 'LICENSE.txt' provided with this source
 * code package.
 *
 */
#include <libraries/fdt/fdtlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define fdt_addr_t size_t

typedef struct fdt_header
{
	uint32_t magic;
	uint32_t totalsize;
	uint32_t off_dt_struct;
	uint32_t off_dt_strings;
	uint32_t off_mem_rsvmap;
	uint32_t version;
	uint32_t last_comp_version;
	uint32_t boot_cpuid_phys;
	uint32_t size_dt_strings;
	uint32_t size_dt_struct;
}fdt_header_t;

typedef struct fdt_reserve_entry
{
	uint64_t address;
	uint64_t size;
}fdt_reserve_entry_t;


typedef struct fdt_prop_entry
{
	uint32_t len;
	uint32_t nameoff;
}fdt_prop_entry_t;



#define FDT_HEADER_MAGIC		0xD00DFEED
#define FDT_BEGIN_NODE			0x00000001
#define FDT_END_NODE			0x00000002
#define FDT_PROP				0x00000003
#define FDT_NOP					0x00000004
#define FDT_END					0x00000009


#define ENDIAN_BIG				0
#define ENDIAN_LITTLE			1

static void set_endian(void);
static uint32_t be32(uint32_t in);
static uint64_t be64(uint64_t in);
static void make_path(char *buf, fdt_addr_t *addr_list, size_t no_addr);
static fdt_addr_t up_align(fdt_addr_t ptr, uint8_t align_to);
static fdt_addr_t find_node(char *node_name, fdt_addr_t origbase);
static char * next_token(char *input, char *token);
static int find_props(char *path, fdt_addr_t base, fdtlib_prop_callback_t callback, void *arg);


static int ENDIAN_SETTING = -1;
static fdt_header_t *FDT_HEADER = NULL;


int fdtlib_init(void *fdtaddr)
{
	FDT_HEADER = (fdt_header_t*) fdtaddr;
	if(FDT_HEADER)
	{
		if(be32(FDT_HEADER->magic) == FDT_HEADER_MAGIC)
		{
			return FDT_OK;
		}
		return FDT_BAD_MAGIC;
	}
	return FDT_ERROR;
}

int fdtlib_find_by_phandle(uint32_t phandle, fdtlib_match_callback_t callback, void *arg)
{

	int count = FDT_ERROR;
	char path[512];
	fdt_addr_t base = (fdt_addr_t) be32(FDT_HEADER->off_dt_struct) + (fdt_addr_t) FDT_HEADER + (fdt_addr_t)8; /* +8 skips root node */
	fdt_addr_t struct_end_addr = (fdt_addr_t)base + (fdt_addr_t)be32(FDT_HEADER->size_dt_struct);
	char *name_base = (char *) ( (fdt_addr_t)FDT_HEADER +  (fdt_addr_t)be32(FDT_HEADER->off_dt_strings));
	fdt_addr_t offsets[16];
	size_t offset_index = 0;

	if(callback)
	{
		count = 0;
	}

	while(base < struct_end_addr)
	{
		uint32_t token = *(uint32_t*)base;
		switch(be32(token))
		{
			case FDT_BEGIN_NODE:
			{

				base += sizeof(uint32_t); /* skip begin token */
				offsets[offset_index++] = base;
				base += strlen((char*)base)+1;
				base = up_align(base, 4);
				break;
			};
			case FDT_NOP:
			{
				base += sizeof(uint32_t); /* skip begin token */
				break;
			}
			case FDT_END_NODE:
			{
				base += sizeof(uint32_t); /* skip begin token */
				offset_index--;
				break;
			}
			case FDT_PROP:
			{
				base += sizeof(uint32_t); /* skip prop token */
				fdt_prop_entry_t *prop = (fdt_prop_entry_t*)base;
				size_t len = be32(prop->len);
				base +=  2*sizeof(uint32_t);
				if(strcmp("phandle", &name_base[be32(prop->nameoff)])==0)
				{

					while(len)
					{
						if(be32(*(uint32_t*)base) == phandle)
						{
							/*offsets[offset_index++] = (fdt_addr_t)&name_base[be32(prop->nameoff)]; */
							make_path(path, offsets, offset_index);
							/*offset_index--; */
							count++;
							if(!callback(path, arg))
							{
								return count;
							}

						}
						len -= be32(prop->len);
						base +=  be32(prop->len);

					}
				}
				base += (fdt_addr_t)len;
				base = up_align(base, 4);
				break;
			}
			case FDT_END:
			{
				return count;
			}
		}

	}



	return count;
}

int fdtlib_find_by_prop(char *propkey, char *matchval, fdtlib_match_callback_t callback, void *arg)
{
	int count = FDT_ERROR;
	char path[512];
	fdt_addr_t base = (fdt_addr_t) be32(FDT_HEADER->off_dt_struct) + (fdt_addr_t) FDT_HEADER + (fdt_addr_t)8; /* +8 skips root node */
	fdt_addr_t struct_end_addr = (fdt_addr_t)base + (fdt_addr_t)be32(FDT_HEADER->size_dt_struct);
	char *name_base = (char *) ( (fdt_addr_t)FDT_HEADER +  (fdt_addr_t)be32(FDT_HEADER->off_dt_strings));
	fdt_addr_t offsets[16];
	size_t offset_index = 0;

	if(propkey && matchval && callback)
	{
		count = 0;
	}

	while(base < struct_end_addr)
	{
		uint32_t token = *(uint32_t*)base;
		switch(be32(token))
		{
			case FDT_BEGIN_NODE:
			{

				base += sizeof(uint32_t); /* skip begin token */
				offsets[offset_index++] = base;
				base += strlen((char*)base)+1;
				base = up_align(base, 4);
				break;
			};
			case FDT_NOP:
			{
				base += sizeof(uint32_t); /* skip begin token */
				break;
			}
			case FDT_END_NODE:
			{
				base += sizeof(uint32_t); /* skip begin token */
				offset_index--;
				break;
			}
			case FDT_PROP:
			{
				base += sizeof(uint32_t); /* skip prop token */
				fdt_prop_entry_t *prop = (fdt_prop_entry_t*)base;
				size_t len = be32(prop->len);
				base +=  2*sizeof(uint32_t);
				if(strcmp(propkey, &name_base[be32(prop->nameoff)])==0)
				{

					while(len)
					{
						if(strstr((char*)base, matchval) || (strcmp((char*)base, matchval) == 0))
						{
							/*offsets[offset_index++] = (fdt_addr_t)&name_base[be32(prop->nameoff)]; */
							make_path(path, offsets, offset_index);
							/*offset_index--; */
							count++;
							if(!callback(path, arg))
							{
								return count;
							}

						}
						len -= (strlen((char*)base) + 1);
						base += (fdt_addr_t)(strlen((char*)base) + 1);

					}
				}
				base += (fdt_addr_t)len;
				base = up_align(base, 4);
				break;
			}
			case FDT_END:
			{
				return count;
			}
		}

	}



	return count;
}

int fdtlib_get_props_by_path(char *path, void *arg, fdtlib_prop_callback_t callback)
{
	char token[64];
	char orig_path[256];
	strncpy(orig_path, path, sizeof(path));
	fdt_addr_t base = (fdt_addr_t) be32(FDT_HEADER->off_dt_struct) + (fdt_addr_t) FDT_HEADER + (fdt_addr_t)8; /* +8 skips root node */
	fdt_addr_t move = 0;
	path += 1; /* remove leading slash */

	while(strlen(path))
	{
		path = next_token(path, token);
		move = find_node(token, base);
		if(move == (fdt_addr_t)-1)
		{
			goto fdt_prop_get_exit;
		}
		base += move;

	}
	/* base is now pointing to the properties */
	return find_props(orig_path, base, callback, arg);

fdt_prop_get_exit:
	return FDT_ERROR;
}

struct __getprophelper
{
	char *prop;
	void **propaddr;
	size_t proplen;
};
static int __fdtlib_has_prop_callback(char *path, void *arg, char*propname, void*propaddr, size_t proplen)
{
	struct __getprophelper *helper = (struct __getprophelper*)arg;
	if(strcmp(propname, helper->prop) == 0)
	{
		*(helper->propaddr) = propaddr;
		helper->proplen = proplen;
		return 0;
	}
	return 1;
}
int fdtlib_has_prop(char *path, char *propname)
{
	struct __getprophelper helper;
	helper.prop = propname;
	void *local = NULL;
	helper.propaddr = &local;
	fdtlib_get_props_by_path(path, (void* )&helper, __fdtlib_has_prop_callback);
	return (*helper.propaddr) != NULL;
}

void *fdtlib_get_prop(char *path, char *propname)
{
	struct __getprophelper helper;
	helper.prop = propname;
	void *local = NULL;
	helper.propaddr = &local;
	fdtlib_get_props_by_path(path, (void* )&helper, __fdtlib_has_prop_callback);
	return (*helper.propaddr);
}

size_t fdtlib_get_prop_len(char* path, char *propname)
{
	struct __getprophelper helper;
	helper.prop = propname;
	void *local = NULL;
	helper.propaddr = &local;
	fdtlib_get_props_by_path(path, (void* )&helper, __fdtlib_has_prop_callback);
	return ((*helper.propaddr) != NULL)?helper.proplen:0;
}

uint32_t fdtlib_conv_u32(void *at)
{
	uint32_t *in = (uint32_t *)at;
	return be32(*in);
}
uint64_t fdtlib_conv_u64(void *at)
{
	uint64_t *in = (uint64_t *)at;
	return be64(*in);
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
uint64_t be64(uint64_t in)
{
	if(ENDIAN_SETTING == -1)
	{
		set_endian();
		return be64(in);
	}
	else if(ENDIAN_SETTING == ENDIAN_LITTLE)
	{
		return ((((in) >> 56) & 0x00000000000000FF) | (((in) >> 40) & 0x000000000000FF00) |
				(((in) >> 24) & 0x0000000000FF0000) | (((in) >>  8) & 0x00000000FF000000) |
				(((in) <<  8) & 0x000000FF00000000) | (((in) << 24) & 0x0000FF0000000000) |
				(((in) << 40) & 0x00FF000000000000) | (((in) << 56) & 0xFF00000000000000));

	}
	return in;
}


void make_path(char *buf, fdt_addr_t *addr_list, size_t no_addr)
{
	size_t i = 0, b = 0;


	while(i < no_addr)
	{
		char *tmp = (char *)addr_list[i++];
		buf[b++]='/';
		while(*tmp != '\0')
		{
			buf[b++] = *tmp++;
		}
	}
	buf[b++] = '\0';
}
fdt_addr_t up_align(fdt_addr_t ptr, uint8_t align_to)
{
	uint8_t mod = ptr%align_to;
	if(mod == 0)return ptr;
	uint8_t add = align_to - mod;
	ptr+= add;
	return ptr;
}

char * next_token(char *input, char *token)
{
	char *tmp = input;
	while(*tmp == '/') tmp++; /*strip leading / */

	while(*tmp != '/' && *tmp)
	{
		*token ++ = *tmp++;
	}
	*token = '\0';

	while(*tmp == '/') tmp++; /*strip trailing / */


	return tmp;


}


fdt_addr_t find_node(char *node_name, fdt_addr_t origbase)
{
	fdt_addr_t base = (fdt_addr_t) be32(FDT_HEADER->off_dt_struct) + (fdt_addr_t) FDT_HEADER;
	fdt_addr_t struct_end_addr = (fdt_addr_t)base + (fdt_addr_t)be32(FDT_HEADER->size_dt_struct);
	base = origbase;
	int depth = 0;
	while(base < struct_end_addr)
	{
		uint32_t token = *(uint32_t*) base;
		switch(be32(token))
		{
			case FDT_BEGIN_NODE:
			{
				base += sizeof(uint32_t); /* skip start token */
				depth ++;
				if(strcmp(node_name, (char*)base) == 0 && depth == 1)
				{
					/* this is our node */
					base += strlen((char*)base)+1;
					base = up_align(base, 4);
					return (base-origbase);
				}
				else
				{
					/* scan to end of node */
					base += strlen((char*)base)+1 ; /* size + null */
					base = up_align(base, 4);


				}
				break;
			}
			case FDT_END_NODE:
			{
				base += sizeof(uint32_t); /* skip e token */
				depth--;
				break;
			}
			case FDT_NOP:
			{
				base += sizeof(uint32_t);
				break;
			}
			case FDT_PROP:
			{
				base += sizeof(uint32_t); /* skip prop token */
				fdt_prop_entry_t *prop = (fdt_prop_entry_t*)base;
				base += (fdt_addr_t)be32(prop->len) + 2*sizeof(uint32_t);
				base = up_align(base, 4);
				break;
			}
			case FDT_END:
			{
				base += sizeof(uint32_t); /* skip prop token */
				break;
			}
		}
	}

	return (fdt_addr_t)-1;
}

int find_props(char *path, fdt_addr_t base, fdtlib_prop_callback_t callback, void *arg)
{
	fdt_addr_t struct_end_addr = (fdt_addr_t)FDT_HEADER + (fdt_addr_t)be32(FDT_HEADER->size_dt_struct) + (fdt_addr_t)be32(FDT_HEADER->off_dt_struct);
	int count = 0;
	while(base < struct_end_addr)
	{
		uint32_t token = *(uint32_t*) base;
		switch(be32(token))
		{
			case FDT_BEGIN_NODE:
			{
				return count;
			}
			case FDT_NOP:
			{
				base += sizeof(uint32_t);
				break;
			}
			case FDT_PROP:
			{
				base += sizeof(uint32_t); /* skip prop token */
				fdt_prop_entry_t *prop = (fdt_prop_entry_t*)base;
				char *name_base = (char *) ( (fdt_addr_t)FDT_HEADER +  (fdt_addr_t)be32(FDT_HEADER->off_dt_strings));
				char *name = &name_base[be32(prop->nameoff)];
				count++;
				void *prop_base = (void*)(base + 2*sizeof(uint32_t));
				if(!callback(path, arg, name, prop_base , be32(prop->len)))
				{
					return count;
				}

				base += (fdt_addr_t)be32(prop->len) + 2*sizeof(uint32_t);
				base = up_align(base, 4);
				break;
			}
			case FDT_END_NODE:
			{
				base += sizeof(uint32_t); /* skip e token */
				break;
			}
			case FDT_END:
			{
				base += sizeof(uint32_t); /* skip prop token */
				break;
			}
		}
	}
	return count;
}

