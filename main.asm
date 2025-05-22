  jmp main
puts:
  lodb %esi %eax
  cmp %eax $00
  re
  push %eax
  int $2
  jmp puts
main:
  mov %esi hw
  call puts
  hlt
hw: bytes "Hello, World!$^@"
