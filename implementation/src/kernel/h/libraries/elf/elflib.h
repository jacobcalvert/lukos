/**
 * @file elflib.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date May 01, 2020
 * @brief This file defines the interface to the ELF file handling library.
 * 
 * This file's functions can be used to load an ELF file into
 * an address space with appropriate permissions. This allows
 * the kernel to ready an application for execution.
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
#ifndef __ELFLIB_H__
#define __ELFLIB_H__

#include <managers/virtual-memory-manager.h>


/**
 * init the ELF library
 */
void elflib_init(void);

/**
 * load an ELF file into the given Address Space and return the VA of the entry point in the Address Space
 * @param vakernel		the kernel address for the start of the ELF
 * @param as			the destination address space
 * @param entry			the output entry address
 * @return 0 if ELF is valid non-zero if failure to load
 */
int elflib_binary_load(void *vakernel, address_space_t *as, void **entry);


#endif 
