/* Paint -- an image drawing program written in GovnASM */

  jmp main
draw:
  mov %esi x
  lodw %esi %eax
  mov %ebx y
  lodw %ebx %esi
  mul %esi 640
  add %esi %eax
  add %esi $400000
  stob %esi %edx
  ret
main:
  mov %eax $43
  mov #450000 %eax
  int $12
.l:
  int 1
  pop %eax
  push %eax
  mov %esi color
  lodb %esi %edx
  call draw
  pop %eax
  cmp %eax 'a'
  je .left
  cmp %eax 'd'
  je .right
  cmp %eax 'w'
  je .up
  cmp %eax 's'
  je .down
  cmp %eax 'x'
  je .colorx
  cmp %eax 'c'
  je .colorc
  cmp %eax 'q'
  je term
  jmp render
.left:
  dex @x
  jmp render
.right:
  inx @x
  jmp render
.up:
  dex @y
  jmp render
.down:
  inx @y
  jmp render
.colorx:
  dex #color
  jmp render.panel
.colorc:
  inx #color
  jmp render.panel

render:
  mov %edx $00
  call draw
  jmp .end
.panel:
  mov %esi color
  lodb %esi %eax
  mov %esi $447E00
  mov %ecx 12800
.l1:
  stob %esi %eax
  loop .l1
.end:
  int $11
  jmp main.l
term:
  hlt

x:     reserve 2 bytes
y:     reserve 2 bytes
color: reserve 1 bytes
