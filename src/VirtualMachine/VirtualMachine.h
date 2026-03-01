#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <stdint.h>

#include "../Compiler/IR.h"

typedef enum {
    BC_OP_NOP,

    BC_OP_CONST,
    BC_OP_MOV,
    BC_OP_LOAD,
    BC_OP_STORE,
    BC_OP_LEA,

    BC_OP_ADD,
    BC_OP_SUB,
    BC_OP_MUL,
    BC_OP_DIV,
    BC_OP_MOD,
    BC_OP_NEG,

    BC_OP_CMP_EQ,
    BC_OP_CMP_NE,
    BC_OP_CMP_LT,
    BC_OP_CMP_GT,
    BC_OP_CMP_LE,
    BC_OP_CMP_GE,

    BC_OP_JMP,
    BC_OP_JMP_IF,
    BC_OP_JMP_IFN,

    BC_OP_CALL,
    BC_OP_RET,

    BC_OP_HALT
} Opcodes;

typedef struct {
    IRValue *Value;

    uint16_t Register;
} RegisterEntry;

typedef struct {
    const char *Label;

    size_t InstructionIndex;
} LabelEntry;

typedef struct {
    uint8_t Opcode;

    uint8_t A, B, C;
} BytecodeInstruction;

typedef struct {
    BytecodeInstruction *Code;
    uint32_t InstructionCount;

    uint64_t *Constants;
    uint32_t ConstantCount;

    uint16_t RegisterCount;
    uint16_t ArgumentCount;
} BytecodeFunction;

typedef struct {
    BytecodeFunction **Functions;
    uint32_t FunctionCount;

    uint32_t GlobalCount;
    uint32_t EntryFunction;
} BytecodeProgram;

typedef struct VirtualFrame {
    uint64_t *Registers;

    struct VirtualFrame *Caller;
    BytecodeFunction *Function;
    
    size_t InstructionPointer;
} VirtualFrame;

typedef struct {
    VirtualFrame *CurrentFrame;

    BytecodeProgram *Program;

    uint64_t *Globals;

    int Halted;
} VirtualMachine;

static uint16_t AssignRegister(IRValue *Value, RegisterEntry *Map, size_t *MapCount, uint16_t *NextRegister);

BytecodeFunction *LowerFunction(IRFunction *_IRFunction);
VirtualMachine *LowerProgram(IRProgram *Program);

#endif
