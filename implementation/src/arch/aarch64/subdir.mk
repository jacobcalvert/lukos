KERNEL_OBJS += $(BUILD_DIR)/src/arch/$(ARCH)/src/mmu/mmu.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/init/arch.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/init/pe.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/exceptions/stubs.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/exceptions/exceptions.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/interrupt-controllers/interrupt-controller.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/interrupt-controllers/gicv2.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/managers/vmm_arch.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/managers/pm_arch.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/os/atomic.o \
$(BUILD_DIR)/src/arch/$(ARCH)/src/scheduling/scheduling.o





$(BUILD_DIR)/src/arch/$(ARCH)/src/init/pe.o:./src/arch/$(ARCH)/src/init/pe.S
	echo "[AS] Assembling $^ with PIC"
	$(call checkdstdir, $@)
	$(AS) $(AS_OPTS) $(KERNEL_INCLUDES) -fpic -c -o $@ $^
