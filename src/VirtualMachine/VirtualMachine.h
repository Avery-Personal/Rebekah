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

    BC_OP_GLOAD,
    BC_OP_GSTORE,

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
    
    BC_OP_AND,
    BC_OP_OR,
    BC_OP_NOT,

    BC_OP_JMP,
    BC_OP_JMP_IF,
    BC_OP_JMP_IFN,

    BC_OP_CALL,
    BC_OP_RET,

    BC_OP_ARRAY_ALLOC,
    BC_OP_ARRAY_INDEX,
    BC_OP_PLOAD,
    BC_OP_PSTORE,

    BC_OP_HALT
} Opcodes;

typedef enum {
    FUNCTION_BYTECODE,
    FUNCTION_NATIVE
} FunctionType;

typedef uint64_t (*NativeFunction)(uint64_t *Arguments, uint8_t *ArgumentTypes, size_t ArgumentCount, uint8_t ReturnType);

typedef struct {
    IRValue *Variable;
    uint16_t Index;
} VariableEntry;

typedef struct {
    IRValue *Value;

    uint16_t Register;
} RegisterEntry;

typedef struct {
    const char *Label;

    size_t InstructionIndex;
} LabelEntry;

typedef struct {
    const char *Name;

    NativeFunction Function;
    FunctionType Type;

    uint16_t ArgumentCount;
} NativeFunctionEntry;

typedef struct {
    const char *FunctionName;

    uint32_t InstructionIndex;

    uint8_t *ArgumentTypes;
    uint16_t *ArgumentRegisters;
    uint16_t ArgumentCount;

    uint8_t ReturnType;
} CallArgumentInfo;

typedef struct {
    uint8_t Opcode;

    uint16_t A, B, C;
} BytecodeInstruction;

typedef struct {
    const char *Name;

    BytecodeInstruction *Code;
    uint32_t InstructionCount;

    uint64_t *Constants;
    uint32_t ConstantCount;

    CallArgumentInfo *CallArguments;
    uint32_t CallArgumentCount;

    uint16_t RegisterCount;
    uint16_t ArgumentCount;

    uint16_t GlobalCount;
} BytecodeFunction;

typedef struct {
    BytecodeFunction **Functions;
    uint32_t FunctionCount;

    NativeFunctionEntry *Natives;
    uint32_t NativeCount;

    uint32_t GlobalCount;
    uint32_t EntryFunction;
} BytecodeProgram;

typedef struct VirtualFrame {
    uint64_t *Registers;
    uint16_t ReturnRegister;

    struct VirtualFrame *Caller;
    BytecodeFunction *Function;
    
    size_t InstructionPointer;

    uint64_t *Globals;
} VirtualFrame;

typedef struct {
    VirtualFrame *CurrentFrame;

    BytecodeProgram *Program;

    uint64_t *Globals;

    int Halted;
} VirtualMachine;

const char *OpcodeName(uint8_t Op);

static uint16_t AssignRegister(IRValue *Value, RegisterEntry **MapPointer, size_t *MapCapacity, size_t *MapCount, uint16_t *NextRegister);

BytecodeFunction *LowerFunction(IRFunction *_IRFunction, IRValue **GlobalVariables, size_t GlobalVariableCount);
BytecodeProgram *LowerProgram(IRProgram *Program);

VirtualMachine *VirtualMachineCreate(BytecodeProgram *Program);
static VirtualFrame *VirtualMachineCreateFrame(VirtualMachine *_VirtualMachine, BytecodeFunction *Function, VirtualFrame *Caller);
static void VirtualMachineDestroyFrame(VirtualMachine *_VirtualMachine);
void VirtualMachineExecute(VirtualMachine *_VirtualMachine);

void PrintInstruction(BytecodeInstruction *Instruction, uint32_t Index);
void PrintFunction(BytecodeFunction *Function, uint32_t Index);
void PrintProgram(BytecodeProgram *Program);

void DumpVirtualMachineState(VirtualMachine *_VirtualMachine);

#endif
