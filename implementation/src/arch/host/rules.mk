$(BUILD_DIR)/%.o:%.S
	echo "[AS] Assembling $^"
	$(call checkdstdir, $@)
	$(AS) $(AS_OPTS) $(KERNEL_INCLUDES) -c -o $@ $^


$(BUILD_DIR)/%.o:%.c
	echo "[CC] Compiling $^"
	$(call checkdstdir, $@)
	$(CC) $(CC_OPTS) $(KERNEL_DEFINES) $(KERNEL_INCLUDES) -c -o $@ $^


$(BUILD_DIR)/%.o:%.cpp
	echo "[CXX] Compiling $^"
	$(call checkdstdir, $@)
	$(CXX) $(CXX_OPTS) $(KERNEL_DEFINES) $(KERNEL_INCLUDES) -c -o $@ $^
