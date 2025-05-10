cat_main:
  mov %esi %r12
  mov %eax $00
  call strtok
  mov %r13 %esi
  lodb %r13 %e8
  dex %r13
  cmp %e8 $00 ; If no CLI arguments
  je .no_args
  mov %esi %r13
  mov %eax $20
  call strtok
  sub %esi 2
  mov %eax $00
  stob %esi %eax
.open:
; mov %esi command
  mov %esi txt_header
  mov %ecx 16
  call b_memset
  push %r13
  mov %esi %r13
  mov %egi txt_header
  inx %egi
  call b_strcpy
  pop %r13
  mov %esi txt_tag
  mov %egi txt_header
  add %egi 13
  mov %ecx 3
  call b_memcpy

  mov %ebx txt_header
  mov %egi $280000
  call gfs2_read_file
  cmp %eax $00
  je .output
  jmp .failed
.output:
  mov %esi $280000
  int $81
  ret
.failed:
  mov %esi cat_msg01
  int $81
  mov %esi %r13
  int $81
  mov %esi cat_msg02
  int $81
  ret
.no_args:
  mov %esi cat_msg00
  int $81
  ret

cat_msg00: bytes "No tag given. Try `cat test.txt` to see the test.txt file's contents.$^@"
cat_msg01: bytes "cat: file `^@"
cat_msg02: bytes "` not found$^@"
txt_header:                reserve 16 bytes
txt_tag:                   bytes "txt^@"
