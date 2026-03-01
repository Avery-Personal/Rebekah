CC = gcc

SRC = $(wildcard src/*.c src/Compiler/*.c src/Compiler/Codegen/*.c src/Lexer/*.c src/Parser/*.c src/VirtualMachine/*.c)
OBJ = $(SRC:.c=.o)
OUT = build/Rebekah.exe

CFLAGS = -Wall -std=c23 -g -fsanitize=address

all: $(OUT)

run: $(OUT)
	$(OUT)

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	del /Q $(OUT) $(OBJ)
