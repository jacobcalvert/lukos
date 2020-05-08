/**
 * @file ipc-manager.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date May 04, 2020
 * @brief This file defines the interface into the Inter-Process Communication Manager (IPCM).
 * 
 * Copyright (c) 2020 Jacob Calvert
 * All rights reserved. 
 *
 * This file is subject to the terms and conditions
 * defined in 'LICENSE.txt' provided with this source
 * code package. 
 * 
 */
#ifndef __LUKOS_IPCM__
#define __LUKOS_IPCM__
#include <stddef.h>
#include <stdint.h>

#include <managers/virtual-memory-manager.h>

#define IPC_PIPE_FLAG_STS_EMPTY				(1<<0)
#define IPC_PIPE_FLAG_CFG_BLOCK_ON_FULL		(1<<16)
#define IPC_PIPE_FLAG_CFG_MODE_UPDATE		(1<<17)

typedef struct 
{
	char *name;					/**< string name, useful for well-known services */
	size_t id;					/**< id, useful for automated pipe bringup */
	size_t message_size;		/**< size of a single message in this pipe */
	size_t max_messages;		/**< max number of messages to queue */
	
	void *queue[];				/**< memory to store the queue */

	size_t flags;				/**< configuration and status flags */
	
	address_space_t *reader;	/**< the AS of the reader for sanity checking */
	address_space_t *writer;	/**< the AS of the writer */
	

}ipc_pipe_t;

/**
 * init the IPC manager
 */
void ipcm_init(void);


ipc_pipe_t * ipcm_pipe_create(char *name, size_t msg_size, size_t max_msgs);


#endif
