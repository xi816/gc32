main:
  mov %eax 3
  trap
  mov %ebx 2
  trap
.loop:
  pow %eax %ebx
  trap
  jmp .loop

