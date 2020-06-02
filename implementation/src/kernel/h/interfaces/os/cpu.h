/**
 * @file cpu.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date June 02, 2020
 * @brief This file defines the cpu functions required by the OS.
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

#ifndef __LUKOS_CPU__
#define __LUKOS_CPU__
#include <stddef.h>

/*
 * return the current running CPU 
 */
size_t cpu_current_get(void);


#endif
