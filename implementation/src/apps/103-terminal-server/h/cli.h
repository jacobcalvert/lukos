/**
 * @file cli.h
 * @date Jan 14, 2019
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @brief
 *
 */
#ifndef __cli__
#define __cli__


#include <stdbool.h>


#define CLI_MATCH_ANY	"[*]"
#define CLI_MATCH_NUM	"[n]"
#define CLI_MATCH_STR	"[s]"


#ifndef CLI_MAX_LINE
#define CLI_MAX_LINE	512
#endif

#ifndef CLI_MAX_TOKEN
#define CLI_MAX_TOKEN	32
#endif

#ifndef CLI_MAX_NUM_TOKEN
#define CLI_MAX_NUM_TOKEN	32
#endif

#ifndef CLI_MAX_HISTORY
#define CLI_MAX_HISTORY		32
#endif



struct cli;
typedef struct cli cli_t;

struct cli_node;
typedef struct cli_node cli_node_t;
struct cli_list;
typedef struct cli_list cli_list_t;

typedef bool (*cli_poll_rx)(char *c);
typedef bool (*cli_tx)(char c);
typedef void (*cli_outstream)(char *str);
typedef int (*cli_user_func)(cli_t *intf, cli_list_t *wildcards, void *user);


typedef struct cli_options
{
	cli_poll_rx rx_func;
	cli_tx tx_func;
	cli_node_t *root;

	char *prompt;

}cli_options_t;


/**
 * create a cli interface object
 * @param opts		the intf options
 * @return a cli_t*  pointer or NULL
 */
cli_t * cli_create(cli_options_t *opts);

/**
 * clean up any memory used by CLI
 * @param cli		the interface
 */
void cli_destroy(cli_t *cli);

/**
 * perform work loop on this interface
 * @param cli		the interface
 */
void cli_loop(cli_t *cli);

/**
 * printf on a particular cli output stream
 * @param cli		the interface
 * @param fmt		the formant
 * @param ...		the rest
 *
 */
void cli_printf(cli_t *cli, char *fmt , ...);

/**
 * set the prompt on a given interface
 * @param cli 	interface
 * @param prompt	the prompt
 */
void cli_prompt_set(cli_t *cli, char *prompt);

/**
 * create a cli node
 * @param match_string		the match string can be CLI_MATCH_* macro or can be a direct match
 * @param help_string		a help string to be shown
 * @param user_func			the user function to be called when this is executed
 * @return a pointer to a cli_node_t or NULL
 */
cli_node_t *cli_node_create(char *match_string, char *help_string, cli_user_func user_func);

/**
 * add a child node to the cli parent node
 * @param parent		the parent node
 * @param child			the child node
 * @return success
 */
bool cli_node_add_child(cli_node_t *parent, cli_node_t *child);

/**
 * get the current item in the list
 * @param list	the list
 * @param value	the item
 * @return success
 */
bool cli_list_item_get(cli_list_t *list, char **value);

/**
 * move the list pointer
 * @param list	the list
 * @return true unless EOL then false
 */
bool cli_list_next(cli_list_t *list);

#endif /* __cli__ */
