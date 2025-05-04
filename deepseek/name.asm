start:
    ; Вывод приглашения
    mov %esi prompt
    call puts
    
    ; Ввод имени
    mov %esi buf
    mov %ecx 100
    call scans
    
    ; Вывод результата
    mov %esi result
    call puts
    
    ; Вывод имени
    mov %esi buf
    call puts
    
    ; Перевод строки и завершение
    push '$'
    int 2
    hlt

; Функция вывода строки
puts:
    lodb %esi %eax
    cmp %eax 0
    re
    push %eax
    int 2
    jmp puts

; Функция ввода строки с echo
scans:
    cmp %ecx 0
    re
    
    int 1
    pop %eax
    push %eax
    int 2
    
    cmp %eax 10        ; LF - конец ввода
    re
    
    stob %esi %eax
    dex %ecx
    jmp scans

; Данные
prompt: bytes "Enter your name: ^@"
result: bytes "Your name is: ^@"
buf: reserve 100 bytes
