KERNEL_INCLUDES += -I ./src/kernel/h
include ./src/kernel/src/libraries/subdir.mk
include ./src/kernel/src/managers/subdir.mk
include ./src/kernel/src/userspace/subdir.mk
include ./src/kernel/src/idle/subdir.mk
