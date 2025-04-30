; boot.asm -- a bootloader for the GovnOS operating system
reboot: jmp boot

b_scans:
  int 1
  pop %eax
  cmp %eax $7F
  je .back
  cmp %eax $1B
  je b_scans
  push %eax
  int 2
  cmp %eax $0A
  je .end
  stob %esi %eax
  inx @clen
  jmp b_scans
.back:
  mov %egi clen
  lodw %egi %eax
  cmp %eax $00
  je b_scans
.back_strict:
  push %esi
  mov %esi bs_seq
  int $81
  pop %esi
  dex %esi
  dex @clen
  jmp b_scans
.end:
  mov %eax $00
  stob %esi %eax
  ret

b_strcmp:
  lodb %esi %eax
  lodb %egi %ebx
  cmp %eax %ebx
  jne .fail
  cmp %eax $00
  je .eq
  jmp b_strcmp
.eq:
  mov %eax $00
  ret
.fail:
  mov %eax $01
  ret

b_pstrcmp:
  lodb %esi %eax
  lodb %egi %ebx
  cmp %eax %ebx
  jne .fail
  cmp %eax %ecx
  je .eq
  jmp b_pstrcmp
.eq:
  mov %eax $00
  ret
.fail:
  mov %eax $01
  ret

b_dmemcmp:
  dex %ecx
.loop:
  ldds
  inx %esi
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

b_strtok:
  lodb %esi %eax
  cmp %eax %ecx
  re
  jmp b_strtok

b_strnul:
  lodb %esi %eax
  cmp %eax $00
  je .nul
  mov %eax $01
  ret
.nul:
  mov %eax $00
  ret

b_strcpy:
  lodb %esi %eax
  cmp %eax $00
  re
  stob %egi %eax
  jmp b_strcpy

b_memcpy:
  dex %ecx
.loop:
  lodb %esi %eax
  stob %egi %eax
  loop .loop
  ret

b_memset:
  dex %ecx
  mov %eax $00
.loop:
  stob %esi %eax
  loop .loop
  ret

b_write:
  push %eax
  dex %ecx
.loop:
  lodb %esi %eax
  push %eax
  int $02
  loop .loop
  pop %eax
  ret

b_puti:
  mov %egi b_puti_buf
  add %egi 7
.loop:
  div %eax 10 ; Divide and get the remainder into %edx
  add %edx 48 ; Convert to ASCII
  stob %egi %edx
  sub %egi 2
  cmp %eax $00
  jne .loop
  mov %esi b_puti_buf
  mov %ecx 8
  call b_write
  call b_puti_clr
  ret
b_puti_clr:
  mov %esi b_puti_buf
  mov %eax $00
  mov %ecx 7 ; 8
.loop:
  stob %esi %eax
  loop .loop
  ret
b_puti_buf: reserve 8 bytes

b_scani:
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
  mov %esi bs_seq
  push %eax
  int $81
  pop %eax
  div %eax 10
  jmp .loop

; GovnFS 2.0 driver stores frequently used information
; to a config field at bank $1F
; Entries:
;   $1F0000 -- Drive letter
gfs2_configure:
  mov %esi $000010
  mov %egi $1F0000
  ldds
  stob %egi %eax
  ret

gfs2_read_file:
  mov %edx $0001 ; Starting search at sector 1 (sector 0 is for header data)
  mov %esi $0200 ; Precomputed starting address (sector*512)
.loop:
  ldds          ; Read the first byte from a sector to get its type
                ;   00 -- Empty sector
                ;   01 -- File start
                ;   02 -- File middle/end
                ;   F7 -- Disk end
  cmp %eax $01
  je .flcheck   ; Check filename and tag
  cmp %eax $00
  je .skip      ; Skip
  cmp %eax $F7
  je .fail
.flcheck:
  inx %esi       ; Get to the start of filename
  mov %ecx 15    ; Check 15 bytes
  push %egi
  push %ebx
  mov %egi %ebx
  inx %egi
  call b_dmemcmp
  pop %ebx
  pop %egi
  cmp %eax $00
  je flcpy
  inx %edx
  mov %esi %edx
  mul %esi $0200
  jmp .loop
.skip:
  add %esi $0200
  jmp .loop
.fail:
  mov %eax $01
  ret

flcpy: ; %edx should already contain file's start sector
  ; mov %egi $200000 ; should be configured by the caller
.read:
  cmp %edx $0000
  je .end
  mov %esi %edx
  mul %esi $0200
  add %esi 16  ; Skip the processed header
  mov %ecx 494 ; Copy 494 bytes (sectorSize(512) - sectorHeader(16) - sectorFooter(2))
  dex %ecx
.loop:
  ldds
  inx %esi
  stob %egi %eax
  loop .loop
  ; inx %esi
  ldds
  mov %ebx %eax
  inx %esi
  ldds
  mul %eax $100
  add %eax %ebx
  cmp %eax $00
  je .end
  mov %esi %eax
  mul %esi $200
  add %esi 16
  mov %ecx 494
  ; dex %ecx
  loop .loop
.end:
  mov %eax $00
  ret

fre_sectors:
  mov %ebx 0
  mov %esi $000200
.loop:
  ldds
  add %esi $200
  cmp %eax $01
  je .inc
  cmp %eax $F7
  je .end
  cmp %esi $800000
