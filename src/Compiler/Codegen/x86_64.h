#ifndef x86_64_H
#define x86_64_H

#include "../IR.h"
#include <stdio.h>

typedef struct CodeGen {
    FILE *Output;
    IRFunction *CurrentFunction;
    
    int StackOffset;
    
    struct {
        IRValue *Value;
        int Offset;
    } *VarLocations;

    size_t VarLocationCount;
    
    int LabelCounter;
} CodeGen;

void GenerateAssembly(IRProgram *Program, const char *OutputFile);

CodeGen *CreateCodeGen(FILE *Output);
void DestroyCodeGen(CodeGen *Gen);

void GenerateFunction(CodeGen *Gen, IRFunction *Function);
void GenerateInstruction(CodeGen *Gen, IRInstruction *Inst);

int GetVarLocation(CodeGen *Gen, IRValue *Value);
void SetVarLocation(CodeGen *Gen, IRValue *Value, int Offset);
const char *GetTempRegister(int TempID);

#endif