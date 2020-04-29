.section ".dtb" /* stick this in the .dtb section */

.global __dtb
__dtb:
	.fill (1<<13), 1, 0x5A
