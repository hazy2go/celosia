/* vita-elf-create appends the SCE import/export data right after the code
   segment and fails when the data segment starts closer than that ("segment 1
   overlaps"): start the data segment 8 KB further on, whatever the code size. */
SECTIONS
{
  . = ALIGN(. + 0x2000, 0x1000);
}
INSERT BEFORE .init_array;
