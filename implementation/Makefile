#######################################################
# File: Makefile
# Desc: Top level makefile for LuKOS
# Date: 16-Apr-2020
# Author: Jacob Calvert <jcalvert@jacobncalvert.com>
# Revision History
# -------------------
#######################################################

# define the build directory
BUILD_DIR=./build

# define source root
SRC_DIR=./src

# define a default platform
PLATFORM?=qemu-aarch64-virt

PYTHON=/usr/bin/python3.6

# kernel objects
KERNEL_OBJS=

# default includes 
KERNEL_INCLUDES = -I ./src/arch/$(ARCH)/h 

KERNEL_DEFINES = -D __LUKOS___




# include the platform (architecture will be included by platform)
include ./src/platform/$(PLATFORM)/platform.mk
include ./src/kernel/subdir.mk


all: $(KERNEL_OBJS) pre_kernel kernel post_kernel

clean:
	rm -rf $(BUILD_DIR)/*
	
	

.DEFAULT_GOAL:= all


FORCE: ;





# a commonly used routine
define checkdstdir
	mkdir -p $(dir $(1))
endef

# be quiet!
ifndef VERBOSE
.SILENT:
endif

