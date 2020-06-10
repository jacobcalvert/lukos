#include <cli.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


#define CLI_CLEAR_LINE 			"\33[2K\r"
#define CLI_CURSOR_MOVE_LEFT	"\33[1D"
#define CLI_CURSOR_MOVE_RIGHT	"\33[1C"


struct cli_node
{
	cli_node_t *children;
	cli_node_t *next;

	char *match_string;
	char *help_string;
	cli_user_func func;
};

struct cli
{
	cli_poll_rx rx;
	cli_tx tx;
	cli_node_t *root;

	char current_line[CLI_MAX_LINE];
	unsigned int current_line_pointer;
	cli_list_t *wildcards;
	cli_list_t *history;
	char *prompt;

	unsigned int max_history;

	void *cookie;
};

struct cli_list_node
{
	struct cli_list_node *next, *prev;
	char *value;
};

struct cli_list
{
	struct cli_list_node *head, *current;
	unsigned int size;

};

static cli_node_t* cli_parse_down(cli_t *cli);


static void cli_end_user_func(cli_t *cli);

static char *cli_token_get(char *str, char *token);

static bool cli_is_number(char *token);
static bool cli_is_string(char *token);

static void cli_print_help(cli_t *cli, cli_node_t *node);


static void recursive_free_node(cli_node_t *node);


static cli_list_t *cli_list_create();
static void cli_list_add(cli_list_t* list, char *val);
static void cli_list_destroy(cli_list_t *list);
static bool cli_list_prev(cli_list_t *list);


static void cli_history_add(cli_t *cli, char*val);

static int cli_help(cli_t *intf, cli_list_t*wildcards,  void *user);


cli_t * cli_create(cli_options_t *opts)
{
	cli_t *intf = (cli_t *)malloc(sizeof(cli_t));
	if(intf)
	{
		intf->tx = opts->tx_func;
		intf->rx = opts->rx_func;
		intf->root = opts->root;
		intf->prompt = (char *)malloc(strlen(opts->prompt));
		intf->current_line_pointer = 0;
		strcpy(intf->prompt, opts->prompt);
		cli_puts(intf, intf->prompt);
		intf->max_history = CLI_MAX_HISTORY;
		intf->history = cli_list_create();
		cli_node_add_child(intf->root, cli_node_create("help", "show the help", cli_help));
	}

	return intf;
}

void cli_prompt_set(cli_t *cli, char *prompt)
{
	if(cli)
	{
		free(cli->prompt);
		cli->prompt = (char*) malloc(strlen(prompt) + 1);
		strcpy(cli->prompt, prompt);
	}
}

void cli_destroy(cli_t *cli)
{
	cli_list_destroy(cli->history);
	recursive_free_node(cli->root);
	free(cli->prompt);
	free(cli);

}

