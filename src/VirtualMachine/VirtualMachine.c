#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "VirtualMachine.h"
#include "../Compiler/IR.h"

const char *OpcodeName(uint8_t Op) {
    switch (Op) {
        case BC_OP_NOP: return "NOP";
        case BC_OP_CONST: return "CONST";
        case BC_OP_MOV: return "MOV";
        case BC_OP_LOAD: return "LOAD";
        case BC_OP_STORE: return "STORE";
        case BC_OP_LEA: return "LEA";

        case BC_OP_ADD: return "ADD";
        case BC_OP_SUB: return "SUB";
        case BC_OP_MUL: return "MUL";
        case BC_OP_DIV: return "DIV";
        case BC_OP_MOD: return "MOD";
        case BC_OP_NEG: return "NEG";

        case BC_OP_CMP_EQ: return "CMP_EQ";
        case BC_OP_CMP_NE: return "CMP_NE";
        case BC_OP_CMP_LT: return "CMP_LT";
        case BC_OP_CMP_GT: return "CMP_GT";
        case BC_OP_CMP_LE: return "CMP_LE";
        case BC_OP_CMP_GE: return "CMP_GE";

        case BC_OP_JMP: return "JMP";
        case BC_OP_JMP_IF: return "JMP_IF";
        case BC_OP_JMP_IFN: return "JMP_IFN";

        case BC_OP_CALL: return "CALL";
        case BC_OP_RET: return "RET";

        case BC_OP_HALT: return "HALT";

        default: return "UNKNOWN";
    }
}

static uint16_t AssignRegister(IRValue *Value, RegisterEntry *Map, size_t *MapCount, uint16_t *NextRegister) {
    for (size_t i = 0; i < *MapCount; i++) {
        if (Map[i].Value == Value)
            return Map[i].Register;
    }

    uint16_t Register = (*NextRegister)++;

    Map[*MapCount].Value = Value;
    Map[*MapCount].Register = Register;

    (*MapCount)++;

    return Register;
}

BytecodeFunction *LowerFunction(IRFunction *_IRFunction) {
    BytecodeFunction *Function = calloc(1, sizeof(BytecodeFunction));

    Function -> InstructionCount = _IRFunction -> InstructionCount;
    Function -> Code = calloc(Function -> InstructionCount, sizeof(BytecodeInstruction));

    LabelEntry *Labels = calloc(_IRFunction -> InstructionCount, sizeof(LabelEntry));
    size_t LabelCount = 0;

    RegisterEntry RegisterMap[512];
    size_t RegisterMapCount = 0;
    uint16_t NextRegister = 0;

    size_t ByteIndex = 0;

    for (size_t i = 0; i < _IRFunction -> InstructionCount; i++) {
        IRInstruction *Instruction = _IRFunction -> Instructions[i];

        if (Instruction -> Op == IR_LABEL) {
            Labels[LabelCount].Label = Instruction -> Destination -> Label;
            Labels[LabelCount].InstructionIndex = ByteIndex;

            LabelCount++;
        } else {
            ByteIndex++;
        }
    }

    ByteIndex = 0;

    for (size_t i = 0; i < _IRFunction -> InstructionCount; i++) {
        IRInstruction *Instruction = _IRFunction -> Instructions[i];

        if (Instruction -> Op == IR_LABEL)
            continue;

        BytecodeInstruction *Output = &Function -> Code[ByteIndex];

        switch (Instruction -> Op) {
            case IR_CONST: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CONST;
                Output -> A = Destination;

                if (!Instruction -> Source1) {
                    printf("IR_CONST has NULL Source1\n");

                    exit(1);
                }

                Function -> Constants = realloc(Function -> Constants, sizeof(uint64_t) * (Function -> ConstantCount + 1));
                Function -> Constants[Function -> ConstantCount] = Instruction -> Source1 -> IntValue;

                Output -> B = Function -> ConstantCount;

                Function -> ConstantCount++;
                
                break;
            }

            case IR_ADD: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_ADD;
                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_RETURN: {
                uint16_t Source = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_RET;
                Output -> A = Source;

                break;
            }

            case IR_JMP: {
                Output -> Opcode = BC_OP_JMP;

                const char *Target = Instruction -> Destination -> Label;

                for (size_t l = 0; l < LabelCount; l++) {
                    if (strcmp(Labels[l].Label, Target) == 0) {
                        Output -> A = Labels[l].InstructionIndex;

                        break;
                    }
                }

                break;
        }

        default:
            break;
        }

        ByteIndex++;
    }

    Function -> RegisterCount = NextRegister;

    free(Labels);

    return Function;
}

