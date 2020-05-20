# tools and options

TOOLCHAIN ?= GNU

ifeq ($(TOOLCHAIN), LLVM)
	
endif

ifeq ($(TOOLCHAIN), GNU)
	
	CC=gcc
	CXX=g++
	AS=gcc
	LD=gcc
	OBJCOPY=objcopy
	OBJDUMP=objdump
	SIZE=size
	GDB=gdb
	
	CC_OPTS= -Wall -O0 -ffreestanding -nostdlib -nostartfiles -std=c11 -g
	CXX_OPTS= $(CC_OPTS)
	AS_OPTS= -g
	LD_OPTS= $(CC_OPTS) -Wl,-Map=$(BUILD_DIR)/$(MAP_NAME).map -Wl,-cref  -Wl,--gc-sections -g

endif






TERMINAL=x-terminal-emulator

ifdef APP_NAME
	MAP_NAME=$(APP_NAME)
else
	MAP_NAME=kernel
endif


