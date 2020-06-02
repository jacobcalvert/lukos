#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/userspace/schedule.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libc-glue.h>
#include <romfs.h>
#include <cli.h>
#include <log.h>

static bool rx_char(char *c);
static bool tx_char(char c);

static cli_t *cli;
static cli_options_t cli_opts = {

	.prompt = "romfs-server>",
	.rx_func = rx_char,
	.tx_func = tx_char

};
extern size_t file_handles[3];

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
static size_t stdout_message_buffer_index = 0;


void server_start(romfs_hdr_t *hdr);

extern const char *__romfs_data;

int initial_iterator(char *name, void *base, uint32_t length, void *arg);

int fs_list_iter(char *name, void *base, uint32_t length, void *arg);

static int log_show(cli_t *intf, cli_list_t *wildcards, void *user);
static int cmdlog_clear(cli_t *intf, cli_list_t *wildcards, void *user);
static int fs_list(cli_t *intf, cli_list_t *wildcards, void *user);
romfs_hdr_t *hdr;
int main(void *arg)
{
	/* initialize the C library glue code */
	libc_init();
	log_write("romfs-server: application loaded\r\n");
	 hdr = romfs_load((void*)&__romfs_data);
	if(hdr != NULL)
	{
		server_start(hdr);
	}
	
	
	while(1);
}

int initial_iterator(char *name, void *base, uint32_t length, void *arg)
{
	log_write("romfs-server: found file %s @ %p (%.2fkiB)\r\n", name, base, (float)length/1024.0);
	return 1;
}	

bool rx_char(char *c)
{	
	/* if we have already gotten some buffered data don't try to read */
	if(stdin_message_buffer_len == 0)
	{
		while(syscall_ipc_pipe_read(file_handles[0], stdin_message_buffer, &stdin_message_buffer_len) == SYSCALL_RESULT_PIPE_EMPTY);
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
	
	if(stdout_message_buffer_index)
	{
		
		while(syscall_ipc_pipe_write(file_handles[1], stdout_message_buffer, stdout_message_buffer_index) == SYSCALL_RESULT_PIPE_FULL);
		stdout_message_buffer_index = 0;
		
	}
	
	return true;
}

cli_node_t * build_root()
{
	cli_node_t *root = cli_node_create(NULL, NULL, NULL);
	
	cli_node_t *log = cli_node_create("log", "perform actions on the log", NULL);
	cli_node_add_child(root, log);
	cli_node_add_child(log, cli_node_create("show", "show the log", log_show));
	cli_node_add_child(log, cli_node_create("clear", "clear the log", cmdlog_clear));
	
	cli_node_t *fs = cli_node_create("fs", "perform filesystem actions", NULL);
	cli_node_add_child(root, fs);
	cli_node_add_child(fs, cli_node_create("list", "list the files in the filesystem", fs_list));

	return root;
}

int log_show(cli_t *intf, cli_list_t *wildcards, void *user)
{
	char *log = log_get();
	size_t log_size = strlen(log);
	cli_puts(intf, "\r\n");
	cli_puts(intf, log);
	cli_puts(intf, "\r\n");
	return 0;
}
int cmdlog_clear(cli_t *intf, cli_list_t *wildcards, void *user)
{
	log_clear();
	return 0;
}
int fs_list_iter(char *name, void *base, uint32_t length, void *arg)
{
	cli_printf((cli_t*)arg, "\r\n%-32s - %.02fkB", name, (float)length/1024.0);
	return 1;
}
int fs_list(cli_t *intf, cli_list_t *wildcards, void *user)
{
	romfs_iterate_files(hdr, fs_list_iter, intf);
	return 0;
}
void server_start(romfs_hdr_t *hdr)
{
	syscall_ipc_pipe_info_get(file_handles[0], &stdin_message_size, &stdin_message_depth, &stdin_message_flags);
	syscall_ipc_pipe_info_get(file_handles[1], &stdout_message_size, &stdout_message_depth, &stdout_message_flags);
	stdin_message_buffer = (char*)malloc(stdin_message_size);
	stdout_message_buffer = (char*)malloc(stdout_message_size);
	cli_opts.root = build_root();
	cli = cli_create(&cli_opts);
	
	romfs_iterate_files(hdr, initial_iterator, NULL);
	while(1)
	{
		cli_loop(cli);
	}
}
