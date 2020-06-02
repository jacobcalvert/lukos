# tools and options

TOOLCHAIN ?= GNU

OPTIMIZE ?= 0

ifeq ($(TOOLCHAIN), LLVM)
	CC=clang
	CXX=clang
	AS=clang
	LD=aarch64-none-elf-gcc
	CC_OPTS= --target=aarch64  -Wall -O$(OPTIMIZE) -ffreestanding -nostdlib -fno-builtin -nostdinc -std=c11 -g -mcpu=$(CPU) -I /opt/apps/cross-compilers/gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf/lib/gcc/aarch64-none-elf/9.2.1/include
	CXX_OPTS= $(CC_OPTS)
	AS_OPTS= -g --target=aarch64 -mcpu=$(CPU)
	LD_OPTS=  -Wall -O$(OPTIMIZE) -ffreestanding -nostdlib -fno-builtin -nostdinc -std=c11 -g -mcpu=$(CPU) -Wl,-Map=$(BUILD_DIR)/$(MAP_NAME).map -Wl,-cref  -Wl,--gc-sections -g
	CROSS_COMPILE=aarch64-none-elf
	OBJCOPY=$(CROSS_COMPILE)-objcopy
	OBJDUMP=$(CROSS_COMPILE)-objdump
	SIZE=$(CROSS_COMPILE)-size
	GDB=$(CROSS_COMPILE)-gdb
endif

ifeq ($(TOOLCHAIN), GNU)
	CROSS_COMPILE=aarch64-none-elf
	CC=$(CROSS_COMPILE)-gcc
	CXX=$(CROSS_COMPILE)-g++
	AS=$(CROSS_COMPILE)-gcc
	LD=$(CROSS_COMPILE)-gcc
	OBJCOPY=$(CROSS_COMPILE)-objcopy
	OBJDUMP=$(CROSS_COMPILE)-objdump
	SIZE=$(CROSS_COMPILE)-size
	GDB=$(CROSS_COMPILE)-gdb
	
	CC_OPTS= -Wall -O$(OPTIMIZE) -ffreestanding -nostdlib -nostartfiles -std=c11 -g -mcpu=$(CPU)
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


