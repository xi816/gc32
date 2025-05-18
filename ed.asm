  jmp start

strcpy:
  lodb %esi %eax
  cmp %eax $00
  re
  stob %egi %eax
  jmp strcpy

strtok:
  lodb %esi %eax
  cmp %eax %ebx
  re
  jmp strtok

start:
  ; Инициализация буфера
  mov %esi buf_start
  mov %eax 0        ; Нулевой байт
  stob %esi %eax    ; [buf_start] = 0 (пустая строка)

main_loop:
  ; Вывод приглашения
  mov %esi prompt
  call puts
  
  ; Ввод команды (2 символа)
  mov %esi cmd_buf
  mov %ecx 10
  call scans
  mov %esi cmd_buf
  
  ; Проверка команды
  lodb %esi %eax     ; Первый символ
  lodb %esi %ebx     ; Второй символ
  
  ; Команда выхода (q)
  cmp %eax 'q'
  re
  
  ; Команда вывода (p)
  cmp %ebx 'p'
  je .print_cmd
  
  ; Команда добавления текста (a)
  cmp %ebx 'a'
  je .add_cmd_real
  
  jmp main_loop

.print_cmd:
  ; Вывод содержимого буфера
  mov %esi buf_start
  call puts
  push '$'
  int 2
  jmp main_loop

.add_cmd_real:
  mov %egi buf_start
  mov %ebx $0A
  call strtok
.add_cmd:
  mov %esi insert_buf
  mov %ecx 256
  call scans
  
  ; Добавление перевода строки
  mov %eax '$'
  stob %esi %eax
  mov %eax '^@'
  stob %esi %eax

  ; Проверка на точку (конец ввода)
  mov %esi insert_buf
  lodb %esi %eax
  cmp %eax '.'
  je main_loop
  dex %esi
  call strcpy
  jmp .add_cmd
  
; Функция вывода строки
puts:
  lodb %esi %eax
  cmp %eax 0
  re
  push %eax
  int 2
  jmp puts

; Функция ввода строки
scans:
  cmp %ecx 0
  re
  int 1
  pop %eax
  push %eax
  int 2
  cmp %eax '$'      ; Проверка на перевод строки
  re
  stob %esi %eax
  dex %ecx
  jmp scans

; Данные
prompt: bytes "ed> ^@"
cmd_buf: reserve 16 bytes            ; Буфер для команды (2 байта)
buf_start: reserve 1024 bytes ; Основной буфер
buf_end: bytes 0              ; Маркер конца буфера
insert_buf: reserve 256 bytes

