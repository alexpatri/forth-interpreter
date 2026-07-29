# forthress

Um interpretador/compilador da linguagem **Forth** escrito do zero em Assembly x86-64 (NASM) para Linux, sem libc — apenas syscalls.

O objetivo do projeto é didático: construir um dialeto Forth mínimo mas funcional, com pilha de dados, pilha de retorno, dicionário extensível em tempo de execução e modo de compilação.

## Requisitos

- `nasm`
- `ld` (binutils)
- `make`
- Linux x86-64

## Build

```sh
make          # compila e gera o executável ./forthress
make clean    # remove build/ e o executável
```

Os objetos são gerados em `build/` e o binário final na raiz do projeto.

## Uso

```sh
./forthress
```

O REPL exibe o prompt `>>` e lê palavras de `stdin`:

```
>> 5 3 + .
8
>> : quadrado dup * ;
>> 5 quadrado .
25
>> bye
```

## Estrutura

| Arquivo | Conteúdo |
|---|---|
| `main.asm` | ponto de entrada, seções `.data`/`.bss`, rotina `next` (o núcleo do threaded code) |
| `macros.inc` | macros `native`, `colon`, `rpush`/`rpop`, `branch`/`branch0` |
| `words.inc` | definição de todas as palavras do dialeto e o loop do interpretador |
| `utils.inc` | declarações `extern` das rotinas auxiliares |
| `utils/io.asm` | entrada/saída e strings: `print`, `read_word`, `parse_int`, `strlen`, ... |
| `utils/dict.asm` | `find_word` — busca uma palavra no dicionário |
| `utils/cfa.asm` | `code_from_addr` — do cabeçalho da palavra para seu execution token |
| `Makefile` | regras de compilação e linkagem |

## Como funciona

### Threaded code indireto

A execução usa três registradores dedicados:

| Registrador | Papel |
|---|---|
| `r15` (`pc`) | ponteiro para o próximo execution token a executar |
| `r14` (`w`) | palavra atualmente em execução |
| `r13` (`rstack`) | topo da pilha de retorno |

O motor é a rotina `next` em `main.asm`:

```asm
next:
    mov w, [pc]   ; carrega o próximo xt
    add pc, 8     ; avança
    jmp [w]       ; salta para o código da palavra
```

A pilha de dados é a própria pilha do processador (`rsp`); a pilha de retorno é uma região separada em `.bss`, manipulada por `rpush`/`rpop`.

O laço do REPL é fechado por `program_stub`: uma célula vazia seguida do xt de `interpreter`, de modo que ao terminar a interpretação o `next` volta para o interpretador.

### Dicionário

Cada palavra é uma entrada de lista ligada em memória:

```
[ ponteiro para a palavra anterior (8 bytes) ]
[ nome terminado em \0                       ]
[ byte de flags (1 = imediata)               ]
[ xt — execution token                       ]
```

A variável `last_word` aponta para a entrada mais recente. Palavras definidas em tempo de execução (via `:`) são gravadas em `dict_memory`, com `here` apontando para a próxima posição livre.

Duas macros criam as palavras embutidas:

- `native "nome", label` — palavra implementada diretamente em assembly
- `colon "nome", label` — palavra composta por uma lista de xts, iniciada por `docol` e terminada por `exit`

### Bootstrapping

O loop de interpretação (`interpreter`, em `words.inc`) não é escrito em assembly puro: ele é uma palavra *colon* construída com as próprias palavras do dialeto (`word`, `find`, `cfa`, `state`, `exec`, `comma`, `number`, `branch`...). Ele lê uma palavra, procura no dicionário e então:

- em **modo de interpretação** (`state = 0`): executa a palavra, ou empilha o número;
- em **modo de compilação** (`state = 1`): compila o xt na definição atual, a menos que a palavra esteja marcada como imediata (caso de `;`);
- se não for palavra conhecida nem número: imprime `The provided word does not exist.`

## Palavras disponíveis

**Aritmética e lógica**
`+` `-` `*` `/` `=` `<` `>` `<=` `>=` `not` `and` `or`

**Pilha**
`dup` `drop` `swap` `rot` `.S` `>r` `r>` `r@`

**I/O**
`.` `key` `emit` `print` `word` `input` `hello` `prompt`

**Memória e variáveis**
`@` `!` `c@` `,` `mem` `inbuf` `here` `state`

**Dicionário e compilação**
`:` `;` `create` `find` `cfa` `number` `lit` `exec` `docol` `exit`

**Controle**
`branch` `0branch`

**Sistema**
`init` `interpreter` `bye`
