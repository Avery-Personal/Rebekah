#include <stdlib.h>
#include <string.h>

#include "x86_64.h"

const char *RegisterString(CodeGen *Code, const char *String) {
    for (size_t i = 0; i < Code -> StringCount; i++) {
        if (strcmp(Code -> Strings[i].String, String) == 0) {
            return Code -> Strings[i].Label;
        }
    }
    
    char *Label = malloc(32);

    snprintf(Label, 32, "str_%zu", Code -> StringCount);
    
    Code -> Strings = realloc(Code -> Strings, sizeof(*Code -> Strings) * (Code -> StringCount + 1));
    Code -> Strings[Code -> StringCount].String = String;
    Code -> Strings[Code -> StringCount].Label = Label;
    Code -> StringCount++;
    
    return Label;
}

CodeGen *CreateCodeGen(FILE *Output) {
    CodeGen *Code = malloc(sizeof(CodeGen));

    Code -> Output = Output;
    Code -> CurrentFunction = NULL;
    Code -> StackOffset = 0;
    Code -> VarLocations = NULL;
    Code -> VarLocationCount = 0;
    Code -> LabelCounter = 0;

    return Code;
}

void DestroyCodeGen(CodeGen *Code) {
    free(Code -> VarLocations);
    free(Code);
}

int GetVarLocation(CodeGen *Code, IRValue *Value) {
    for (size_t i = 0; i < Code -> VarLocationCount; i++) {
        if (Code -> VarLocations[i].Value == Value) {
            return Code -> VarLocations[i].Offset;
        }
    }

    return 0;
}

void SetVarLocation(CodeGen *Code, IRValue *Value, int Offset) {
    Code -> VarLocations = realloc(Code -> VarLocations, sizeof(*Code -> VarLocations) * (Code -> VarLocationCount + 1));
    Code -> VarLocations[Code -> VarLocationCount].Value = Value;
    Code -> VarLocations[Code -> VarLocationCount].Offset = Offset;
    Code -> VarLocationCount++;
}

const char *GetTempRegister(int TempID) {
    return "rax";
}

void GeneratePrologue(CodeGen *Code, IRFunction *Function) {
    fprintf(Code -> Output, "%s:\n", Function -> Name);
    fprintf(Code -> Output, "    push rbp\n");
    fprintf(Code -> Output, "    mov rbp, rsp\n");
    fprintf(Code -> Output, "    sub rsp, 256\n");
    
    Code -> StackOffset = 0;
}

void GenerateEpilogue(CodeGen *Code) {
    fprintf(Code -> Output, "    mov rsp, rbp\n");
    fprintf(Code -> Output, "    pop rbp\n");
    fprintf(Code -> Output, "    ret\n");
}

