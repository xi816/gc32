inter_main:
  ; Set up Palitro mode
  mov %eax $01
  mov #49FF00 %eax
  mov %eax $02
  mov #450000 %eax

  mov %esi $4A0000
  mov %eax $01FF
  stow %esi %eax
  mov %eax $318C
  stow %esi %eax
  mov %eax $01BA
  stow %esi %eax
  mov %eax $7FFF
  stow %esi %eax
  int $12

  mov %esi $447180
  mov %ecx 15999
  mov %eax $01
.draw_panel:
  stob %esi %eax
  loop .draw_panel
  int $11

  ; int $01
  ; pop %edx

aloop:
; Check key state
  mov %eax $480005
  mov %ecx 5
.kbdloop:
  lodb %eax %edx
  cmp %edx 0
  jne .noCur
  loop .kbdloop
  ; Hehe
.check_serial:
  int $9
  cmp %edx 0
  jne .popi
  ; Clear old cursor
  mov %eax old_cur_pos
  lodw %eax %ebx
  lodw %eax %ecx
  mov %egi old_on_cur
  call spr

  mov %eax $480000
  lodw %eax %ebx
  lodw %eax %ecx
  lodb %eax %edx ; Mouse state
  mov %e8 old_mouse_pos
  lodb %e8 %e9
  cmp %edx %e9
  jne .first_mouse_check
  jmp .mouse_checked
.first_mouse_check:
  mov %e8 old_mouse_pos
  stob %e8 %edx
  cmp %edx 2
  je .pressedc
  jmp .pressedca
.pressedc:
  call .pressed
.pressedca:
.mouse_checked:
  mov %egi old_on_cur
  call read_spr
  mov %egi cursor
  call spr
  mov %esi old_cur_pos
  stow %esi %ebx
  stow %esi %ecx
.noCur:
  int $11
  mov %edx 8
  int $22
  jmp aloop
.popi:
  int $1
  pop %edx
  jmp .check_serial
.pressed:
  mov %e9 $4A0000
  lodw %e9 %e10
  lodw %e9 %e11
  lodw %e9 %e12
  lodw %e9 %e13
  inx %e10 inx %e11 inx %e12
  mov %e9 $4A0000
  stow %e9 %e10
  stow %e9 %e11
  stow %e9 %e12
  stow %e9 %e13
  push 'a'
  int 2
  ret

spr:
  mov %esi %ecx
  mul %esi 640
  add %esi %ebx
  add %esi $400000
  int $13
  ret

read_spr:
  mov %esi %ecx
  mul %esi 640
  add %esi %ebx
  add %esi $400000
  int $15
  ret

old_cur_pos:   reserve 4 bytes
old_mouse_pos: reserve 1 bytes
cursor:
  bytes $FF $FF $FF $FF $00 $00 $00 $00
  bytes $FF $FF $00 $00 $00 $00 $00 $00
  bytes $FF $00 $FF $00 $00 $00 $00 $00
  bytes $FF $00 $00 $FF $00 $00 $00 $00
  bytes $00 $00 $00 $00 $FF $00 $00 $00
  bytes $00 $00 $00 $00 $00 $FF $00 $00
  bytes $00 $00 $00 $00 $00 $00 $FF $00
  bytes $00 $00 $00 $00 $00 $00 $00 $FF

; cursor:
;   bytes $FF $FF $00 $00 $00 $00 $00 $00
;   bytes $FF $03 $FF $00 $00 $00 $00 $00
;   bytes $FF $03 $03 $FF $00 $00 $00 $00
;   bytes $FF $03 $03 $03 $FF $00 $00 $00
;   bytes $FF $03 $03 $03 $03 $FF $00 $00
;   bytes $FF $03 $03 $03 $FF $FF $00 $00
;   bytes $FF $03 $FF $03 $FF $00 $00 $00
;   bytes $FF $FF $FF $03 $FF $00 $00 $00

old_on_cur: reserve 256 bytes
