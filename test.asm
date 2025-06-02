_main:
  push %ebp
  mov %ebp %esp
  sub %esp 16
.op_0:
  mov %eax 40960
  sub %ebp 4
  stod %ebp %eax
  add %ebp 0
.op_1:
  mov %eax dat
  add %eax 0
  call _puts
  sub %ebp 8
  lodd %ebp %eax
  add %ebp 4
.op_2:
  mov %eax 69
  mov %esp %ebp
  pop %ebp
  ret
.op_3:
  xor %eax %eax
  mov %esp %ebp
  pop %ebp
  ret
_puts:
  lodb %eax %e9
  cmp %e9 0
  re
  push %e9
  int 2
  jmp _puts

dat: bytes $48 $65 $6C $6C $6F $2C $20 $47 $43 $33 $32 $21 $0A $00
