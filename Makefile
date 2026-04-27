# Executável
TARGET := forthress

# Ferramentas
ASM := nasm
LD  := ld

# Flags
ASMFLAGS := -f elf64 -g -F dwarf
LDFLAGS  := 

# Diretórios
OBJ_DIR   := build
UTILS_DIR := utils

# Fontes
SRCS := main.asm \
        $(UTILS_DIR)/cfa.asm \
        $(UTILS_DIR)/dict.asm \
        $(UTILS_DIR)/io.asm

# Objetos
OBJS := $(OBJ_DIR)/main.o \
        $(OBJ_DIR)/utils/cfa.o \
        $(OBJ_DIR)/utils/dict.o \
        $(OBJ_DIR)/utils/io.o

# Includes
INCLUDES := macros.inc utils.inc words.inc

# Regra padrão
all: $(TARGET)

# Linkagem
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# main.asm
$(OBJ_DIR)/main.o: main.asm $(INCLUDES)
	@mkdir -p $(OBJ_DIR)
	$(ASM) $(ASMFLAGS) -o $@ $<

# utils/*.asm
$(OBJ_DIR)/utils/%.o: $(UTILS_DIR)/%.asm $(INCLUDES)
	@mkdir -p $(OBJ_DIR)/utils
	$(ASM) $(ASMFLAGS) -o $@ $<

# Limpeza
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
