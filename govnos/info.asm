info_main:
  mov %esi govnos_info
  int $81
  ret

govnos_info: bytes "^[[96m"
             bytes "GovnOS version: 0.6.0-32$"
             bytes "Release date: 04.1E.E9 (2025-04-30)$"
             bytes "(c) Xi816, 253*8+1$"
             bytes "^[[0m^@"
