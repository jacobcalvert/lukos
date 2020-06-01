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

typedef struct
{
	char *app_key;
	size_t in;
	size_t out;
	size_t err;

}termsrv_mux_record_t;

typedef struct termsrv_mr_list_node
{
	struct termsrv_mr_list_node *next;
	termsrv_mux_record_t record;
}termsrv_mr_list_node_t;

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
static size_t stdout_message_buffer_index = 0;

static termsrv_mr_list_node_t* MUX_LIST = NULL;

static void mux_list_append(termsrv_mux_record_t *rcd);

static cli_t *cli;
static cli_node_t * build_root(void);
static int info_processes(cli_t *intf, cli_list_t *wildcards, void *user);
static int attach_N(cli_t *intf, cli_list_t *wildcards, void *user);
static int CLI_MODE = 1;
static termsrv_mux_record_t* MUX = NULL;

static void termsrv_aux_thread(void*arg);
static void termsrv_listen_thread(void*arg);


static cli_options_t cli_opts = {

	.prompt = "lukos-termsrv>>",
	.rx_func = rx_char,
	.tx_func = tx_char

};

static thread_info_t threads[2] = {
	{
		.name = "terminal-server-aux",
		.entry = termsrv_aux_thread,
		.arg = NULL,
		.stack_size = 0x8000,
		.priority = 60
	
	},
	{
		.name = "terminal-server-listen",
		.entry = termsrv_listen_thread,
		.arg = NULL,
		.stack_size = 0x8000,
		.priority = 60
	
	}


};


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
	
	char buf[256];
	size_t sz;
	libc_thread_start(&threads[0]);
	libc_thread_start(&threads[1]);
	while(1)
	{
		if(CLI_MODE)
		{
			cli_loop(cli);
		}
		else
		{
			/* pend on read, write when got */
			while(syscall_ipc_pipe_read(stdin_pipe_id, buf, &sz) == SYSCALL_RESULT_PIPE_EMPTY);
			for(size_t i = 0; i < sz; i++)
			{
				if(buf[i] == '\003') /* CTRL-C or ETX (end-of-text) */
				{
					CLI_MODE = 1; 
					sz = i; /* send everything up to this point, but not the  ETX */
				}
			}
			while(syscall_ipc_pipe_write(MUX->in, buf, sz) == SYSCALL_RESULT_PIPE_FULL);
		}
	}
	
}
void termsrv_listen_thread(void*arg)
{
	size_t request_pipe;
	char name[64];
	char reply_channel_name[64+64];
	char stdx_channel_name[64+64];
	size_t reply_channel, in, out, err;
	size_t len;
	termsrv_mux_record_t rec;
	

	syscall_ipc_pipe_create("terminal-server/request", sizeof(char)*64, 64, 0);
	syscall_ipc_pipe_id_get("terminal-server/request", &request_pipe);
	while(1)
	{
		/* listen to request */
		while(syscall_ipc_pipe_read(request_pipe, name, &len) == SYSCALL_RESULT_PIPE_EMPTY);
		/* create pipes */
		/* in, out, err */
		snprintf(stdx_channel_name, sizeof(stdx_channel_name), "terminal-server/%s/stdin", name);
		syscall_ipc_pipe_create(stdx_channel_name, stdin_message_size, stdin_message_depth, 0);
		syscall_ipc_pipe_id_get(stdx_channel_name, &in);
		
		/*snprintf(stdx_channel_name, sizeof(stdx_channel_name), "terminal-server/%s/stderr", name);
		syscall_ipc_pipe_create(stdx_channel_name, stderr_message_size, stderr_message_depth, 0);
		syscall_ipc_pipe_id_get(stdx_channel_name, &err);*/
		
		snprintf(stdx_channel_name, sizeof(stdx_channel_name), "terminal-server/%s/stdout", name);
		syscall_ipc_pipe_create(stdx_channel_name, stdout_message_size, stdout_message_depth, 0);
		syscall_ipc_pipe_id_get(stdx_channel_name, &out);
		rec.in = in;
		rec.out = out;
		rec.err = 0;
		rec.app_key = name;
		mux_list_append(&rec);
		
		
		/* reply with IDs */
		snprintf(reply_channel_name, sizeof(reply_channel_name), "terminal-server/%s/reply",name);
		syscall_ipc_pipe_id_get(reply_channel_name, &reply_channel);
		
		
		size_t pipes[3] = {in, out, 0};
		while(syscall_ipc_pipe_write(reply_channel, pipes, sizeof(pipes)) == SYSCALL_RESULT_PIPE_FULL);
		
	}

}
void termsrv_aux_thread(void*arg)
{
	char buf[256];
	size_t sz;
	while(1)
	{
		
		if(CLI_MODE)
		{
			syscall_schedule_sleep_ticks(10);
		}
		else
		{
			/* pend on read, write when got */
			while(syscall_ipc_pipe_read(MUX->out, buf, &sz) == SYSCALL_RESULT_PIPE_EMPTY);
			while(syscall_ipc_pipe_write(stdout_pipe_id, buf, sz) == SYSCALL_RESULT_PIPE_FULL);
			
		}
	
	}

}