void cli_loop(cli_t *cli)
{
	if(cli)
	{
		char c;
		if(cli->rx(&c))
		{
			switch(c)
			{
				case '\r':
				case '\n':
				{
					cli_node_t *node;
					if(strlen(cli->current_line))
					{
						cli->wildcards = cli_list_create();
						node = cli_parse_down(cli);
						cli->wildcards->current = cli->wildcards->head;
						if(node && node->func)
						{
							node->func(cli, cli->wildcards, cli->cookie);
							cli_history_add(cli, cli->current_line);

						}
						else
						{
							cli_puts(cli, "\r\n\tNo user command registered for '");
							cli_puts(cli, cli->current_line);
							cli_puts(cli, "'\r\n");
						}
						cli_list_destroy(cli->wildcards);
					}

					cli_end_user_func(cli);
					break;
				}
				case '\t':
				{

					break;
				}
				case '?':
				{
					cli_node_t *node = cli_parse_down(cli);
					if(node == NULL)node = cli->root;
					cli_print_help(cli, node);
					cli_puts(cli, cli->prompt);
					cli_puts(cli, cli->current_line);
					break;
				}
				case 0x7F: /*delete */
				case '\b':
				{
					if(cli->current_line_pointer != 0)
					{
						cli->current_line_pointer--;
						cli->current_line[cli->current_line_pointer] = '\0';


					}
					cli_puts(cli, CLI_CLEAR_LINE);
					cli_puts(cli, cli->prompt);
					cli_puts(cli, cli->current_line);

					break;
				}
				case 0x1B: /* escape */
				{
					while(!cli->rx(&c)); /*skip the [ */
					while(!cli->rx(&c)); /* c now contains the code */
					switch(c)
					{
						case 'A': /*up */
						{
							char *line;
							if(cli_list_item_get(cli->history, &line))
							{
								cli_list_prev(cli->history);
								strcpy(cli->current_line, line);
								cli_puts(cli, CLI_CLEAR_LINE);
								cli_puts(cli, cli->prompt);
								cli_puts(cli, cli->current_line);
							}


							break;
						}
						case 'B': /*down*/
						{
							char *line;
							cli_list_next(cli->history);
							if(cli_list_item_get(cli->history, &line))
							{

								strcpy(cli->current_line, line);
								cli_puts(cli, CLI_CLEAR_LINE);
								cli_puts(cli, cli->prompt);
								cli_puts(cli, cli->current_line);
							}
							break;
						}
						case 'C': /* right */
						{
							cli_puts(cli, CLI_CURSOR_MOVE_RIGHT);
							break;
						}
						case 'D': /*left*/
						{
							cli_puts(cli, CLI_CURSOR_MOVE_LEFT);
							cli->current_line_pointer--;
							cli->current_line[cli->current_line_pointer] = '\0';
							break;
						}
						default:break;

					}
					break;
				}

				default:
				{
					cli->current_line[cli->current_line_pointer++] = c;
					while(cli->tx == NULL);
					cli->tx(c);
					break;
				}
			}
		}
	}
}

cli_node_t *cli_node_create(char *match_string, char *help_string, cli_user_func user_func)
{
		cli_node_t *node = (cli_node_t*)malloc(sizeof(cli_node_t));
		if(node)
		{
			node->func = user_func;
			node->children = NULL;
			node->next = NULL;
			if(strlen(help_string))
			{
				node->help_string = (char*)malloc(strlen(help_string) + 1);
				strcpy(node->help_string, help_string);
			}
			if(strlen(match_string))
			{
				node->match_string = (char*)malloc(strlen(match_string) + 1);
				strcpy(node->match_string, match_string);
			}

		}
		return node;
}

void cli_printf(cli_t *cli, char *fmt , ...)
{
	char tmp[256];
	va_list lst;
	va_start(lst, fmt);
	vsnprintf(tmp, 256, fmt, lst);
	va_end(lst);
	cli_puts(cli, tmp);
}

bool cli_node_add_child(cli_node_t *parent, cli_node_t *child)
{
	if(parent && child)
	{
		if(parent->children)
		{
			cli_node_t *p = parent->children;
			while(p->next)
			{
				p = p->next;
			}
			p->next = child;
		}
		else
		{
			parent->children = child;
		}
	}
	return false;
}

void cli_puts(cli_t *cli, char *str)
{
	while(*str)
	{
		while(cli->tx == NULL);
		cli->tx(*str++);
	}
}

void cli_end_user_func(cli_t *cli)
{
	cli->current_line_pointer = 0;
	memset(cli->current_line, 0, sizeof(cli->current_line));
	cli_puts(cli, "\r\n");
	cli_puts(cli, cli->prompt);
}

cli_node_t* cli_parse_down(cli_t *cli)
{
	cli_node_t *result = NULL;

	cli_node_t *p = cli->root->children;


	char *line = &cli->current_line[0];
	char token[CLI_MAX_TOKEN];

	token[0] = '\0';
	line = cli_token_get(line, token);

	while(strlen(token) && p)
	{
		if(strcmp(token, p->match_string) == 0)
		{
			/* direc match */
			result = p;
			p = p->children;
			token[0] = '\0';
			memset(token, 0, sizeof(token));
			line = cli_token_get(line, token);

		}
		else if(strcmp(p->match_string,CLI_MATCH_ANY) == 0)
		{
			/* wildcard */
			result = p;
			p = p->children;
			cli_list_add(cli->wildcards, token);

		}
		else if( (strcmp(p->match_string,CLI_MATCH_NUM) == 0) && cli_is_number(token))
		{
			/* number only */
			result = p;
			p = p->children;
			cli_list_add(cli->wildcards, token);
		}
		else if( (strcmp(p->match_string,CLI_MATCH_STR) == 0) && cli_is_string(token))
		{
			result = p;
			p = p->children;
			cli_list_add(cli->wildcards, token);
		}
		else
		{
			p = p->next;
		}



	}


	return result;
}

