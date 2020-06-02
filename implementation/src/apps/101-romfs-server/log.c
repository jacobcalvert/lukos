#include <log.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define LOG_BUFFER_SIZE		(64*1024)

static char LOG_BUFFER[LOG_BUFFER_SIZE];
static size_t LOG_BUFFER_INDEX = 0;

void log_init(void)
{
	memset(LOG_BUFFER, 0, LOG_BUFFER_SIZE);
	LOG_BUFFER_INDEX = 0;

}

void log_write(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LOG_BUFFER_INDEX += vsnprintf(&LOG_BUFFER[LOG_BUFFER_INDEX], (LOG_BUFFER_SIZE - LOG_BUFFER_INDEX), fmt, args);
	va_end(args);
}

char *log_get(void)
{
	return &LOG_BUFFER[0];
}

void log_clear(void)
{
	log_init();

}