BytecodeProgram *LowerProgram(IRProgram *Program) {
    BytecodeProgram *Bytecode = calloc(1, sizeof(BytecodeProgram));

    Bytecode -> FunctionCount = Program -> FunctionCount;
    Bytecode -> Functions = calloc(Bytecode -> FunctionCount, sizeof(BytecodeFunction*));

    for (size_t i = 0; i < Program -> FunctionCount; i++) {
        Bytecode -> Functions[i] = LowerFunction(Program -> Functions[i]);
    }

    Bytecode -> EntryFunction = 0;
    Bytecode -> GlobalCount = 0;

    return Bytecode;
}

VirtualMachine *VirtualMachineCreate(BytecodeProgram *Program) {
    VirtualMachine *_VirtualMachine = malloc(sizeof(VirtualMachine));

    _VirtualMachine -> Program = Program;
    _VirtualMachine -> Halted = 0;

    _VirtualMachine -> Globals = calloc(Program -> GlobalCount, sizeof(uint64_t));

    BytecodeFunction *Entry = Program -> Functions[Program -> EntryFunction];
    VirtualFrame *Frame = calloc(1, sizeof(VirtualFrame));

    Frame -> Function = Entry;
    Frame -> Registers = calloc(Entry -> RegisterCount, sizeof(uint64_t));
    Frame -> InstructionPointer = 0;

    _VirtualMachine -> CurrentFrame = Frame;

    return _VirtualMachine;
}

void PrintInstruction(BytecodeInstruction *Instruction, uint32_t Index) {
    printf("%04u | %-8s  A=%u  B=%u  C=%u\n", Index, OpcodeName(Instruction -> Opcode), Instruction -> A, Instruction -> B, Instruction -> C);
}

void PrintFunction(BytecodeFunction *Function, uint32_t Index) {
    printf("Function %u\n", Index);
    printf("Registers: %u\n", Function -> RegisterCount);
    printf("Arguments: %u\n", Function -> ArgumentCount);
    printf("Instructions: %u\n", Function -> InstructionCount);
    printf("Constants: %u\n", Function -> ConstantCount);

    for (uint32_t i = 0; i < Function -> ConstantCount; i++) {
        printf("  CONST[%u] = %llu\n", i, (uint64_t) Function -> Constants[i]);
    }

    for (uint32_t i = 0; i < Function -> InstructionCount; i++) {
        PrintInstruction(&Function -> Code[i], i);
    }
}

void PrintProgram(BytecodeProgram *Program) {
    printf("Function Count: %u\n", Program -> FunctionCount);
    printf("Global Count: %u\n", Program -> GlobalCount);
    printf("Entry Function: %u\n", Program -> EntryFunction);

    for (uint32_t i = 0; i < Program -> FunctionCount; i++) {
        PrintFunction(Program -> Functions[i], i);
    }
}

void DumpVirtualMachineState(VirtualMachine *_VirtualMachine) {
    VirtualFrame *Frame = _VirtualMachine -> CurrentFrame;

    if (!Frame) {
        fprintf(stderr, "No active Frame.\n");

        return;
    }

    printf("IP: %u\n", Frame -> InstructionPointer);
    printf("Registers:\n");

    for (uint16_t i = 0; i < Frame -> Function -> RegisterCount; i++) {
        printf("  R%-3u = %llu\n", i, (uint64_t) Frame -> Registers[i]);
    }
}