void GenerateInstruction(CodeGen *Code, IRInstruction *Instruction) {
    FILE *Output = Code -> Output;
    
    switch (Instruction -> Op) {
        case IR_CONST: {
            int Offset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, Offset);

            if (Instruction -> Destination -> Kind == VALUE_CONST && Instruction -> Destination -> Type == IR_TYPE_PTR) {
                const char *Label = RegisterString(Code, Instruction -> Destination -> Label);

                fprintf(Output, "    lea rax, [rel %s]\n", Label);
                fprintf(Output, "    mov [rbp-%d], rax\n", Offset);
            } else {
                fprintf(Output, "    mov QWORD [rbp-%d], %lld\n", Offset, Instruction -> Destination -> IntValue);
            }

            break;
        }
        
        case IR_MOVE: {
            int SourceOffset = GetVarLocation(Code, Instruction -> Source1);
            int DestinationOffset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, DestinationOffset);
            
            fprintf(Output, "    mov rax, [rbp-%d]\n", SourceOffset);
            fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);

            break;
        }
        
        case IR_ADD:
        case IR_SUB:
        case IR_MUL: {
            int LeftOffset = GetVarLocation(Code, Instruction -> Source1);
            int RightOffset = GetVarLocation(Code, Instruction -> Source2);
            int DestinationOffset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, DestinationOffset);
            
            fprintf(Output, "    mov rax, [rbp-%d]\n", LeftOffset);
            fprintf(Output, "    mov rbx, [rbp-%d]\n", RightOffset);
            
            if (Instruction -> Op == IR_ADD) {
                fprintf(Output, "    add rax, rbx\n");
            } else if (Instruction -> Op == IR_SUB) {
                fprintf(Output, "    sub rax, rbx\n");
            } else if (Instruction -> Op == IR_MUL) {
                fprintf(Output, "    imul rax, rbx\n");
            }
            
            fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);

            break;
        }
        
        case IR_DIV: {
            int LeftOffset = GetVarLocation(Code, Instruction -> Source1);
            int RightOffset = GetVarLocation(Code, Instruction -> Source2);
            int DestinationOffset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, DestinationOffset);
            
            fprintf(Output, "    mov rax, [rbp-%d]\n", LeftOffset);
            fprintf(Output, "    mov rbx, [rbp-%d]\n", RightOffset);
            fprintf(Output, "    cqo\n");
            fprintf(Output, "    idiv rbx\n");
            fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);

            break;
        }
        
        case IR_NEG: {
            int SourceOffset = GetVarLocation(Code, Instruction -> Source1);
            int DestinationOffset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, DestinationOffset);
            
            fprintf(Output, "    mov rax, [rbp-%d]\n", SourceOffset);
            fprintf(Output, "    neg rax\n");
            fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);

            break;
        }
        
        case IR_EQ:
        case IR_NE:
        case IR_LT:
        case IR_LE:
        case IR_GT:
        case IR_GE: {
            int LeftOffset = GetVarLocation(Code, Instruction -> Source1);
            int RightOffset = GetVarLocation(Code, Instruction -> Source2);
            int DestinationOffset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, DestinationOffset);
            
            fprintf(Output, "    mov rax, [rbp-%d]\n", LeftOffset);
            fprintf(Output, "    cmp rax, [rbp-%d]\n", RightOffset);
            
            const char *Set;

            if (Instruction -> Op == IR_EQ) Set = "sete";
            else if (Instruction -> Op == IR_NE) Set = "setne";
            else if (Instruction -> Op == IR_LT) Set = "setl";
            else if (Instruction -> Op == IR_LE) Set = "setle";
            else if (Instruction -> Op == IR_GT) Set = "setg";
            else Set = "setge";
            
            fprintf(Output, "    %s al\n", Set);
            fprintf(Output, "    movzx rax, al\n");
            fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);

            break;
        }
        
        case IR_LOAD: {
            int AddressOffset = GetVarLocation(Code, Instruction -> Source1);
            int DestinationOffset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, DestinationOffset);
            
            if (Instruction -> Source1 -> Kind == VALUE_VAR || Instruction -> Source1 -> Kind == VALUE_PARAM) {
                fprintf(Output, "    mov rax, [rbp-%d]\n", AddressOffset);
                fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);
            } else {
                fprintf(Output, "    mov rax, [rbp-%d]\n", AddressOffset);
                fprintf(Output, "    mov rax, [rax]\n");
                fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);
            }

            break;
        }
        
        case IR_STORE: {
            int AddressOffset = GetVarLocation(Code, Instruction -> Destination);
            int valOffset = GetVarLocation(Code, Instruction -> Source1);
            
            fprintf(Output, "    mov rax, [rbp-%d]\n", valOffset);
            
            if (Instruction -> Destination -> Kind == VALUE_VAR || Instruction -> Destination -> Kind == VALUE_PARAM) {
                fprintf(Output, "    mov [rbp-%d], rax\n", AddressOffset);
            } else {
                fprintf(Output, "    mov rbx, [rbp-%d]\n", AddressOffset);
                fprintf(Output, "    mov [rbx], rax\n");
            }

            break;
        }
        
        case IR_CALL: {
            const char *ArgRegisters[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
            
            for (size_t i = 0; i < Instruction -> ArgCount && i < 6; i++) {
                int ArgOffset = GetVarLocation(Code, Instruction -> Args[i]);

                fprintf(Output, "    mov %s, [rbp-%d]\n", ArgRegisters[i], ArgOffset);
            }
            
            for (int i = Instruction -> ArgCount - 1; i >= 6; i--) {
                int ArgOffset = GetVarLocation(Code, Instruction -> Args[i]);

                fprintf(Output, "    push QWORD [rbp-%d]\n", ArgOffset);
            }
            
            fprintf(Output, "    call %s\n", Instruction -> Source1 -> Label);
            
            if (Instruction -> ArgCount > 6) {
                int StackCleanup = (Instruction -> ArgCount - 6) * 8;

                fprintf(Output, "    add rsp, %d\n", StackCleanup);
            }
            
            if (Instruction -> Destination) {
                int DestinationOffset = Code -> StackOffset;

                Code -> StackOffset += 8;

                SetVarLocation(Code, Instruction -> Destination, DestinationOffset);

                fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);
            }

            break;
        }
        
        case IR_RETURN: {
            if (Instruction -> Destination) {
                int ReturnOffset = GetVarLocation(Code, Instruction -> Destination);

                fprintf(Output, "    mov rax, [rbp-%d]\n", ReturnOffset);
            }

            GenerateEpilogue(Code);

            break;
        }
        
        case IR_LABEL: {
            fprintf(Output, "%s:\n", Instruction -> Extra -> Label);

            break;
        }
        
        case IR_JMP: {
            fprintf(Output, "    jmp %s\n", Instruction -> Extra -> Label);

            break;
        }
        
        case IR_JMP_IF: {
            int ConditionOffset = GetVarLocation(Code, Instruction -> Source1);

            fprintf(Output, "    cmp QWORD [rbp-%d], 0\n", ConditionOffset);
            fprintf(Output, "    jne %s\n", Instruction -> Extra -> Label);

            break;
        }
        
        case IR_JMP_IF_FALSE: {
            int ConditionOffset = GetVarLocation(Code, Instruction -> Source1);

            fprintf(Output, "    cmp QWORD [rbp-%d], 0\n", ConditionOffset);
            fprintf(Output, "    je %s\n", Instruction -> Extra -> Label);

            break;
        }
        
        case IR_ARRAY_ALLOC: {
            int SizeOffset = GetVarLocation(Code, Instruction -> Source1);
            int DestinationOffset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, DestinationOffset);
            
            fprintf(Output, "    lea rax, [rbp-%d]\n", Code -> StackOffset);

            Code -> StackOffset += 256;
            
            fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);

            break;
        }
        
        case IR_ARRAY_INDEX: {
            int ArrayOffset = GetVarLocation(Code, Instruction -> Source1);
            int IndexOffset = GetVarLocation(Code, Instruction -> Source2);
            int DestinationOffset = Code -> StackOffset;

            Code -> StackOffset += 8;

            SetVarLocation(Code, Instruction -> Destination, DestinationOffset);
            
            fprintf(Output, "    mov rax, [rbp-%d]\n", ArrayOffset);
            fprintf(Output, "    mov rbx, [rbp-%d]\n", IndexOffset);
            fprintf(Output, "    imul rbx, 8\n");
            fprintf(Output, "    add rax, rbx\n");
            fprintf(Output, "    mov [rbp-%d], rax\n", DestinationOffset);

            break;
        }
        
        default:
            fprintf(Output, "    ; Unimplemented IR opcode: %s\n", IROpcodeName(Instruction -> Op));

            break;
    }
}

void GenerateFunction(CodeGen *Code, IRFunction *Function) {
    Code -> CurrentFunction = Function;
    Code -> StackOffset = 0;
    Code -> VarLocationCount = 0;
    
    GeneratePrologue(Code, Function);
    
    for (size_t i = 0; i < Function -> ParamCount; i++) {
        int Offset = Code -> StackOffset;

        Code -> StackOffset += 8;

        SetVarLocation(Code, Function -> Params[i], Offset);
        
        const char *ArgRegs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

        if (i < 6) {
            fprintf(Code -> Output, "    mov [rbp-%d], %s\n", Offset, ArgRegs[i]);
        }
    }
    
    for (size_t i = 0; i < Function -> InstructionCount; i++) {
        GenerateInstruction(Code, Function -> Instructions[i]);
    }
    
    fprintf(Code -> Output, "\n");
}

void GenerateAssembly(IRProgram *Program, const char *OutputFile) {
    FILE *Output = fopen(OutputFile, "w");
    if (!Output) {
        fprintf(stderr, "Error: Could not open output file %s\n", OutputFile);

        return;
    }
    
    CodeGen *Code = CreateCodeGen(Output);
    
    fprintf(Output, "; Rebekah | Copyright 2026 © AveriC\n");
    fprintf(Output, "section .text\n\n");

    fprintf(Output, "extern outputstr\n");
    fprintf(Output, "extern outputint\n");
    fprintf(Output, "extern outputstrint\n");
    fprintf(Output, "extern outputintstr\n");
    fprintf(Output, "extern outputstrstr\n");
    fprintf(Output, "extern outputintint\n");
    fprintf(Output, "extern input\n");
    fprintf(Output, "\n");
    
    for (size_t i = 0; i < Program -> FunctionCount; i++) {
        fprintf(Output, "global %s\n", Program -> Functions[i] -> Name);
    }

    fprintf(Output, "\n");
    fprintf(Output, "section .text\n\n");

    for (size_t i = 0; i < Program -> FunctionCount; i++) {
        GenerateFunction(Code, Program -> Functions[i]);
    }

    fprintf(Output, "\nsection .data\n");

    for (size_t i = 0; i < Code -> StringCount; i++) {
        fprintf(Output, "%s: db `%s`, 0\n", Code -> Strings[i].Label, Code -> Strings[i].String);
    }
    
    DestroyCodeGen(Code);
    fclose(Output);
    
    printf("Assembly written to %s\n", OutputFile);
}
