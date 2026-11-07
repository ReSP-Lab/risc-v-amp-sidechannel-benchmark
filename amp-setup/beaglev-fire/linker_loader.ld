OUTPUT_ARCH( "riscv" )
ENTRY( loader_main )

SECTIONS
{
  . = 0x82000000;
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
