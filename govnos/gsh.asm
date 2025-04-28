; gsh.asm -- a basic shell implementation, that originally was at boot.asm
; but now the RO GovnFS 2.0 driver is complete, so i seprated it to a file that
; loads at startup

  jmp gshMain
gshMain:
  mov %esi gshWelcome
  int $81
gshShell:
  mov %esi gshPS1
  int $81

  mov %esi scans_len
  mov %eax $0000
  stow %esi %eax

  mov %esi gshCommand
  call scans

  mov %esi gshCommand
  mov %egi gshCommExit
  call strcmp
  cmp %eax $00
  je gshExit

  mov %esi gshCommand
  mov %egi gshCommHelp
  call strcmp
  cmp %eax $00
  je gshHelp

  mov %esi gshCommand
  mov %egi gshCommHi
  call strcmp
  cmp %eax $00
  je gshHi

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
  jmp gshBadComm
.call:
  call $200000
  jmp gshAftexec
gshHi:
  mov %esi gshHw
  int $81
  jmp gshAftexec
gshHelp:
  mov %esi gshHelpMsg
  int $81
  jmp gshAftexec
gshExit:
  ret
gshBadComm:
  push '"' ; "
  int $02
  mov %esi gshCommand
  int $81
  mov %esi gshBadCommMsg
  int $81
  jmp gshAftexec

gshAftexec:
  jmp gshShell
  ret

gshHw:      bytes "Hiiiii :3$^@"
gshWelcome: bytes "Welcome to gsh, a simple GovnOS shell$^@"
gshPS1:     bytes "^$ ^@"
gshCommand: reserve 128 bytes

gshCommHi:     bytes "hi^@"
gshCommHelp:   bytes "help^@"
gshCommExit:   bytes "exit^@"
gshBadCommMsg: bytes "\": bad command or file name$^@"

gshHelpMsg:    bytes "+-------------------------------+$"
               bytes "| GovnOS help page 1/1          |$"
               bytes "|   exit    Exit from the shell |$"
               bytes "|   help    Show help           |$"
               bytes "+-------------------------------+$^@"

