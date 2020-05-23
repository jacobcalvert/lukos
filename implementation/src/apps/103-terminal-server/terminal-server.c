#include <terminal-server.h>
#include <cli.h>
#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/userspace/schedule.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static bool rx_char(char *c);
static bool tx_char(char c);


static size_t stdin_pipe_id = 0;
static size_t stdout_pipe_id = 0;
static size_t stderr_pipe_id = 0;

static size_t stdin_message_size = 0;
static size_t stdin_message_depth = 0;
static size_t stdin_message_flags = 0;
static char *stdin_message_buffer = NULL;
static size_t stdin_message_buffer_len = 0;
static size_t stdin_message_buffer_index = 0;

static size_t stdout_message_size = 0;
static size_t stdout_message_depth = 0;
static size_t stdout_message_flags = 0;
static char *stdout_message_buffer = NULL;
static size_t stdout_message_buffer_len = 0;
static size_t stdout_message_buffer_index = 0;

static cli_options_t cli_opts = {

	.prompt = "lukos-termsrv>>",
	.rx_func = rx_char,
	.tx_func = tx_char

};

static cli_t *cli;
static cli_node_t * build_root(void);



void terminal_server_init(void)
{
	/* get the in, out, err pipe numbers and info*/
	while(syscall_ipc_pipe_id_get("stdin", &stdin_pipe_id) == SYSCALL_RESULT_NOT_FOUND);
	syscall_ipc_pipe_info_get(stdin_pipe_id, &stdin_message_size, &stdin_message_depth, &stdin_message_flags);
	while(syscall_ipc_pipe_id_get("stdout", &stdout_pipe_id) ==  SYSCALL_RESULT_NOT_FOUND);
	syscall_ipc_pipe_info_get(stdout_pipe_id, &stdout_message_size, &stdout_message_depth, &stdout_message_flags);
	//while(syscall_ipc_pipe_id_get("stderr", &stderr_pipe_id) == SYSCALL_RESULT_NOT_FOUND);
	
	stdin_message_buffer = (char*)malloc(stdin_message_size);
	stdout_message_buffer = (char*)malloc(stdout_message_size);
	
	
	cli_opts.root = build_root();
	cli = cli_create(&cli_opts);
	
	
	
	while(1)
	{
		//while(syscall_ipc_pipe_read(stdin_pipe_id, stdin_message_buffer, &stdin_message_buffer_len) == SYSCALL_RESULT_PIPE_EMPTY);
		//while(syscall_ipc_pipe_write(stdout_pipe_id, stdin_message_buffer, stdin_message_buffer_len) == SYSCALL_RESULT_PIPE_FULL);
		cli_loop(cli);	
	}
	
}

cli_node_t * build_root()
{
	cli_node_t *root = cli_node_create(NULL, NULL, NULL);
	cli_node_t *info = cli_node_create("info", "get information about the system", NULL);
	cli_node_t *test = cli_node_create("test", "execute test procedures", NULL);
	cli_node_add_child(root, info);
	cli_node_add_child(root, test);
	return root;
}

bool rx_char(char *c)
{	
	/* if we have already gotten some buffered data don't try to read */
	if(stdin_message_buffer_len == 0)
	{
		while(syscall_ipc_pipe_read(stdin_pipe_id, stdin_message_buffer, &stdin_message_buffer_len) == SYSCALL_RESULT_PIPE_EMPTY);
		stdin_message_buffer_index = 0;
		return rx_char(c);
	}
	else
	{
		*c = stdin_message_buffer[stdin_message_buffer_index++];
		if(stdin_message_buffer_index == stdin_message_buffer_len)
		{
			stdin_message_buffer_len = 0;
		}
	}
	
	return true;
	
	
}

bool tx_char(char c)
{
	stdout_message_buffer[stdout_message_buffer_index++] = c;
	
	if(stdout_message_buffer_index == stdout_message_buffer_len || c == '\n' || c == '>')
	{
		
		while(syscall_ipc_pipe_write(stdout_pipe_id, stdout_message_buffer, stdout_message_buffer_index) == SYSCALL_RESULT_PIPE_FULL);
		stdout_message_buffer_index = 0;
		
	}
	
	return true;
}
