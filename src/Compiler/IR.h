#ifndef IR_H
#define IR_H

#include <stdint.h>

typedef enum {
    IR_NOP,
    IR_CONST,
    IR_MOVE,

    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_NEG,
    
    IR_EQ,
    IR_NE,
    IR_LT,
    IR_LE,
    IR_GT,
    IR_GE,
    
    IR_AND,
    IR_OR,
    IR_NOT,
    
    IR_LOAD,
    IR_STORE,
    IR_ADDR,
    
    IR_ARRAY_ALLOC,
    IR_ARRAY_INDEX,
    
    IR_CALL,
    IR_PARAM,
    IR_RETURN,
    
    IR_LABEL,
    IR_JMP,
    IR_JMP_IF,
    IR_JMP_IF_FALSE,
} IROpcode;

typedef enum {
    IR_TYPE_INT,
    IR_TYPE_FLOAT,
    IR_TYPE_BOOL,
    IR_TYPE_CHAR,
    IR_TYPE_VOID,
    IR_TYPE_PTR
} IRTypeKind;

typedef enum {
    VALUE_TEMP,
    VALUE_VAR,
    VALUE_PARAM,
    VALUE_CONST,
    VALUE_LABEL,
} IRValueKind;

typedef struct {
    IRValueKind Kind;
    IRTypeKind Type;
    
    union {
        int64_t IntVal;
        double FloatVal;
        
        char CharVal;
        int BoolVal;
        
        const char *Label;
        const char *Name;
    };
    
    int TempID;
} IRValue;

typedef struct {
    IROpcode Op;

    IRValue *Destination;
    IRValue *Source1;
    IRValue *Source2;
    
    IRValue *Extra;

    IRValue **Args;
    size_t ArgCount;
} IRInstruction;

typedef struct {
    const char *Name;

    IRTypeKind ReturnType;

    IRValue **Params;
    size_t ParamCount;

    IRInstruction **Instructions;
    size_t InstructionCount;

    IRValue **Locals;
    size_t LocalCount;
} IRFunction;

typedef struct {
    IRFunction **Functions;
    size_t FunctionCount;
} IRProgram;

char *ExtractName(const char *Source);

IRValue *IRCreateTemp(IRTypeKind Type);
IRValue *IRCreateVar(const char *Name, IRTypeKind Type);
IRValue *IRCreateConst(int64_t Value);

IRInstruction *IRCreateConstInst(IRValue *Destination, int64_t Value);
IRInstruction *IRCreateMove(IRValue *Destination, IRValue *Source);
IRInstruction *IRCreateBinary(IRValue *Destination, IRValue *LHS, IRValue *RHS, IROpcode Op);
IRInstruction *IRCreateUnary(IRValue *Destination, IRValue *Source, IROpcode Op);

IRInstruction *IRCreateLoad(IRValue *Destination, IRValue *Address);
IRInstruction *IRCreateStore(IRValue *Address, IRValue *Value);
IRInstruction *IRCreateAddr(IRValue *Destination, IRValue *Variable);

IRInstruction *IRCreateArrayAlloc(IRValue *Destination, IRValue *Size);
IRInstruction *IRCreateArrayIndex(IRValue *Destination, IRValue *Array, IRValue *Index);

IRInstruction *IRCreateLabel(const char *Label);
IRInstruction *IRCreateJump(const char *Label);
IRInstruction *IRCreateIfJump(IRValue *Condition, const char *Label);
IRInstruction *IRCreateIfFalseJump(IRValue *Condition, const char *Label);

IRInstruction *IRCreateCall(IRValue *Destination, const char *FunctionName, IRValue **Args, size_t ArgCount);
IRInstruction *IRCreateReturn(IRValue *Value);

IRProgram *IRCreateProgram(void);
IRFunction *IRCreateFunction(const char *Name, IRTypeKind ReturnType);
void IRAddInstruction(IRFunction *Function, IRInstruction *Instruction);
void IRAddFunction(IRProgram *Program, IRFunction *Function);

void IRPrint(IRProgram *Program);
const char *IROpcodeName(IROpcode Op);

#endif
