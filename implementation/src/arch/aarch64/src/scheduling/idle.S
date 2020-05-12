.section ".data" /* stick this in the .data section */
.align 12
.global __idle
__idle:
	.incbin "./src/arch/aarch64/obj/idle.elf"
