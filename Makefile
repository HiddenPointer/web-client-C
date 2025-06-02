# Makefile para proyecto web-client-C en RHEL 8.1 con GCC

# Herramientas
CC = gcc
AR = ar

# Flags de compilación
CFLAGS = -std=c99 -Wall -Wextra -O3 -pthread -D_XOPEN_SOURCE=700 $(shell pkg-config --cflags libcurl)
LDFLAGS = $(shell pkg-config --libs libcurl) -lcrypto

# Directorios
SRC_DIR = src
EX_DIR  = examples

# Librería estática
LIB = librest_client.a

# Fuentes y objetos incluyendo Parson
SRC = $(SRC_DIR)/rest_client.c $(SRC_DIR)/lib/parson.c
OBJ = $(SRC_DIR)/rest_client.o $(SRC_DIR)/lib/parson.o

# Ejecutables
EX_SRCS = $(EX_DIR)/obp_example.c $(EX_DIR)/benchMark.c
EX_OBJS = $(EX_DIR)/obp_example.o $(EX_DIR)/benchMark.o
EXES    = obp_example benchMark

.PHONY: all clean
all: $(LIB) $(EXES)

# Construir librería estática
$(LIB): $(OBJ)
	$(AR) rcs $@ $^

# Compilar fuentes de src
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/rest_client.h
	$(CC) $(CFLAGS) -c $< -o $@
lib/parson.o: lib/parson.c lib/parson.h
	$(CC) $(CFLAGS) -c $< -o $@

# Regla genérica para ejemplos
$(EX_DIR)/%.o: $(EX_DIR)/%.c $(SRC_DIR)/rest_client.h
	$(CC) $(CFLAGS) -c $< -o $@

# Enlazar ejecutables
obp_example: $(EX_DIR)/obp_example.o $(LIB)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

benchMark: $(EX_DIR)/benchMark.o $(LIB)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Limpiar artefactos
clean:
	rm -f $(OBJ) $(LIB) $(EX_OBJS) $(EXES)
