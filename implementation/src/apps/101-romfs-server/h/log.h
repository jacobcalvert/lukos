#ifndef __ROMFS_LOG_H__
#define __ROMFS_LOG_H__

void log_init(void);

void log_write(char *fmt, ...);

char *log_get(void);

void log_clear(void);
#endif
