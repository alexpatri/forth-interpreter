native "hello", hello 
    mov rdi, hello_msg
    call print
    jmp next

native "bye", bye
    mov rax, 60
    xor rdi, rdi
    syscall

native "+", add
    pop rax
    add [rsp], rax
    jmp next

native "-", sub
    pop rax
    sub [rsp], rax
    jmp next

native "*", mul
    pop rax
    pop rdx
    imul rdx
    push rax
    jmp next

native "/", div
    pop rax
    pop rcx
    cqo         ; replica o bit de sinal de RAX em todos os bits de RDX
    idiv rcx
    push rax
    jmp next

native "=", is_equal
    pop rax
    cmp rax, [rsp]
    je .equal

    mov qword [rsp], 0
    jmp next

    .equal:
    mov qword [rsp], 1
    jmp next

native "<", is_less
    pop rax
    cmp rax, [rsp]
    jl .less

    mov qword [rsp], 0
    jmp next

    .less:
    mov qword [rsp], 1
    jmp next

native "not", not
    pop rax
    test rax, rax
    jz .zero

    push 0
    jmp next

    .zero:
    push 1
    jmp next

native "and", and
    pop rax
    pop rdx

    test rax, rax
    jz .false

    test rdx, rdx
    jz .false

    push 1
    jmp next

    .false:
    push 0
    jmp next

native ".", pop
    pop rdi
    call print_int
    call print_newline
    jmp next

native ".S", show_stack
    mov rcx, [stack_base]
    sub rcx, 8

    .loop:
    cmp rcx, rsp
    jb .end 

    mov rdi, [rcx]
    push rcx

    call print_int
    mov rdi, " "
    call print_char

    pop rcx
    sub rcx, 8
    jmp .loop

    .end:
    call print_newline
    jmp next

native "rot", rot
    pop rax
    pop rdx
    pop rcx
    push rdx
    push rax
    push rcx
    jmp next

native "swap", swap
    pop rax
    mov rdx, [rsp]
    mov [rsp], rax
    push rdx
    jmp next

native "dup", dup
    push qword [rsp]
    jmp next

native "drop", drop
    add rsp, 8
    jmp next

native "key", key
    call read_char
    push rax
    jmp next

native "emit", emit
    pop rdi
    call print_char
    jmp next

native "docol", docol
    rpush pc
    add w, 8
    mov pc, w
    jmp next

native "exit", exit
    rpop pc
    jmp next

native "mem", mem
    push qword user_memory
    jmp next

native "inbuf", inbuf
    push qword input_buffer
    jmp next

native "state", state
    push qword state
    jmp next

native "here", here
    push qword here
    jmp next

native "lit", lit
    push qword [pc]
    add pc, 8
    jmp next

native ",", comma
    mov rax, [here]
    pop qword [rax]
    add qword [here], 8
    jmp next

native "word", word
    pop rdi
    call read_word
    push rdx
    jmp next

native "branch", branch
    add pc, [pc]
    add pc, 8
    jmp next

native "0branch", branch0
    pop rax
    test rax, rax
    jnz .skip
    add pc, [pc]
    .skip:
    add pc, 8
    jmp next

native "!", store
    pop rax
    pop rdx
    mov [rdx], rax
    jmp next

native "@", fetch
    pop rax
    push qword [rax]
    jmp next

native "c@", fetch_char
    pop rax
    movzx rax, byte [rax]
    push rax
    jmp next

native "create", create
    mov rcx, [last_word]
    mov rsi, [here]
    mov [rsi], rcx
    mov [last_word], rsi
    add rsi, 8

    pop rdi
    push rsi
    mov rdx, 255
    call string_copy
    pop rsi
    push rsi
    mov rdi, rsi
    call strlen
    pop rsi
    add rsi, rax

    ; flags
    inc rsi
    pop rax
    mov [rsi], al
    inc rsi
     
    mov [here], rsi
    jmp next

native "find", find
    pop rdi
    mov rsi, last_word
    call find_word
    push rax
    jmp next

native "cfa", cfa
    pop rdi
    call code_from_addr
    push rax
    jmp next

native "number", number
    pop rdi
    call parse_int
    push rax
    push rdx
    jmp next

native "print", print
    pop rdi
    call print
    jmp next

native "exec", exec
    pop rax
    mov w, rax
    jmp [rax]

native "r>", rpop
    rpop rax
    push rax
    jmp next

native ">r", rpush
    pop rax
    rpush rax
    jmp next

native "r@", rcopy
    push qword [rstack]
    jmp next

colon ">", is_greater
    dq xt_swap
    dq xt_is_less
    dq xt_exit

colon "<=", is_less_or_equal
    dq xt_is_greater
    dq xt_not
    dq xt_exit

colon ">=", is_greater_or_equal
    dq xt_is_less
    dq xt_not
    dq xt_exit

colon "or", or
    dq xt_not
    dq xt_swap
    dq xt_not
    dq xt_and
    dq xt_not
    dq xt_exit

colon ":", colon
    .read:
    dq xt_inbuf
    dq xt_word
    branch0 .read
    dq xt_lit, 0
    dq xt_inbuf
    dq xt_create
    dq xt_state
    dq xt_lit, 1
    dq xt_store
    dq xt_lit, i_docol
    dq xt_comma
    dq xt_exit

colon ";", semicolon, 1
    dq xt_state
    dq xt_lit, 0
    dq xt_store
    dq xt_lit, xt_exit
    dq xt_comma
    dq xt_exit

native "init", init
    mov rstack, rstack_start
    mov [stack_base], rsp
    mov pc, program_stub
    jmp next 

colon "interpreter", interpreter
.start:
    dq xt_inbuf
    dq xt_word
    branch0 .end

    dq xt_inbuf
    dq xt_find
    dq xt_dup
    branch0 .number

    dq xt_cfa

    dq xt_state, xt_fetch
    branch0 .execute

    ; se a palavra estivar marcada como imediata é
    ; executada mesmo em modo de compilação
    dq xt_dup
    dq xt_lit, 1
    dq xt_sub
    dq xt_fetch_char
    dq xt_not
    branch0 .execute

    dq xt_comma
    branch .start

.execute: 
    dq xt_exec
    branch .start

.number:
    dq xt_drop
    dq xt_inbuf
    dq xt_number
    branch0 .not_found

    dq xt_state, xt_fetch
    branch0 .start

    dq xt_here
    dq xt_lit, 8
    dq xt_sub
    dq xt_lit, xt_branch0
    dq xt_is_equal
    dq xt_not
    branch0 .is_branch

    dq xt_here
    dq xt_lit, 8
    dq xt_sub
    dq xt_lit, xt_branch
    dq xt_is_equal
    dq xt_not
    branch0 .is_branch
 
    dq xt_lit, xt_lit, xt_comma

.is_branch:
    dq xt_comma
    branch .start

.not_found:
    dq xt_drop
    dq xt_lit, error_msg 
    dq xt_print
    branch .start

.end:
    dq xt_bye

