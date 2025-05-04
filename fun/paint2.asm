loop: ._:
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
    ; mov %eax old_cur_pos
    ; lodw %eax %ebx
    ; lodw %eax %ecx
    ; mov %egi nul
    ; call spr

    mov %eax $480000
    lodw %eax %ebx
    lodw %eax %ecx
    ; lodb %eax %edx - Mouse state
    mov %egi cursor
    call spr
    mov %esi old_cur_pos
    stow %esi %ebx
    stow %esi %ecx
.noCur:
    int $11
    mov %edx 8
    int $22
    jmp ._
.popi:
    int $1
    pop %edx
    jmp .check_serial

spr:
    mov %esi %ecx
    mul %esi 640
    add %esi %ebx
    add %esi $400000
    int $13
    ret

old_cur_pos: reserve 4 bytes
cursor:
    bytes $FF $FF $00 $00 $00 $00 $00 $00 $FF $00 $FF $00 $00 $00 $00 $00 $FF $00 $00 $FF $00 $00 $00 $00 $FF $00 $00 $00 $FF $00 $00 $00 $FF $00 $00 $00 $00 $FF $00 $00 $FF $00 $00 $00 $FF $FF $00 $00 $FF $00 $FF $00 $FF $00 $00 $00 $FF $FF $FF $00 $FF $00 $00 $00

nul:
