%include "macros.inc"
%include "utils.inc"

%define MAX_WORD_SIZE 1024
%define MEMORY_SIZE 65536

%define pc r15
%define w r14
%define rstack r13

%include "words.inc"

section .data
    hello_msg: db "Hello, World!", 10, 0
    error_msg: db "The provided word does not exist.", 0

    stack_base: dq 0

    ; program_stub é dividido em duas partes
    ; primeiro um espaço vazio para um xt (onde fica armazenado o xt da palavra lida em stdin)
    ; sedundo o xt do interpretador (é dessa forma que é feito o loop em next)
    program_stub: dq 0
    xt_interpreter: dq .interpreter
    .interpreter: dq i_interpreter

    ; necessário para modo de compilação
    state: dq 0
    here: dq dict_memory
    last_word: dq LAST_WORD

section .bss
    ; pilha para 'colon'
    resq 1023
    rstack_start: resq 1

    input_buffer: resb MAX_WORD_SIZE

    user_memory: resq MEMORY_SIZE
    dict_memory: resq MEMORY_SIZE

section .text
global _start

next:
    mov w, [pc]
    add pc, 8
    jmp [w]

_start: jmp i_init