bool cli_is_number(char *token)
{
	while(*token)
	{
		if(*token < '0' || *token > '9')
		{
			return false;
		}
		token++;
	}
	return true;
}
bool cli_is_string(char *token)
{
	return false;
}

char *cli_token_get(char *str, char *token)
{
	char *p = str;
	char end_char = ' ';
	while(*p == ' ')p++;
	if(*p == '"' || *p == '\'')
	{
		end_char = *p;
		p++;
	}
	while((*p != end_char) && (*p != '\0'))
	{
		*token++ = *p++;
	}

	p++;
	return p;
}
int cli_help(cli_t *intf, cli_list_t*wildcards,  void *user)
{
	cli_print_help(intf, intf->root);
	return 0;
}
void cli_print_help(cli_t *cli, cli_node_t *node)
{
	cli_node_t *p = node->children;
	char kw_buf[32];
	cli_puts(cli, "\r\n");
	while(p)
	{
		snprintf(kw_buf, 32, "\t%-31s", p->match_string);
		cli_puts(cli, kw_buf);
		cli_puts(cli, "- ");
		cli_puts(cli, p->help_string);
		cli_puts(cli, "\r\n");
		p = p->next;
	}

}
bool cli_list_item_get(cli_list_t *list, char **value)
{
	if(list && list->current)
	{
		*value = list->current->value;
		return true;
	}
	return false;
}
bool cli_list_next(cli_list_t *list)
{
	if(list && list->current)
	{
		list->current = list->current->next;
		return true;
	}
	return false;
}
bool cli_list_prev(cli_list_t *list)
{
	if(list)
	{
		if(list->current)list->current = list->current->prev;
		else
		{
			struct cli_list_node *p = list->head;
			while(p->next)p = p->next;
			list->current = p;
		}
		return true;
	}
	return false;
}
void recursive_free_node(cli_node_t *node)
{
	if(node)
	{
		if(node->children)
		{
			recursive_free_node(node->children);
		}
		if(node->next)
		{
			recursive_free_node(node->next);
		}
		free(node->help_string);
		free(node->match_string);
		free(node);
	}
}

cli_list_t *cli_list_create()
{
	cli_list_t * list = (cli_list_t*)malloc(sizeof(cli_list_t));
	list->head = NULL;
	return list;
}
void cli_list_add(cli_list_t* list, char *val)
{
	if(list->head)
	{
		struct cli_list_node * p = list->head;
		while(p->next) p = p->next;
		p->next =  (struct cli_list_node*)malloc(sizeof(struct cli_list_node));
		p->next->prev = p;
		p->next->next = NULL;
		p->next->value = (char *)malloc(strlen(val)+1);
		strcpy(p->next->value, val);
		list->size ++;
	}
	else
	{
		list->head = (struct cli_list_node*)malloc(sizeof(struct cli_list_node));
		list->head->next = NULL;
		list->head->prev = NULL;
		list->head->value = (char *)malloc(strlen(val)+1);
		strcpy(list->head->value, val);
		list->size ++;
	}
}
void recurse_destroy(struct cli_list_node *n)
{
	if(n)
	{
		if(n->next)recurse_destroy(n->next);
		free(n->value);
		free(n);
	}


}

void cli_list_destroy(cli_list_t *list)
{
	recurse_destroy(list->head);
	free(list);
}

void cli_history_add(cli_t *cli, char*val)
{
	struct cli_list_node *p;
	cli_list_add(cli->history, val);

	if(cli->history->size == cli->max_history)
	{
		p = cli->history->head;
		cli->history->head = cli->history->head->next;
		cli->history->head->prev = NULL;
		free(p->value);
		free(p);
		cli->history->size --;

	}
	p = cli->history->head;
	while(p->next) p = p->next;
	cli->history->current = p;
}
