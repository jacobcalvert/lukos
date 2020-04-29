/**
 * @file fdtlib.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date February 25, 2020
 * @brief This file declares the interface to the FDT library
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
#ifndef __FDTLIB_H__
#define __FDTLIB_H__

#include <stdlib.h>
#include <stdint.h>

#define FDT_OK					0
#define FDT_ERROR				-1
#define FDT_BAD_MAGIC 			-2



typedef int (*fdtlib_match_callback_t)(char *path, void *arg);
typedef int (*fdtlib_prop_callback_t)(char *path, void *arg, char*propname, void*propaddr, size_t proplen);


/**
 * initialize the fdtlib with a particular blob
 * @param fdtaddr	the start of the blob
 * @return FDT_OK or and error
 */
int fdtlib_init(void *fdtaddr);

/**
 * walk the fdt blob and try to match a key to a value in a node
 * calls the callback when a match is found, if callback returns
 * non-zero we will continue searching, if zero we will stop
 * @param propkey 	the property key
 * @param matchval	the matching value
 * @param callback	the callback function
 * @param arg		passed on to callback, for user usage
 * @return number of matches or ERROR
 */
int fdtlib_find_by_prop(char *propkey, char *matchval, fdtlib_match_callback_t callback, void *arg);

/**
 * walk the fdt blob and try to match phandle in a node
 * calls the callback when a match is found, if callback returns
 * non-zero we will continue searching, if zero we will stop
 * @param phandle	the phandle to find
 * @param callback	the callback function
 * @param arg		passed on to callback, for user usage
 * @return number of matches or ERROR
 */
int fdtlib_find_by_phandle(uint32_t phandle, fdtlib_match_callback_t callback, void *arg);


/**
 * go to the node specified by path and call the callback for each property
 * if callback returns > 0 we will continue processing, otherwise we will stop
 * @param path 		the node path
 * @param arg		user are to pass to callback
 * @param callback	the callback
 * @return number of properties processed or ERROR
 */
int fdtlib_get_props_by_path(char *path, void *arg, fdtlib_prop_callback_t callback);

/**
 * check if a node has a property
 * @param path		the node path
 * @param propname	the property key
 * @return 1 if it does, 0 otherwise
 */
int fdtlib_has_prop(char *path, char *propname);

/**
 * return the address of the property at the given path
 * @param path 		the node path
 * @param propname	the property name
 * @return the address of the property or NULL if not found
 */
void *fdtlib_get_prop(char *path, char *propname);

/**
 * convert and return a uint32_t at the given address
 * @param at	the at address
 * @return the converted value
 */
uint32_t fdtlib_conv_u32(void *at);

/**
 * convert and return a uint64_t at the given address
 * @param at	the at address
 * @return the converted value
 */
uint64_t fdtlib_conv_u64(void *at);


#endif
