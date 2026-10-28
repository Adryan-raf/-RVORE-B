# Makefile da Árvore B
#
# Estrutura esperada:
#   implementacoes/*.c   código-fonte dos módulos (criacao, busca, ...)
#   includes/*.h         cabeçalhos
#   main.c               programa de demonstração
#   teste.c              teste aleatório (opcional)
#
# Uso:
#   make          compila ./arvore_b (e ./teste, se teste.c existir)
#   make run      compila e executa a demonstração (ordem 5)
#   make test     compila e executa o teste aleatório
#   make clean    remove binários e objetos

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -g -Iincludes

SRC_DIR   = implementacoes
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
HDRS = $(wildcard includes/*.h)

TARGETS = arvore_b
ifneq ($(wildcard teste.c),)
TARGETS += teste
endif

all: $(TARGETS)

arvore_b: $(BUILD_DIR)/main.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

teste: $(BUILD_DIR)/teste.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# módulos em implementacoes/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# main.c e teste.c na raiz
$(BUILD_DIR)/%.o: %.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: arvore_b
	./arvore_b 5

test: teste
	./teste

clean:
	rm -rf $(BUILD_DIR) arvore_b teste

.PHONY: all run test clean
