#include <interfaces/userspace/interrupt.h>
#include <interfaces/userspace/memory.h>
#include <interfaces/userspace/device.h>
#include <interfaces/userspace/ipc.h>
#include <interfaces/userspace/schedule.h>
#include <stddef.h>
#include <stdbool.h>

void interrupt_handler(size_t irqno)
{

	syscall_interrupt_complete();
}

typedef struct
{
	 uint32_t DR;
	 uint32_t SREC;
	 uint32_t resvd[4];
	 uint32_t FR;
	 uint32_t resvd1;
	 uint32_t ILPR;
	 uint32_t IBRD;
	 uint32_t FBRD;
	 uint32_t LCR_H;
	 uint32_t CR;
	 uint32_t IFLS;
	 uint32_t IMSC;
	 uint32_t MSC;
	 uint32_t RIS;
	 uint32_t ICR;
	 uint32_t DMACR;

}PL011_regs_t;

typedef struct
{
	volatile PL011_regs_t *regs;
	uint64_t reg_size;
	uint64_t baud;
	uint64_t clock;
	char *uuid;

}PL011_t;

static bool pl011_initialize_global(PL011_t *config);
static bool pl011_open(void* cfg, char *opts);
static int pl011_write(void *cfg, char *buf, size_t sz);
static int pl011_read(void *cfg, char *buf, size_t sz);
static bool pl011_close(void *cfg);
static size_t pl011_debug_show(void *cfg, char *buf, size_t buf_len, int lvl);
static char *pl011_inst_uuid_get(void *cfg);
static void pl011_baud_set(PL011_t *cfg);


bool pl011_open(void* cfg, char *opts)
{

	return true;

}

int pl011_write(void *cfg, char *buf, size_t sz)
{
	char *p = buf;
	PL011_t *c =  (PL011_t*)cfg;
	int count = 0;
	while(count < sz)
	{
		while(c->regs->FR & (1<<3));
		c->regs->DR = (p[count] & 0x0FF);
		count++;
	}
	return count;
}
int pl011_read(void *cfg, char *buf, size_t sz)
{
	char *p = buf;
	PL011_t *c =  (PL011_t*)cfg;
	size_t count = 0;
	while(count < sz)
	{
		while( (c->regs->FR & (1<<6)) == 0);
		*p++ = (c->regs->DR & 0xFF);
		count++;
	}
	return (int)count;
}
bool pl011_close(void *cfg)
{
	return true;
}


void pl011_baud_set(PL011_t *cfg)
{
	uint32_t ibaud = cfg->clock/(16*cfg->baud);
	float fbaud_f = (float)cfg->clock/((float)16.0*(float)cfg->baud);
	fbaud_f -= ibaud;
	fbaud_f*=64;
	fbaud_f+= 0.5;
	uint32_t fbaud = (uint32_t)fbaud_f;

	cfg->regs->IBRD = ibaud;
	cfg->regs->FBRD = fbaud;
}


bool pl011_initialize_global(PL011_t *config)
{
	pl011_baud_set(config);
	
	config->regs->LCR_H |= ( (1<<5) | (1<<6) ); /* 8n1 */
	config->regs->CR |= ( (1<<8) | (1<<9) ); /* TX and RX enabled */
	

	config->regs->CR |= 1; /* enable */
	return true;
}


#define STDIO_PIPE		"stdio"
#define STDER_PIPE		"stderr"
#define STDIN_PIPE		"stdin"

#define BUFFER_SIZE		256
#define NO_BUFFERS		10


void main(int argc, char **argv)
{
	PL011_t driver;
	size_t stdio_pipe = 0;
	size_t stderr_pipe = 0;
	size_t stdin_pipe = 0;
	char buf[BUFFER_SIZE];
	size_t len = 0;
	driver.reg_size = 0x1000;
	driver.baud = 115200U;
	driver.clock = 24000000U;
	
	
	
	syscall_interrupt_attach(2, interrupt_handler);
	
	syscall_device_alloc((void*)0x9000000, driver.reg_size, (void**) &driver.regs);
	
	pl011_initialize_global(&driver);
	
	
	syscall_ipc_pipe_create(STDIO_PIPE, BUFFER_SIZE, NO_BUFFERS, 0);
	syscall_ipc_pipe_id_get(STDIO_PIPE, &stdio_pipe);
	
	syscall_ipc_pipe_write(stdio_pipe, "hello", 5);
	
	while(1)
	{
		syscall_ipc_pipe_read(stdio_pipe, buf, &len);
		buf[len] = '\0';
		pl011_write((void*)&driver, buf, len);
	}

}


