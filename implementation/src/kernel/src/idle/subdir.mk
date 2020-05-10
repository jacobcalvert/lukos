KERNEL_OBJS += $(BUILD_DIR)/src/kernel/src/idle/idle.o






$(BUILD_DIR)/src/kernel/src/idle/idle.o:./src/kernel/src/idle/idle.c
	echo "[CC] Compiling $^"
	$(call checkdstdir, $@)
	$(CC) $(CC_OPTS) $(KERNEL_DEFINES) $(KERNEL_INCLUDES) -fPIC -c -o $@ $^


