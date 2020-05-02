APP_NAME = 000-basic


BUILD_DIR=./apps/build/$(APP_NAME)/

# app objects
APP_OBJS= $(BUILD_DIR)/apps/$(APP_NAME)/app.o

# default includes 
KERNEL_INCLUDES = -I ./src/arch/$(ARCH)/h 

APP_INCLUDES = 

KERNEL_DEFINES = -D __LUKOS___

APP_DEFINES = -D __BASIC__APP__

APP_AS_OPTS =
APP_CC_OPTS = -fdata-sections -ffunction-sections
APP_CXX_OPTS =
APP_LD_OPTS = -Wl,-gc-sections 

include ./src/arch/$(ARCH)/app.mk
include ./src/arch/$(ARCH)/tools.mk




