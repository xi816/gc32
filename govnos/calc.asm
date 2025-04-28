calc_main:
  ; First number
  mov %esi calc_00
  int $81
  call scani

  ; Second number
  mov %esi calc_01
  push %eax
  int $81
  call scani
  mov %ebx %eax
  pop %eax

  ; Operation
  mov %esi calc_02
  push %eax
  int $81
  pop %eax
  int $01
  pop %ecx
  push %ecx
  int $02
  push '$'
  int 2

  cmp %ecx '+'
  je .add
  cmp %ecx '-'
  je .sub
  cmp %ecx '*'
  je .mul
  cmp %ecx '/'
  je .div
  jmp .unk
.add:
  add %eax %ebx
  call puti
  push '$'
  int $02
  ret
.sub:
  sub %eax %ebx
  call puti
  push '$'
  int $02
  ret
.mul:
  mul %eax %ebx
  call puti
  push '$'
  int $02
  ret
.div:
  cmp %ebx $00
  je .div_panic
  div %eax %ebx
  call puti
  push '$'
  int $02
  ret
.div_panic:
  mov %eax $000000
  jmp krnl_panic
.unk:
  mov %esi calc_03
  int $81
  ret

calc_00:     bytes "Enter first number: ^@"
calc_01:     bytes "$Enter second number: ^@"
calc_02:     bytes "$Enter operation [+-*/]: ^@"
calc_03:     bytes "Unknown operation. Make sure you typed +, -, *, /$^@"
