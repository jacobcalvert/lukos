KERNEL_INCLUDES += -I ./src/kernel/h/libraries/c
include ./src/kernel/src/libraries/fdt/subdir.mk
include ./src/kernel/src/libraries/c/subdir.mk
include ./src/kernel/src/libraries/mem/subdir.mk
include ./src/kernel/src/libraries/elf/subdir.mk
