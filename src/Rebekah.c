#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Lexer/Lexer.h"
#include "Lexer/Tokens.h"
#include "Parser/Parser.h"
#include "Compiler/Semantics.h"
#include "Compiler/IR.h"
#include "Compiler/Codegen/IRGen.h"
#include "Compiler/Optimization/IROptimization.h"
#include "VirtualMachine/VirtualMachine.h"

#define REPL_BUFFER 4096

char *ReadFile(const char *Path) {
    FILE *File = fopen(Path, "rb");
    if (!File)
        return NULL;

    fseek(File, 0, SEEK_END);

    long _Size = ftell(File);

    rewind(File);

    char *Buffer = malloc(_Size + 1);

    fread(Buffer, 1, _Size, File);

    Buffer[_Size] = '\0';

    fclose(File);

    return Buffer;
}

int RunSource(char *Source) {
    Lexer _Lexer = LexerCreate(Source);
    TokenStream Tokens = Tokenize(&_Lexer);

    Parser _Parser = CreateParser(&Tokens);
    ASTProgram *Program = ParseProgram(&_Parser);

    if (_Parser.HasError)
        return 0;

    SemanticAnalyzer *Analyzer = CreateSemanticAnalyzer();

    if (!AnalyzeProgram(Analyzer, Program)) {
        DestroySemanticAnalyzer(Analyzer);

        return 0;
    }

    IRProgram *IR = GenerateIR(Program);

    OptimizeProgram(IR);

    BytecodeProgram *Bytecode = LowerProgram(IR);
    VirtualMachine *VM = VirtualMachineCreate(Bytecode);

    VirtualMachineExecute(VM);

    DestroySemanticAnalyzer(Analyzer);

    return 1;
}

void StartREPL() {
    char Buffer[REPL_BUFFER];

    printf("Rebekah REPL v0.7\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        printf(">>> ");

        if (!fgets(Buffer, sizeof(Buffer), stdin))
            break;

        if (strncmp(Buffer, "exit", 4) == 0)
            break;

        RunSource(Buffer);
    }
}

int main(int argc, char **argv) {
    if (argc > 1) {
        char *Source = ReadFile(argv[1]);
        if (!Source) {
            printf("Failed to read file: %s\n", argv[1]);
            
            return 1;
        }

        RunSource(Source);

        free(Source);
    } else {
        StartREPL();
    }

    return 0;
}
