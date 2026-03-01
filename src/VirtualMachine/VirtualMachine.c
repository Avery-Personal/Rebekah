#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "VirtualMachine.h"
#include "../Compiler/IR.h"

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

VirtualMachine *LowerProgram(IRProgram *Program) {
    VirtualMachine *_VirtualMachine = calloc(1, sizeof(VirtualMachine));

    _VirtualMachine -> Program -> FunctionCount = Program -> FunctionCount;
    _VirtualMachine -> Program -> Functions = calloc(_VirtualMachine -> Program -> FunctionCount, sizeof(BytecodeFunction *));

    for (size_t i = 0; i < Program -> FunctionCount; i++) {
        _VirtualMachine -> Program -> Functions[i] = LowerFunction(Program -> Functions[i]);
    }

    return _VirtualMachine;
}
