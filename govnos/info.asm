info_main:
  mov %esi govnos_info
  int $81
  ret

govnos_info: bytes "^[[96m"
             bytes "GovnOS version: 0.7.0-32$"
             bytes "Release date: 05.09.E9 (2025-05-09)$"
             bytes "(c) Xi816, 253*8+1$"
             bytes "^[[0m^@"
