/**
 * @file ipc-manager.h
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @date May 04, 2020
 * @brief This file defines the interface into the Inter-Process Communication Manager (IPCM).
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
#ifndef __LUKOS_IPCM__
#define __LUKOS_IPCM__
#include <stddef.h>
#include <stdint.h>

#include <managers/process-manager.h>
#include <managers/virtual-memory-manager.h>
#include <managers/process-manager.h>

#define IPC_PIPE_FLAGS_PURGE_ON_FULL	(1<<16)			/**< purge the oldest message when set, otherwise return error codes to client */

typedef struct 
{
	void *message;
	size_t len;

}ipc_pipe_message_t;

typedef struct ipc_pipe_rw_list_node
{
	struct ipc_pipe_rw_list_node *next;
	thread_t *thread;

}ipc_pipe_rw_list_node_t;

typedef struct 
{
	ipc_pipe_rw_list_node_t* list;
	uint32_t lock;

}ipc_pipe_rw_list_t;



typedef struct 
{
	char *name;					/**< string name, useful for well-known services */
	size_t id;					/**< id, useful for automated pipe bringup */
	size_t message_size;		/**< size of a single message in this pipe */
	size_t max_messages;		/**< max number of messages to queue */
	
	ipc_pipe_message_t *messages;			/**< memory to store the queue */
	size_t head;				/**< head of the queue */
	size_t tail; 				/**< tail of the queue */

	size_t flags;				/**< configuration and status flags */
	
	ipc_pipe_rw_list_t readers;			/**< the readers*/
	ipc_pipe_rw_list_t writers;			/**< the writers */
	
	uint32_t lock;
	

}ipc_pipe_t;

/**
 * init the IPC manager
 */
void ipcm_init(void);

/**
 * create a pipe
 * @param name		the string name of the pipe for named pipes
 * @param msg_size	the message size of a single message in the pipe
 * @param max_msgs	the max number of messages this pipe should handle
 * @param flags		the configuration of this pipe
 * @return			a pointer to the new pipe
 */
ipc_pipe_t * ipcm_pipe_create(char *name, size_t msg_size, size_t max_msgs, size_t flags);

/** 
 * lookup a pipe by its id
 * @param id		the pipe id
 * @return the pointer to the pipe or NULL
 */
ipc_pipe_t * ipcm_pipe_lookup_by_id(size_t id);

/** 
 * lookup a pipe by its name
 * @param name	the pipe name
 * @return the pointer to the pipe or NULL
 */
ipc_pipe_t * ipcm_pipe_lookup_by_name(char *name);

/**
 * write a message to the pipe
 * @param from		the source thread
 * @param id		the pipe to write to
 * @param vamsg		the VA of the source buffer
 * @param len		the length of the message
 * @return an IPCM_STATUS
 */
int ipcm_pipe_write(thread_t *from, size_t id, void *vamsg, size_t len);

/**
 * read a message from a pipe
 * @param to		the destination thread
 * @param id		the pipe id
 * @param vamsg		the destination buffer VA
 * @param valen		the message length retrieved
 * @return an IPCM_STATUS
 */
int ipcm_pipe_read(thread_t *to, size_t id, void *vamsg, size_t *valen);


#endif
