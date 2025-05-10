  jmp kmain

; write -- Write n characters to the screen
; Arguments:
;   cx -- count
;   si -- string
write:
  push %eax
  dex %ecx
.loop:
  lodb %esi %eax
  push %eax
  int $02
  loop .loop
  pop %eax
  ret

; puti -- Write a 24-bit unsigned number to the screen
; Arguments:
;   ax -- number
puti:
  mov %egi puti_buf
  add %egi 9
.loop:
  div %eax 10 ; Divide and get the remainder into %edx
  add %edx 48 ; Convert to ASCII
  stob %egi %edx
  sub %egi 2
  cmp %eax $00
  jne .loop
  mov %esi puti_buf
  mov %ecx 10
  call write
  call puti_clr
  ret
puti_clr:
  mov %esi puti_buf
  mov %eax $00
  mov %ecx 9 ; 10 == floor(log(U24_MAX))+1;
.loop:
  stob %esi %eax
  loop .loop
  ret
puti_buf: reserve 10 bytes

strtok:
  lodb %esi %eax
  cmp %eax %ecx
  re
  jmp strtok

; strcmp -- Compare two strings
; Arguments:
;   si -- string 1
;   gi -- string 2
; Returns:
;   ax -- result (0 for equal, 1 for not equal)
strcmp:
  lodb %esi %eax
  lodb %egi %ebx
  cmp %eax %ebx
  jne .fail
  cmp %eax $00
  je .eq
  jmp strcmp
.eq:
  mov %eax $00
  ret
.fail:
  mov %eax $01
  ret

; scani -- Read a 24-bit unsigned integer stdout and save to ax
; Returns:
;   ax -- number
scani:
  mov %eax $00
.loop:
  int $01
  pop %ebx

  cmp %ebx $0A ; Check for Enter
  re
  cmp %ebx $20 ; Check for space
  re
  cmp %ebx $7F ; Check for Backspace
  je .back
  cmp %ebx $30 ; Check if less than '0'
  jl .loop
  cmp %ebx $3A ; Check if greater than '9'+1
  jg .loop

  mul %eax 10
  push %ebx
  int $02
  sub %ebx 48
  add %eax %ebx
  jmp .loop
.back: ; Backspace handler
  cmp %eax 0
  jne .back_strict
  jmp .loop
.back_strict:
  mov %esi krnl_bksp_seq
  push %eax
  int $81
  pop %eax
  div %eax 10
  jmp .loop

; scans -- Read a string from stdout and save to a buffer
; Arguments:
;   si -- buffer address
scans:
  int 1
  pop %eax
  cmp %eax $7F
  je .back
  cmp %eax $1B
  je scans
  push %eax
  int 2
  cmp %eax $0A
  je .end
  stob %esi %eax
  inx @scans_len
  jmp scans
.back:
  mov %egi scans_len
  lodw %egi %eax
  cmp %eax $00
  je scans
.back_strict:
  push %esi
  mov %esi krnl_bksp_seq
  int $81
  pop %esi
  dex %esi
  dex @scans_len
  jmp scans
.end:
  mov %eax $00
  stob %esi %eax
  ret
scans_len: reserve 2 bytes

; memsub -- Substitute one character to another in a string
; Arguments:
;   ax -- from character
;   bx -- to character
;   si -- string
memsub:
  dex %ecx
.loop:
  lodb %esi %eax
  cmp %eax %ebx
  je .sub
  loop .loop
  ret
.sub:
  dex %esi
  stob %esi %edx
  loop .loop
  ret

memcmp:
  dex %ecx
.loop:
  lodb %esi %eax
  lodb %egi %ebx
  cmp %eax %ebx
  jne .fail
  loop .loop
.eq:
  mov %eax $00
  ret
.fail:
  mov %eax $01
  ret

; dmemcpy -- Load n bytes from disk to memory
; Arguments:
;   si -- from address (disk)
;   gi -- to address (memory)
dmemcpy:
  dex %ecx
.loop:
  ldds
  inx %esi
  stob %egi %eax
  loop .loop
  ret

krnl_panic:
  mov %esi krnl_panic00
  int $81
  int 1
  pop %edx
  mov %esi krnl_show_cursor
  int $81
  hlt

; kmain executes when the kernel loads
kmain:
  ret

; Constants/data
krnl_bksp_seq: bytes "^H ^H^@"
krnl_panic00:  bytes "^[[44m^[[H^[[2J^[[?25l$"
               bytes "A problem has been detected and GovnOS has been down to prevent damage$"
               bytes "to your computer.$$"
               bytes "UNKNOWN_DETECTED_ERROR$$"
               bytes "If this is the first time you've seen this error screen,$"
               bytes "restart your computer. If this screen appears again, follow$"
               bytes "these steps:$$"
               bytes "Check to make sure any new GovnOS binary files are properly installed.$"
               bytes "If this is a new installation, ask your Govno Core / GovnOS manufacturer$"
               bytes "for any Govno Core / GovnOS updates you might need.$$"
               bytes "If problems continue, disable or remove any newly installed hardware$"
               bytes "or software. Do not change BIOS if you don't know how to do it properly.$$"
               bytes "Press any key to shut down"
               bytes "^@"
krnl_show_cursor: bytes "^[[0m^[[H^[[2J^[[?25h^@"
