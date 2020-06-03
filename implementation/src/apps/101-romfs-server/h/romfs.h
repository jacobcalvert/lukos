#ifndef __ROMFS_H__
#define __ROMFS_H__

#include <stdint.h>

#define ROMFS_MAGIC_STRING	"-rom1fs-"

typedef struct __attribute__((packed))
{
	char magic[8];
	uint32_t full_size;
	uint32_t checksum;

}romfs_hdr_t;


typedef struct __attribute__((packed))
{
	uint32_t next_off;
	uint32_t spec_info;
	uint32_t size;
	uint32_t checksum;

}romfs_file_hdr_t;


typedef int (*romfs_file_iter_t)(char *name, void *base, uint32_t length, void *arg);


/**
 * load a ROMFS filesystem and ready for use
 */
romfs_hdr_t *romfs_load(void *base);

/* 
 * iterate the files
 */
void romfs_iterate_files(romfs_hdr_t *hdr, romfs_file_iter_t iter, void *arg);

/*
 * get the file!
 * returns 0 if OK, non-zero otherwise
 */
int romfs_file_find(romfs_hdr_t *hdr, char *name, void **base, uint32_t *len);
#endif
