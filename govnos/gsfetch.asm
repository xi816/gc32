govnos_gsfetch:
  mov %esi gsfc_000
  int $81

  ; Hostname
  mov %esi gsfc_001
  int $81
  mov %esi env_HOST
  int $81

  ; OS name
  mov %esi gsfc_002
  int $81
  mov %esi env_OS
  int $81

  ; CPU name
  mov %esi gsfc_003
  int $81
  mov %esi env_CPU
  int $81

  ; Memory
  mov %esi gsfc_004
  int $81
  mov %eax bse
  sub %eax $030002
  call b_puti
  mov %esi gsfc_005
  int $81

  ; Disk space
  mov %esi gsfc_008
  int $81
  call fre_sectors
  mul %eax 512
  call b_puti
  mov %esi gsfc_009
  int $81

  mov %esi gsfc_006
  int $81
  mov %ecx 7 ; 8 colors
  mov %eax $30
.gsfcL1:
  push '^[' int $2
  push '['  int $2
  push '4'  int $2
  push %eax  int $2
  push 'm'  int $2
  push ' '  int $2
  push ' '  int $2
  inx %eax
  loop .gsfcL1

  mov %esi gsfc_007
  int $81
  mov %esi gsfc_006
  int $81
  mov %ecx 7 ; 8 colors
  mov %eax $30
.gsfcL2:
  push '^[' int $2
  push '['  int $2
  push '1'  int $2
  push '0'  int $2
  push %eax  int $2
  push 'm'  int $2
  push ' '  int $2
  push ' '  int $2
  inx %eax
  loop .gsfcL2
  mov %esi gsfc_007
  int $81

  mov %esi gsfc_logo
  int $81

  jmp shell.aftexec

gsfc_000:    bytes "             ^[[97mgsfetch$^[[0m             ---------$^@"
gsfc_001:    bytes "             ^[[97mHost: ^[[0m^@"
gsfc_002:    bytes "$             ^[[97mOS: ^[[0m^@"
gsfc_003:    bytes "$             ^[[97mCPU: ^[[0m^@"
gsfc_004:    bytes "             ^[[97mMemory: ^[[0m^@"
gsfc_005:    bytes " B/16 MiB$^@"
gsfc_006:    bytes "             ^@"
gsfc_007:    bytes "^[[0m$^@"
gsfc_008:    bytes "             ^[[97mDisk space used: ^[[0m^@"
gsfc_009:    bytes " B/16 MiB$$^@"
gsfc_logo:   bytes "^[[10A^[[33m  .     . .$"
             bytes            "     A     .$"
             bytes            "    (=) .$"
             bytes            "  (=====)$"
             bytes            " (========)^[[0m$$$$$$^@"
