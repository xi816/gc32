gtutor_main:
  mov %esi gtutor_msg00
  int $81

  int $01
  pop %edx

  ret

gtutor_msg00: bytes "^[[92mWelcome to GovnOS tutorial! Press Return to start^[[0m$^[[91m  will be available tomorrow (05.0B.E9)^[[0m$^@"
gtutor_msg01: bytes "^[[H^[[2JThe gsfetch command shows your computer characteristics$> gsfetch$$^@"

