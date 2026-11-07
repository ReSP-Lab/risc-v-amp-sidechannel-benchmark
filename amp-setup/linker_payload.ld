OUTPUT_ARCH( "riscv" )
ENTRY( main )

SECTIONS
{
  /* . = @LOAD_ADDR@; */
  /* . = 0x82300000; */
  /* . = 0x82400000; */
  .text.main :
  {
      *(.text.main)
  }
  .text :
  {
      *(.text*)
  }
  .rodata :
  {
        *(.rodata*)
  }
  .data : {
      *(.data)
      *(.sdata)
  }
  .bss  : { 
      *(.bss)
      *(.sbss)
  }
}
