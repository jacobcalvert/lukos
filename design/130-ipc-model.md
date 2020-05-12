# LuKOS Kernel Functions - Inter-process Communication Model
The IPC Model defines several methods of IPC for message passing, synchronization, etc. This IPC is managed in the kernel by an Inter-Process Communication Manager (IPCM).

## Concepts
### Pipes
Pipes system-wide data busses which can be read from and written to by referencing its ID. The messages which go into a pipe or are read from a pipe have the message boundaries preserved (i.e., each message carries its data and length). Pipes can be thought of as a queue of messages of size S bytes, where the queue length is L messages. When a message is written to the pipe, it is done so atomically. Readers of the pipe will be blocked on a read call until the pipe has some data. Writers to the pipe will either block on a full pipe, or the oldest message discarded and the new one written, depending on configuration. When a thread is blocked on a read or write, that thread will be notified and woken up when it has no more blocking conditions.  

### Pipe Usage
Pipes can be used in a multitude of ways, but here are a few ways they are useful.

#### Blackboard
To keep the latest data available all the time, create a pipe with only one message deep, and configure to discard last on full. When the producer writes to the pipe, the reader will always see the freshest data. 

#### Token-based Semaphore
To use a pipe as a Process/Thread synchronization mechanism, create a pipe of depth __N__, where N is the semaphore resource count. Then either prefill, or leave empty the pipe. When a consumer wants to access a limited resource, they read an "access token" from the pipe (which blocks until the pipe has something in it) and if they can "acquire" this token, this is effectively acquiring the resource. When done, they can write a token back to the pipe, indicating a release.

#### Message Passing
This is a straightforward way to use the pipe. Use the pipe to move data between processes (there are more efficient ways to move data within the same process) by writing to and reading from a shared pipe.

## Kernel Interface
These are the calls defined for use by the kernel. 
```

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


```

## Architecture Interface
There are no architecture calls for this manager component.
