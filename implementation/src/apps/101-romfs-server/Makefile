APP_NAME = 101-romfs-server


BUILD_DIR=./build

# app objects
APP_OBJS= $(BUILD_DIR)/src/apps/$(APP_NAME)/app.o $(BUILD_DIR)/src/apps/$(APP_NAME)/libc-glue.o $(BUILD_DIR)/src/apps/$(APP_NAME)/cli.o $(BUILD_DIR)/src/apps/$(APP_NAME)/log.o $(BUILD_DIR)/src/apps/$(APP_NAME)/romfs.o $(BUILD_DIR)/src/apps/$(APP_NAME)/romfs-bin.o $(NEWLIB_BUILD_DIR)/$(CROSS_COMPILE)/newlib/libc.a

# default includes 
KERNEL_INCLUDES = -I ./src/arch/$(ARCH)/h 

APP_INCLUDES = -I ./src/apps/$(APP_NAME)/newlib/h -I ./src/apps/$(APP_NAME)/h -I ./src/apps/$(APP_NAME)

KERNEL_DEFINES = -D __LUKOS___

APP_DEFINES = -D __BASIC__APP__ -D APP_NAME="\"$(APP_NAME)"\"

APP_AS_OPTS =
APP_CC_OPTS = -fdata-sections -ffunction-sections
APP_CXX_OPTS =
APP_LD_OPTS = -Wl,-gc-sections 

NEWLIB_GIT_ADDRESS= git://sourceware.org/git/newlib-cygwin.git
NEWLIB_BUILD_DIR=$(realpath .)/$(BUILD_DIR)/src/apps/$(APP_NAME)/newlib/build
NEWLIB_SOURCE_DIR=$(realpath .)/$(BUILD_DIR)/src/apps/$(APP_NAME)/newlib/source
NEWLIB_VERSION_TAG=newlib-3.3.0

newlib_download:
	if [ -d "$(NEWLIB_SOURCE_DIR)" ]; then \
		echo "[INFO] newlib source already present"; \
		# cd $(NEWLIB_SOURCE_DIR) && git pull origin $(NEWLIB_VERSION_TAG) > /dev/null; \
	else \
		echo "[INFO] Downloading newlib source"; \
		git clone $(NEWLIB_GIT_ADDRESS) $(NEWLIB_SOURCE_DIR); \
	fi
	git -C $(NEWLIB_SOURCE_DIR) checkout $(NEWLIB_VERSION_TAG) -q > /dev/null
	
$(NEWLIB_BUILD_DIR)/$(CROSS_COMPILE)/newlib/libc.a: newlib_download
	echo "[INFO] Building newlib source"
	if [ -d "$(NEWLIB_BUILD_DIR)" ]; then \
		cd $(NEWLIB_BUILD_DIR) && $(MAKE) > /dev/null; \
	else \
			mkdir -p $(NEWLIB_BUILD_DIR); \
			cd $(NEWLIB_BUILD_DIR) && $(NEWLIB_SOURCE_DIR)/configure --target=$(CROSS_COMPILE) && $(MAKE) CFLAGS_FOR_TARGET="-D__DYNAMIC_REENT__" CFLAGS_FOR_BUILD="-D__DYNAMIC_REENT__" clean all> /dev/null; \
	fi
	rm -rf ./src/apps/$(APP_NAME)/newlib/h
	mkdir -p ./src/apps/$(APP_NAME)/newlib/h
	cp -R $(NEWLIB_SOURCE_DIR)/newlib/libc/include/* ./src/apps/$(APP_NAME)/newlib/h
	

mkromfs:
	echo "[ROMFS] Generating ROMFS"
	genromfs -f ./src/apps/$(APP_NAME)/bin/romfs.img -d ./src/apps/$(APP_NAME)/romfs
	echo "[XXD] Generating romfs-bin.c"
	echo "char __romfs_data[]  __attribute__( ( aligned ( 16 ) ) )  = { " > src/apps/$(APP_NAME)/romfs-bin.c
	xxd -i < ./src/apps/$(APP_NAME)/bin/romfs.img >> src/apps/$(APP_NAME)/romfs-bin.c
	echo " }; " >> src/apps/$(APP_NAME)/romfs-bin.c
	
$(BUILD_DIR)/src/apps/$(APP_NAME)/romfs-bin.o: mkromfs
	echo "[CC] Compiling src/apps/$(APP_NAME)/romfs-bin.c"
	$(call checkdstdir, $@)
	$(CC) $(CC_OPTS) $(KERNEL_DEFINES) $(APP_DEFINES) $(KERNEL_INCLUDES) $(APP_INCLUDES) $(APP_CC_OPTS) -c -o $@ src/apps/$(APP_NAME)/romfs-bin.c



include ./src/arch/$(ARCH)/app.mk
include ./src/arch/$(ARCH)/tools.mk




