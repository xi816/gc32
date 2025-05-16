; Custom interrupt handler for division by zero

main:
  mov %esi func
  sti $C0
  trap

  mov %eax 4
  mov %ebx 0
  div %eax %ebx
  push 'A'
  int $2
  hlt

func:
  push 'B'
  int $2
  iret

