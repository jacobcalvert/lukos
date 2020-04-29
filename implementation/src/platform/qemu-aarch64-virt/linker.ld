/**
 * @file linker.ld
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * 
 * This linker script is for the raspberry pi 3 B/B+
 */


OUTPUT_ARCH(aarch64)

ENTRY(_start)

ALIGN_SIZE_BYTES = 8; /* always align to a 64bit boundary */
PAGE_SIZE_2M = (1<<21); /* align for data/bss/rw data */
PAGE_SIZE_4K = (0x1000);

MEMORY 
{

	RAM_PHYS    (rwx) : ORIGIN = 0x0000000040000000, LENGTH = 64M
	RAM_VIRT  (rwx) : ORIGIN = 0xFFFFFF8000000000, LENGTH = 64M
}

SECTIONS
{
	/* boot code is linked and loaded to the physical ram address */
    .boot : 
    {
        KEEP(*(.text.boot))
     . = ALIGN(PAGE_SIZE_2M); 
    }>RAM_PHYS
   
  	/* text is linked to virtual space, loaded to physical space */
    .text : ALIGN(PAGE_SIZE_4K)
    {  	
        *(.text .text.* .gnu.linkonce.t.*)
        *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
        *(.init .init.*)
		KEEP(*(.note.gnu.build-id))
		. = ALIGN(PAGE_SIZE_4K);
    } > RAM_VIRT AT > RAM_PHYS
 

    .rodata : ALIGN(PAGE_SIZE_4K)
    {
    	*(.rodata .rodata.*)
        *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) 
        *(.rodata)
         . = ALIGN(PAGE_SIZE_2M); /* leave space for mapping the default page table */
        
    }> RAM_VIRT AT > RAM_PHYS

    .data :  ALIGN(PAGE_SIZE_4K)
    {
        *(.data .data.* .gnu.linkonce.d.*)
        *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
		*(.dtb)
 		. = ALIGN(PAGE_SIZE_4K);
    }> RAM_VIRT AT > RAM_PHYS

    .bss :  ALIGN(PAGE_SIZE_4K)
    { 
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
        *(COMMON)

    }> RAM_VIRT AT > RAM_PHYS
    
	

	

    
}

/* LuKOS required symbols */
__text_start_vaddr = ADDR(.text);
__text_start_paddr = LOADADDR(.text);
__text_size	= SIZEOF(.text);

__rodata_start_vaddr = ADDR(.rodata);
__rodata_start_paddr = LOADADDR(.rodata);
__rodata_size = SIZEOF(.rodata);

__data_start_vaddr = ADDR(.data);
__data_start_paddr = LOADADDR(.data);
__data_size = SIZEOF(.data);

__bss_start_vaddr = ADDR(.bss);
__bss_start_paddr = LOADADDR(.bss);
__bss_size = SIZEOF(.bss);

__end_paddr = LOADADDR(.bss) + SIZEOF(.bss);

/* boot code required symbols */
__heap_end_vaddr = (ORIGIN(RAM_VIRT) + LENGTH(RAM_VIRT));

