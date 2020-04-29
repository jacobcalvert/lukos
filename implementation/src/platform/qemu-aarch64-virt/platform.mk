ARCH=aarch64
CPU=cortex-a53

PLATFORM_DIR=./src/platform/$(PLATFORM)/
ARCH_DIR=./src/arch/$(ARCH)/

include ./src/arch/$(ARCH)/tools.mk
include ./src/arch/$(ARCH)/rules.mk
include ./src/arch/$(ARCH)/subdir.mk
include ./src/platform/$(PLATFORM)/subdir.mk

pre_kernel:
	@echo ""


kernel: $(KERNEL_OBJS)
	echo "[LD] Linking kernel"
	$(LD) $(LD_OPTS) -T $(PLATFORM_DIR)/linker.ld -o $(BUILD_DIR)/kernel.elf $(KERNEL_DEFINES) $(KERNEL_OBJS)

post_kernel:
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