.inc: ; RIP GovnDate 2.025e3-04-13-2.025e3-04-13
  inx %ebx
  jmp .loop
.end:
  mov %eax %ebx
  ret

boot:
  ; Load the kernel libraries
  call gfs2_configure
  mov %esi welcome_msg
  int $81
  mov %esi krnl_load_msg
  int $81
  mov %esi emp_sec_msg00
  int $81
  call fre_sectors
  call b_puti
  mov %esi emp_sec_msg01
  int $81

  mov %ebx krnl_file_header
  mov %egi $A00000
  call gfs2_read_file
  cmp %eax $00
  jne shell
  call $A00000
shell:
.prompt:
  mov %esi env_PS
  int $81

  mov %esi clen
  mov %eax $0000
  stow %esi %eax
  mov %esi command
  call b_scans
.process:
  mov %esi command
  call b_strnul
  cmp %eax $00
  je .aftexec

  mov %esi command
  mov %egi com_hi
  call b_strcmp
  cmp %eax $00
  je govnos_hi

  mov %esi command
  mov %egi com_date
  call b_strcmp
  cmp %eax $00
  je govnos_date

  mov %esi command
  mov %egi com_echo
  mov %ecx ' '
  call b_pstrcmp
  cmp %eax $00
  je govnos_echo

  mov %esi command
  mov %egi com_exit
  call b_strcmp
  cmp %eax $00
  je govnos_exit

  mov %esi command
  mov %egi com_cls
  call b_strcmp
  cmp %eax $00
  je govnos_cls

  mov %esi command
  mov %egi com_help
  call b_strcmp
  cmp %eax $00
  je govnos_help

  mov %esi file_header
  mov %ecx 16
  call b_memset
  mov %esi command
  mov %egi file_header
  inx %egi
  call b_strcpy
  mov %esi file_tag
  mov %egi file_header
  add %egi 13
  mov %ecx 3
  call b_memcpy

  mov %ebx file_header
  mov %egi $200000
  call gfs2_read_file
  cmp %eax $00
  je .call
  jmp .bad
.call:
  call $200000
  jmp .aftexec
.bad:
  mov %esi bad_command
  int $81
.aftexec:
  jmp .prompt
govnos_hi:
  mov %esi hai_world
  int $81
  jmp shell.aftexec
govnos_date:
  int 3
  mov %eax %edx
  sar %eax 9
  add %eax 1970
  push %edx
  call b_puti
  pop %edx
  push '-'
  int 2
  mov %eax %edx
  sar %eax 5
  mov %ebx $000F
  and %eax %ebx
  inx %eax
  cmp %eax 10
  jg .p0
  push '0'
  int 2
.p0:
  push %edx
  call b_puti
  pop %edx
  push '-'
  int 2
  mov %eax %edx
  mov %ebx $001F
  and %eax %ebx
  inx %eax
  cmp %eax 10
  jg .p1
  push '0'
  int 2
.p1:
  call b_puti
  push '$'
  int 2
  jmp shell.aftexec
govnos_cls:
  mov %esi cls_seq
  int $81
  jmp shell.aftexec
govnos_help:
  mov %esi help_msg
  int $81
  jmp shell.aftexec
govnos_exit:
  hlt
  jmp shell.aftexec
govnos_echo:
  mov %esi command
  mov %ecx ' '
  call b_strtok
  int $81
  push $0A
  int 2
  jmp shell.aftexec

welcome_msg:   bytes "Welcome to ^[[92mGovnOS^[[0m$^@"
krnl_load_msg: bytes "Loading ^[[92m:/krnl.bin/com^[[0m...$^@"
emp_sec_msg00: bytes "$Disk sectors used: ^[[92m^@"
emp_sec_msg01: bytes "^[[0m$$^@"
bad_command:   bytes "Bad command.$^@"

help_msg:    bytes "+------------------------------------------+$"
             bytes "|GovnOS help page 1/1                      |$"
             bytes "|  calc        Calculator                  |$"
             bytes "|  cls         Clear the screen            |$"
             bytes "|  dir         Show files on the disk      |$"
             bytes "|  echo        Echo text back to output    |$"
             bytes "|  exit        Exit from the shell         |$"
             bytes "|  gsfetch     Show system info            |$"
             bytes "|  help        Show help                   |$"
             bytes "+------------------------------------------+$^@"

com_hi:      bytes "hi^@"
com_cls:     bytes "cls^@"
com_date:    bytes "date^@"
com_help:    bytes "help^@"
com_echo:    bytes "echo "
com_exit:    bytes "exit^@"
hai_world:   bytes "hai world :3$^@"

env_HOST:    bytes "GovnPC 32 Ultra Edition^@"
env_OS:      bytes "GovnOS 0.6.0 For GovnoCore32^@"
env_CPU:     bytes "Govno Core 32 Real$^@"

; TODO: unhardcode file header TODO: remove this todo
com_predefined_file_header: bytes "^Afile.bin^@^@^@^@com^@"
krnl_file_header:           bytes "^Akrnl.bin^@^@^@^@com^@"
file_header:                reserve 16 bytes
file_tag:                   bytes "com^@"

command:     reserve 64 bytes
clen:        reserve 2 bytes

bs_seq:      bytes "^H ^H^@"
cls_seq:     bytes "^[[H^[[2J^@"
env_PS:      bytes "# ^@"

bse:         bytes $AA $55
