OUTPUT_ARCH( "riscv" )
ENTRY( main )

SECTIONS
{
  . = @LOAD_ADDR@;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss  : { *(.bss) }
}
