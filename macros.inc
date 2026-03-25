%define LAST_WORD 0 

%macro colon 3
    section .data
        w_ %+ %2: 
            dq LAST_WORD 
            db %1, 0 
            db %3
        xt_ %+ %2: 
            dq i_docol
        %define LAST_WORD w_ %+ %2 
%endmacro

%macro colon 2
    colon %1, %2, 0
%endmacro

%macro native 3
    section .data
        w_ %+ %2:
            dq LAST_WORD
            db %1, 0
            db %3
        xt_ %+ %2:
            dq i_ %+ %2
        %define LAST_WORD w_ %+ %2
    section .text
        i_ %+ %2:
%endmacro

%macro native 2
    native %1, %2, 0
%endmacro

%macro rpush 1
    sub rstack, 8
    mov qword [rstack], %1
%endmacro

%macro rpop 1
    mov %1, qword [rstack]
    add rstack, 8
%endmacro

%macro branch 1 
dq xt_branch
dq %1 - $ - 8
%endmacro

%macro branch0 1 
dq xt_branch0
dq %1 - $ - 8
%endmacro
