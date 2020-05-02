.section ".data" /* stick this in the .dtb section */
.align 12
.global __app
__app:
	.incbin "./src/platform/qemu-aarch64-virt/000-basic.elf"
