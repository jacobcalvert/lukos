/**
 * @file app.ld
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * 
 * This linker script is for building applications for LuKOS on AARCH64
 */


OUTPUT_ARCH(aarch64)

ENTRY(main)

PAGE_SIZE_4K = (0x1000);

MEMORY 
{
	RAM    (rwx) : ORIGIN = 0, LENGTH = 4096M


}

SECTIONS
{
  	/* text is linked to virtual space */
    .text : ALIGN(PAGE_SIZE_4K)
    {  	
        *(.text .text.* .gnu.linkonce.t.*)
        *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
        *(.init .init.*)
		KEEP(*(.note.gnu.build-id))
		. = ALIGN(PAGE_SIZE_4K);
    }> RAM

    .rodata : ALIGN(PAGE_SIZE_4K)
    {
    	*(.rodata .rodata.*)
        *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) 
        *(.rodata)
         . = ALIGN(PAGE_SIZE_4K); 
        
    }> RAM

    .data :  ALIGN(PAGE_SIZE_4K)
    {
        *(.data .data.* .gnu.linkonce.d.*)
        *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
		*(.dtb)
 		. = ALIGN(PAGE_SIZE_4K);
    }> RAM

    .bss :  ALIGN(PAGE_SIZE_4K)
    { 
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
        *(COMMON)

    }> RAM
   
}

