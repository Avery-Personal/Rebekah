#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "../IR.h"

#define CSE_MAX 256
#define CF_MAX_KNOWN 1024

typedef struct {
    int TempID;
    
    int64_t Value;
} KnownConst;

typedef struct {
    IROpcode Op;
    
    IRValue *Src1;
    IRValue *Src2;
    IRValue *Result;
} CSEEntry;

void OptimizeConstantFolding(IRFunction *Function);
void OptimizeDeadCode(IRFunction *Function);
void OptimizeDeadStores(IRFunction *Function);
void OptimizeCSE(IRFunction *Function);

void OptimizeFunction(IRFunction *Function);
void OptimizeProgram(IRProgram *Program);

#endif