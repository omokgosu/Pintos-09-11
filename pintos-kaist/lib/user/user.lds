OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
  /* 읽기 전용 섹션들, 텍스트 세그먼트로 병합됨: */
  __executable_start = 0x0400000 + SIZEOF_HEADERS;
  . = 0x0400000 + SIZEOF_HEADERS;
  .text : AT(0x400000 + SIZEOF_HEADERS) {
    *(.text)
    *(.note.gnu.build-id)
  } = 0x90

  .rodata : { *(.rodata) }

  /* 데이터 세그먼트의 주소를 조정합니다. 다음 페이지의 
     페이지 내에서 같은 주소까지 조정하고 싶습니다. */
  . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));

  .data : { *(.data) }
  .bss : { *(.bss) }

  . = DATA_SEGMENT_RELRO_END (0, .);

  /* Stabs 디버깅 섹션들 */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }

  /* DWARF 디버그 섹션들.
  DWARF 디버깅 섹션의 심볼들은 섹션의 시작점을 기준으로 하므로 
  0에서 시작합니다. */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 확장 */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1과 DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 확장 */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /DISCARD/ : { *(.note.GNU-stack) }
  /DISCARD/ : { *(.eh_frame) }
}
