; Rebekah | Copyright 2026 © AveriC
section .text
global CreateInteger
global NextCounter
global IncrementCounter
global Main

CreateInteger:
    push rbp
    mov rbp, rsp
    sub rsp, 256
    mov rax, [rbp-0]
    mov [rbp-0], rax
    mov rax, [rbp-0]
    mov rsp, rbp
    pop rbp
    ret

NextCounter:
    push rbp
    mov rbp, rsp
    sub rsp, 256
    mov rax, [rbp-0]
    mov [rbp-0], rax
    mov QWORD [rbp-8], 0
    mov rax, [rbp-0]
    mov rbx, [rbp-8]
    add rax, rbx
    mov [rbp-16], rax
    mov rax, [rbp-16]
    mov [rbp-0], rax

IncrementCounter:
    push rbp
    mov rbp, rsp
    sub rsp, 256
    mov rax, [rbp-0]
    mov [rbp-0], rax
    mov rax, [rbp-0]
    mov [rbp-8], rax
    mov rax, [rbp-0]
    mov rbx, [rbp-8]
    add rax, rbx
    mov [rbp-16], rax
    mov rax, [rbp-16]
    mov [rbp-0], rax

Main:
    push rbp
    mov rbp, rsp
    sub rsp, 256
    lea rax, [rbp-8]
    mov [rbp-0], rax
    lea rax, [rbp-272]
    mov [rbp-264], rax
    mov QWORD [rbp-528], 0
    mov rax, [rbp-264]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-536], rax
    mov rax, [rbp-528]
    mov rbx, [rbp-536]
    mov [rbx], rax
    mov QWORD [rbp-544], 0
    mov rax, [rbp-264]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-552], rax
    mov rax, [rbp-544]
    mov rbx, [rbp-552]
    mov [rbx], rax
    mov QWORD [rbp-560], 0
    mov rax, [rbp-264]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-568], rax
    mov rax, [rbp-560]
    mov rbx, [rbp-568]
    mov [rbx], rax
    mov rax, [rbp-0]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-576], rax
    mov rax, [rbp-264]
    mov rbx, [rbp-576]
    mov [rbx], rax
    lea rax, [rbp-592]
    mov [rbp-584], rax
    mov QWORD [rbp-848], 0
    mov rax, [rbp-584]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-856], rax
    mov rax, [rbp-848]
    mov rbx, [rbp-856]
    mov [rbx], rax
    mov QWORD [rbp-864], 0
    mov rax, [rbp-584]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-872], rax
    mov rax, [rbp-864]
    mov rbx, [rbp-872]
    mov [rbx], rax
    mov QWORD [rbp-880], 0
    mov rax, [rbp-584]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-888], rax
    mov rax, [rbp-880]
    mov rbx, [rbp-888]
    mov [rbx], rax
    mov rax, [rbp-0]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-896], rax
    mov rax, [rbp-584]
    mov rbx, [rbp-896]
    mov [rbx], rax
    lea rax, [rbp-912]
    mov [rbp-904], rax
    mov QWORD [rbp-1168], 0
    mov rax, [rbp-904]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-1176], rax
    mov rax, [rbp-1168]
    mov rbx, [rbp-1176]
    mov [rbx], rax
    mov QWORD [rbp-1184], 0
    mov rax, [rbp-904]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-1192], rax
    mov rax, [rbp-1184]
    mov rbx, [rbp-1192]
    mov [rbx], rax
    mov QWORD [rbp-1200], 0
    mov rax, [rbp-904]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-1208], rax
    mov rax, [rbp-1200]
    mov rbx, [rbp-1208]
    mov [rbx], rax
    mov rax, [rbp-0]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-1216], rax
    mov rax, [rbp-904]
    mov rbx, [rbp-1216]
    mov [rbx], rax
    mov rax, [rbp-0]
    mov [rbp-0], rax
    lea rax, [rbp-1232]
    mov [rbp-1224], rax
    lea rax, [rbp-1496]
    mov [rbp-1488], rax
    lea rax, [rbp-1760]
    mov [rbp-1752], rax
    mov QWORD [rbp-2016], 0
    mov rax, [rbp-1752]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2024], rax
    mov rax, [rbp-2016]
    mov rbx, [rbp-2024]
    mov [rbx], rax
    mov QWORD [rbp-2032], 0
    mov rax, [rbp-1752]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2040], rax
    mov rax, [rbp-2032]
    mov rbx, [rbp-2040]
    mov [rbx], rax
    mov rax, [rbp-1488]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2048], rax
    mov rax, [rbp-1752]
    mov rbx, [rbp-2048]
    mov [rbx], rax
    lea rax, [rbp-2064]
    mov [rbp-2056], rax
    mov QWORD [rbp-2320], 0
    mov rax, [rbp-2056]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2328], rax
    mov rax, [rbp-2320]
    mov rbx, [rbp-2328]
    mov [rbx], rax
    mov QWORD [rbp-2336], 0
    mov rax, [rbp-2056]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2344], rax
    mov rax, [rbp-2336]
    mov rbx, [rbp-2344]
    mov [rbx], rax
    mov rax, [rbp-1488]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2352], rax
    mov rax, [rbp-2056]
    mov rbx, [rbp-2352]
    mov [rbx], rax
    mov rax, [rbp-1224]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2360], rax
    mov rax, [rbp-1488]
    mov rbx, [rbp-2360]
    mov [rbx], rax
    lea rax, [rbp-2376]
    mov [rbp-2368], rax
    lea rax, [rbp-2640]
    mov [rbp-2632], rax
    mov QWORD [rbp-2896], 0
    mov rax, [rbp-2632]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2904], rax
    mov rax, [rbp-2896]
    mov rbx, [rbp-2904]
    mov [rbx], rax
    mov QWORD [rbp-2912], 0
    mov rax, [rbp-2632]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2920], rax
    mov rax, [rbp-2912]
    mov rbx, [rbp-2920]
    mov [rbx], rax
    mov rax, [rbp-2368]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-2928], rax
    mov rax, [rbp-2632]
    mov rbx, [rbp-2928]
    mov [rbx], rax
    lea rax, [rbp-2944]
    mov [rbp-2936], rax
    mov QWORD [rbp-3200], 0
    mov rax, [rbp-2936]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-3208], rax
    mov rax, [rbp-3200]
    mov rbx, [rbp-3208]
    mov [rbx], rax
    mov QWORD [rbp-3216], 0
    mov rax, [rbp-2936]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-3224], rax
    mov rax, [rbp-3216]
    mov rbx, [rbp-3224]
    mov [rbx], rax
    mov rax, [rbp-2368]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-3232], rax
    mov rax, [rbp-2936]
    mov rbx, [rbp-3232]
    mov [rbx], rax
    mov rax, [rbp-1224]
    mov rbx, [rbp-0]
    imul rbx, 8
    add rax, rbx
    mov [rbp-3240], rax
    mov rax, [rbp-2368]
    mov rbx, [rbp-3240]
    mov [rbx], rax
    mov rax, [rbp-1224]
    mov [rbp-0], rax
    mov QWORD [rbp-3248], 0
    mov rdi, [rbp-3248]
    call CreateInteger
    mov [rbp-3256], rax
    mov rax, [rbp-3256]
    mov [rbp-0], rax
    call NextCounter
    mov [rbp-3264], rax
    call NextCounter
    mov [rbp-3272], rax
    mov QWORD [rbp-3280], 0
    mov rax, [rbp-0]
    mov [rbp-3288], rax
    mov rdi, [rbp-3280]
    mov rsi, [rbp-3288]
    call output
    mov [rbp-3296], rax
    mov QWORD [rbp-3304], 0
    mov rdi, [rbp-3304]
    call IncrementCounter
    mov [rbp-3312], rax
    mov QWORD [rbp-3320], 0
    mov rax, [rbp-0]
    mov [rbp-3328], rax
    mov rdi, [rbp-3320]
    mov rsi, [rbp-3328]
    call output
    mov [rbp-3336], rax

