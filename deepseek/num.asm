start:
    ; Вывод приглашения
    mov %esi prompt
    call puts
    
    ; Ввод числа
    call scan_num      ; Результат в %eax
    
    ; Сохраняем число
    push %eax
    
    ; Вывод результата
    mov %esi result
    call puts

    ; Восстанавливаем и выводим число
    pop %eax
    call print_num
    
    ; Перевод строки и завершение
    push '$'
    int 2
    hlt

; Функции вывода и ввода строк
puts:
    lodb %esi %eax
    cmp %eax 0
    re
    push %eax
    int 2
    jmp puts

scans:
    cmp %ecx 0
    re
    int 1
    pop %eax
    push %eax
    int 2
    cmp %eax 10
    re
    stob %esi %eax
    dex %ecx
    jmp scans

; Функция ввода числа (возвращает в %eax)
scan_num:
    push %ebx
    push %ecx
    push %egi
    
    mov %eax 0
    mov %ebx 10
    mov %egi buf
    mov %ecx 12
    
    ; Вводим строку
    mov %esi %egi
    call scans
    
    ; Конвертируем строку в число
.convert_loop:
    lodb %egi %ecx
    cmp %ecx 0
    re
    
    sub %ecx '0'
    mul %eax %ebx
    add %eax %ecx
    jmp .convert_loop
    
    pop %egi
    pop %ecx
    pop %ebx
    re

; Функция вывода числа (из %eax)
print_num:
    push %ebx
    push %ecx
    push %edx
    push %esi
    
    mov %ebx 10
    mov %esi buf
    add %esi 11       ; Конец буфера
    
    ; Установка нуль-терминатора
    mov %ecx 0
    stob %esi %ecx
    
    ; Обработка нуля
    cmp %eax 0
    jne .non_zero
    dex %esi
    mov %ecx '0'
    stob %esi %ecx
    jmp .print
    
.non_zero:
    ; Разбираем число на цифры
.div_loop:
    cmp %eax 0
    re
    
    div %eax %ebx     ; %eax /= 10, %edx = %eax % 10
    add %edx '0'
    dex %esi
    stob %esi %edx
    jmp .div_loop
    
.print:
    ; Выводим строку
    call puts
    
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    re

; Данные
prompt: bytes "Enter a number: ^@"
result: bytes "You entered: ^@"
buf: reserve 16 bytes
