# tools and options



CROSS_COMPILE=aarch64-none-elf

CC=$(CROSS_COMPILE)-gcc
CXX=$(CROSS_COMPILE)-g++
AS=$(CROSS_COMPILE)-gcc
LD=$(CROSS_COMPILE)-gcc
OBJCOPY=$(CROSS_COMPILE)-objcopy
OBJDUMP=$(CROSS_COMPILE)-objdump
SIZE=$(CROSS_COMPILE)-size
GDB=$(CROSS_COMPILE)-gdb


TERMINAL=x-terminal-emulator

ifdef APP_NAME
	MAP_NAME=$(APP_NAME)
else
	MAP_NAME=kernel
endif

CC_OPTS= -Wall -O0 -ffreestanding -nostdlib -nostartfiles -std=c11 -g -mcpu=$(CPU)
CXX_OPTS= $(CC_OPTS)
AS_OPTS= -g
LD_OPTS= $(CC_OPTS) -Wl,-Map=$(BUILD_DIR)/$(MAP_NAME).map -Wl,-cref  -Wl,--gc-sections -g

