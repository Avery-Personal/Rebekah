#ifndef IR_H
#define IR_H

#include <stdint.h>

typedef enum {
    IR_NOP,
    IR_CONST,
    IR_MOVE,
    IR_UNARY,
    IR_BINARY,
    IR_LOAD,
    IR_STORE,
    IR_CALL,
    IR_RETURN,
    IR_LABEL,
    IR_JMP,
    IR_JMP_IF,
    IR_PARAM,
} IROpcode;

typedef enum {
    IR_TYPE_INT,
    IR_TYPE_FLOAT,
    IR_TYPE_BOOL,
    IR_TYPE_CHAR,
    IR_TYPE_VOID,
    IR_TYPE_PTR
} IRTypeKind;

typedef struct IRValue {
    IRTypeKind Type;
    
    union {
        int64_t IntVal;
        double FloatVal;

        char CharVal;
        int BoolVal;

        const char *Label;
        const char *Function;
        
        struct IRValue *Temp;
    };
    
    int TempID;
} IRValue;

typedef struct IRInstruction {
    IROpcode Op;

    IRValue *Destination;
    IRValue *Source1;
    IRValue *Source2;
    
    IRValue *Extra;
} IRInstruction;

typedef struct IRFunction {
    const char *Name;

    IRTypeKind ReturnType;

    IRValue **Params;
    size_t ParamCount;

    IRInstruction **Instructions;
    size_t InstructionCount;
} IRFunction;

typedef struct IRProgram {
    IRFunction **Functions;
    size_t FunctionCount;
} IRProgram;

#endif
