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

#define MAX_PATH 1024
#define MAX_OUTPUT 65536

static int TestsRun = 0;
static int TestsPassed = 0;
static int TestsFailed = 0;

void RunTests(const char *Path);

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

#ifndef _WIN32
    #include <dirent.h>
    #include <sys/stat.h>

    static char *RunTestFile(const char *Path, const char *Binary) {
        char Command[MAX_PATH * 2];

        snprintf(Command, sizeof(Command), "%s \"%s\" 2>&1", Binary, Path);

        FILE *Pipe = popen(Command, "r");
        if (!Pipe)
            return NULL;

        char *Output = malloc(MAX_OUTPUT);
        if (!Output) {
            pclose(Pipe);
            
            return NULL;
        }

        size_t Total = 0;

        char Chunk[512];

        while (fgets(Chunk, sizeof(Chunk), Pipe) && Total < MAX_OUTPUT - 1) {
            size_t len = strlen(Chunk);

            memcpy(Output + Total, Chunk, len);

            Total += len;
        }

        Output[Total] = '\0';

        pclose(Pipe);

        return Output;
    }

    static void TrimTrailing(char *String) {
        int len = (int) strlen(String);

        while (len > 0 && (String[len - 1] == '\n' || String[len - 1] == '\r' || String[len - 1] == ' ')) {
            String[--len] = '\0';
        }
    }

    static void RunTest(const char *Path, const char *Binary) {
        char ExpectedPath[MAX_PATH];

        strncpy(ExpectedPath, Path, MAX_PATH - 1);

        ExpectedPath[MAX_PATH - 1] = '\0';

        char *Extension = strstr(ExpectedPath, ".rbk");
        if (!Extension)
            return;

        strcpy(Extension, ".Expected");

        FILE *Check = fopen(ExpectedPath, "r");
        if (!Check) {
            printf("  [SKIP] %s  (no .Expected file)\n", Path);

            return;
        }

        fclose(Check);

        TestsRun++;

        char *Expected = ReadFile(ExpectedPath);
        char *Actual = RunTestFile(Path, Binary);

        if (!Expected || !Actual) {
            printf("  [ERROR] %s  (could not read files)\n", Path);

            TestsFailed++;

            free(Expected);
            free(Actual);

            return;
        }

        TrimTrailing(Expected);
        TrimTrailing(Actual);

        if (strcmp(Expected, Actual) == 0) {
            printf("  [PASS] %s\n", Path);

            TestsPassed++;
        } else {
            printf("  [FAIL] %s\n", Path);
            printf("         Expected : %s\n", Expected);
            printf("         Actual   : %s\n", Actual);

            TestsFailed++;
        }

        free(Expected);
        free(Actual);
    }

    static void WalkDirectory(const char *DirectoryPath, const char *Binary) {
        DIR *Directory = opendir(DirectoryPath);
        if (!Directory) {
            fprintf(stderr, "Warning: could not open directory '%s'\n", DirectoryPath);

            return;
        }

        struct dirent *Entry;

        char Subdirectories[64][MAX_PATH];
        char RebekahFiles[256][MAX_PATH];

        int SubdirectoryCount = 0;
        int RebekahFileCount = 0;

        while ((Entry = readdir(Directory)) != NULL) {
            if (Entry -> d_name[0] == '.')
                continue;

            char FullPath[MAX_PATH];

            snprintf(FullPath, sizeof(FullPath), "%s/%s", DirectoryPath, Entry -> d_name);

            struct stat Stat;
            if (stat(FullPath, &Stat) != 0)
                continue;

            if (S_ISDIR(Stat.st_mode) && SubdirectoryCount < 64) {
                strncpy(Subdirectories[SubdirectoryCount++], FullPath, MAX_PATH - 1);
            } else if (S_ISREG(Stat.st_mode) && RebekahFileCount < 256) {
                const char *ext = strrchr(Entry -> d_name, '.');
                
                if (ext && strcmp(ext, ".rbk") == 0) {
                    strncpy(RebekahFiles[RebekahFileCount++], FullPath, MAX_PATH - 1);
                }
            }
        }

        closedir(Directory);

        for (int i = 0; i < RebekahFileCount; i++)
            RunTest(RebekahFiles[i], Binary);

        for (int i = 0; i < SubdirectoryCount; i++) {
            printf("\n-- %s --\n", Subdirectories[i]);

            WalkDirectory(Subdirectories[i], Binary);
        }
    }

    void RunTests(const char *Path) {
        TestsRun = TestsPassed = TestsFailed = 0;

        WalkDirectory("tests/Workflow", Path);

        printf("  %d / %d tests passed", TestsPassed, TestsRun);
        if (TestsFailed > 0)
            printf("  |  %d FAILED", TestsFailed);

        if (TestsFailed > 0)
            exit(1);
    }
#elifdef _WIN32
    void RunTests(const char *Path) {
        int i = 0;
    }
#endif

int main(int argc, char **argv) {
    if (argc > 1) {
        if (strcmp(argv[1], "--run-tests") == 0) {
            RunTests(argv[0]);
        }

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
