ifeq ($(OS),Windows_NT)
    RM = del /Q
    EXT = .exe
    MKDIR = if not exist build mkdir build
else
    RM = rm -f
    EXT =
    MKDIR = mkdir -p build
endif


CC = gcc

SRC = $(wildcard src/*.c src/Compiler/*.c src/Compiler/Codegen/*.c src/Compiler/Optimization/*.c src/Lexer/*.c src/Parser/*.c src/VirtualMachine/*.c)
OBJ = $(SRC:.c=.o)
OUT = build/Rebekah$(EXT)

CFLAGS = -Wall -std=c23 -g

all: $(OUT)

run: $(OUT)
	$(OUT)

$(OUT): $(OBJ)
	$(MKDIR)
	$(CC) $(OBJ) -o $(OUT)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	del /Q $(OUT) $(OBJ)