cli_node_t * build_root()
{
	cli_node_t *root = cli_node_create(NULL, NULL, NULL);
	cli_node_t *info = cli_node_create("info", "get information about the system", NULL);
	cli_node_t *attach = cli_node_create("attach", "attach to a running process", NULL);
	cli_node_t *test = cli_node_create("test", "execute test procedures", NULL);
	cli_node_add_child(info, cli_node_create("processes", "show information about the processes served", info_processes));
	cli_node_add_child(attach, cli_node_create(CLI_MATCH_ANY, "attach to running process with app key [app_key]", attach_N));
	cli_node_add_child(root, info);
	cli_node_add_child(root, test);
	cli_node_add_child(root, attach);
	return root;
}

int info_processes(cli_t *intf, cli_list_t *wildcards, void *user)
{
	cli_printf(intf, "\r\n|-------------------------------------------------------------------------|\r\n");
	cli_printf(intf, "|------------------------------- Process Info ----------------------------|\r\n");
	cli_printf(intf, "|-------------------------------------------------------------------------|\r\n");
	cli_printf(intf, "| %-32s | %-10s | %-10s | %-10s |\r\n", "App Key", "stdin", "stdout", "stderr");
	cli_printf(intf, "|-------------------------------------------------------------------------|\r\n");
	cli_printf(intf, "| %-32s | %-10lu | %-10lu | %-10lu |\r\n", "I/O Server",stdin_pipe_id ,stdout_pipe_id , stderr_pipe_id);
	
	termsrv_mr_list_node_t*p = MUX_LIST;
	while(p != NULL)
	{
		cli_printf(intf, "| %-32s | %-10lu | %-10lu | %-10lu |\r\n", p->record.app_key ,p->record.in ,p->record.out , p->record.err);
		p = p->next;
	}
	
	cli_printf(intf, "|-------------------------------------------------------------------------|\r\n\r\n");
	return 0;
}

int attach_N(cli_t *intf, cli_list_t *wildcards, void *user)
{
	char *app_key;
	if(cli_list_item_get(wildcards, &app_key))
	{
		termsrv_mr_list_node_t*p = MUX_LIST;
		MUX = NULL;
		while(p != NULL)
		{
			if(strcmp(p->record.app_key, app_key) == 0)
			{
				MUX = &p->record;
				break;
			}
			p = p->next;
		}
		if(MUX != NULL)
		{
		
			cli_printf(intf, "\r\n -- attaching to %s | use CTRL-C to detach --\r\n", app_key);
			CLI_MODE = 0;
			
		}
		else
		{
			cli_printf(intf, "\r\n -- did not find %s process --\r\n", app_key);
		}
		
	}
	return 0;
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
	
	if(stdout_message_buffer_index)
	{
		
		while(syscall_ipc_pipe_write(stdout_pipe_id, stdout_message_buffer, stdout_message_buffer_index) == SYSCALL_RESULT_PIPE_FULL);
		stdout_message_buffer_index = 0;
		
	}
	
	return true;
}

void mux_list_append(termsrv_mux_record_t *rcd)
{
	termsrv_mr_list_node_t **pMRLn = &MUX_LIST;
	while(*pMRLn != NULL)
	{
		pMRLn = &(*pMRLn)->next;
	}
	
	*pMRLn = (termsrv_mr_list_node_t*)malloc(sizeof(termsrv_mr_list_node_t));
	
	(*pMRLn)->record.in = rcd->in;
	(*pMRLn)->record.out = rcd->out;
	(*pMRLn)->record.err = rcd->err;
	(*pMRLn)->record.app_key = (char*) malloc(strlen(rcd->app_key)+1);
	memset((*pMRLn)->record.app_key, 0, strlen(rcd->app_key)+1);
	strncpy((*pMRLn)->record.app_key, rcd->app_key, strlen(rcd->app_key));

}
