dir_main:
  mov %esi %r12
  mov %eax $00
  call strtok
  mov %r13 %esi

  mov %esi hcom
  int $81
  mov %esi %r13
  int $81
  push '#'
  int $02
  push $0A
  int $02

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
  mov %egi header
  mov %ecx 15
  push %esi
  inx %esi
  call dmemcpy

  mov %esi header
  add %esi 12
  mov %egi %r13
  mov %ecx 3
  call memcmp
  cmp %eax 1
  je .notyourtime

  push ' '
  int 2
  push ' '
  int 2
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
.notyourtime:
  pop %esi
  ; push '$'
  ; int 2
  pop %esi
  add %esi $200
  jmp .loop

dir_msg00: bytes "Volume in drive ^@"
dir_msg01: bytes " has no label.$Directory of ^@"
dir_msg02: bytes "/$^@"
header:    reserve 15 bytes

hcom:      bytes "Arguments given: #^@"

