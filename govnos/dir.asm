dir_main:
  mov %esi $1F0000 ; Drive letter
  lodb %esi %ebx
  mov %esi dir_msg00
  int $81
  push %ebx
  int 2
  mov %esi dir_msg01
  int $81
  push %ebx
  int 2
  mov %esi dir_msg02
  int $81

  mov %ebx 0
  mov %esi $000200
.loop:
  ldds
  cmp %eax $01
  push %esi
  je .print
  pop %esi
  cmp %eax $F7
  re
  cmp %esi $800000
  re
  add %esi $200
  jmp .loop
.print:
  push ' '
  int 2
  push ' '
  int 2
  mov %egi header
  mov %ecx 15
  push %esi
  inx %esi
  call dmemcpy

  mov %esi header
  mov %ebx '^@'
  mov %edx ' '
  mov %ecx 15
  call memsub
  mov %esi header
  mov %ecx 15
  call write
  pop %esi
  push '$'
  int 2
  pop %esi
  add %esi $200
  jmp .loop

dir_msg00: bytes "Volume in drive ^@"
dir_msg01: bytes " has no label.$Directory of ^@"
dir_msg02: bytes "/$^@"
header:    reserve 15 bytes
