main:
  mov     %eax 6
  cif     %eax
  mov     %ebx 1
  cif     %ebx
  negf32  %ebx
  trap

  mulf32  %eax %ebx
  trap
  cfi     %eax
  trap
  hlt
