.section ".data" /* stick this in the .data section */
.align 12
.global __io_server
__io_server:
	.incbin "./src/platform/qemu-aarch64-virt/102-pl011-io-server.elf"
	
.align 12
.global __idle
__idle:
		.incbin "./src/platform/qemu-aarch64-virt/000-basic.elf"
		
		
.align 12
.global __romfs_server
__romfs_server:
		.incbin "./src/platform/qemu-aarch64-virt/101-romfs-server.elf"
		
		
.align 12
.global __terminal_server
__terminal_server:
		.incbin "./src/platform/qemu-aarch64-virt/103-terminal-server.elf"
