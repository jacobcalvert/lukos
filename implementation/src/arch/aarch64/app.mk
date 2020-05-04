
APP_OBJS += $(BUILD_DIR)/src/arch/$(ARCH)/src/userspace/memory.o

KERNEL_INCLUDES += -I ./src/kernel/h/

# a commonly used routine
define checkdstdir
	mkdir -p $(dir $(1))
endef

# be quiet!
ifndef VERBOSE
.SILENT:
endif


$(BUILD_DIR)/%.o:%.S
	echo "[AS] Assembling $^"
	$(call checkdstdir, $@)
	$(AS) $(AS_OPTS) $(KERNEL_INCLUDES) $(APP_INCLUDES) $(APP_AS_OPTS) -c -o $@ $^


$(BUILD_DIR)/%.o:%.c
	echo "[CC] Compiling $^"
	$(call checkdstdir, $@)
	$(CC) $(CC_OPTS) $(KERNEL_DEFINES) $(APP_DEFINES) $(KERNEL_INCLUDES) $(APP_INCLUDES) $(APP_CC_OPTS) -c -o $@ $^


$(BUILD_DIR)/%.o:%.cpp
	echo "[CXX] Compiling $^"
	$(call checkdstdir, $@)
	$(CXX) $(CXX_OPTS) $(KERNEL_DEFINES) $(APP_DEFINES) $(KERNEL_INCLUDES) $(APP_INCLUDES) $(APP_CXX_OPTS) -c -o $@ $^
	
	
	
all: $(APP_OBJS)
	echo "[LD] Linking $(APP_NAME)"
	$(LD) $(LD_OPTS) -T ./src/arch/$(ARCH)/app.ld -o $(BUILD_DIR)/$(APP_NAME).elf $(KERNEL_DEFINES) $(APP_DEFINES) $(APP_LD_OPTS) $(APP_OBJS) 
	
clean:
	rm -rf $(BUILD_DIR)/*
