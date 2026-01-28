#ifndef IRGEN_H
#define IRGEN_H

#include "../Parser/AST.h"
#include "IR.h"

typedef struct IRGenContext {
    IRFunction *CurrentFunction;
    IRProgram *Program;
    
    struct {
        const char *Name;

        IRValue *Value;
    } *Variables;

    size_t VariableCount;
    int LabelCount;
    
    const char *BreakLabel;
    const char *ContinueLabel;
} IRGenContext;

IRProgram *GenerateIR(ASTProgram *Program);

IRGenContext *CreateIRGenContext(void);
void DestroyIRGenContext(IRGenContext *Context);

void GenerateFunction(IRGenContext *Context, ASTSubprogram *Subprogram);
void GenerateStatement(IRGenContext *Context, ASTStatement *Statement);
IRValue *GenerateExpression(IRGenContext *Context, ASTExpression *Expression);

IRValue *LookupVariable(IRGenContext *Context, const char *Name);
void DeclareVariable(IRGenContext *Context, const char *Name, IRValue *Value);
const char *GenerateLabel(IRGenContext *Context, const char *Prefix);

#endif