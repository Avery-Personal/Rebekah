#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IRGen.h"
#include "../IR.h"
#include "../../Lexer/Lexer.h"

static IRTypeKind GetExpressionIRType(ASTExpression *Expression) {
    if (!Expression) return IR_TYPE_VOID;
    
    switch (Expression -> Kind) {
        case EXPR_LITERAL:
            if (Expression -> Literal.String) {
                return IR_TYPE_PTR;
            }

            return IR_TYPE_INT;
        
        case EXPR_IDENTIFIER:
        case EXPR_BINARY:
        case EXPR_UNARY:
        case EXPR_CALL: return IR_TYPE_INT;
        case EXPR_ARRAY_LITERAL: return IR_TYPE_PTR;
        
        default: return IR_TYPE_INT;
    }
}

static const char *MangleOutputName(ASTExpression **Args, size_t ArgCount) {
    static char Buffer[128];

    strcpy(Buffer, "output");
    
    for (size_t i = 0; i < ArgCount; i++) {
        IRTypeKind Type = GetExpressionIRType(Args[i]);

        if (Type == IR_TYPE_PTR) {
            strcat(Buffer, "_str");
        } else {
            strcat(Buffer, "_int");
        }
    }
    
    return Buffer;
}

void DeclareVariablesInStatement(IRGenContext *Context, ASTStatement *Statement) {
    if (!Statement) return;
    
    switch (Statement -> Kind) {
        case STMT_VAR_DECL: {
            IRValue *Variable = IRCreateVar(Statement -> VarDecl.Name, IR_TYPE_INT);

            DeclareVariable(Context, Statement -> VarDecl.Name, Variable);

            break;
        }
        
        case STMT_BLOCK: {
            for (size_t i = 0; i < Statement -> Block.Count; i++) {
                DeclareVariablesInStatement(Context, Statement -> Block.Statements[i]);
            }

            break;
        }
        
        default:
            break;
    }
}

IRGenContext *CreateIRGenContext(void) {
    IRGenContext *Context = malloc(sizeof(IRGenContext));

    Context -> CurrentFunction = NULL;
    Context -> Program = IRCreateProgram();

    Context -> Variables = NULL;
    Context -> VariableCount = 0;
    Context -> LabelCount = 0;

    Context -> BreakLabel = NULL;
    Context -> ContinueLabel = NULL;

    return Context;
}

void DestroyIRGenContext(IRGenContext *Context) {
    free(Context -> Variables);
    free(Context);
}

IRValue *LookupVariable(IRGenContext *Context, const char *Name) {
    size_t NameLen = 0;

    while (Name[NameLen] != '\0' && Name[NameLen] != ' ' && Name[NameLen] != '\t' && Name[NameLen] != '\n' && Name[NameLen] != '\r' && Name[NameLen] != '(' && Name[NameLen] != ')' && Name[NameLen] != '[' && Name[NameLen] != ']' && Name[NameLen] != '<' && Name[NameLen] != '>' && Name[NameLen] != ',' && Name[NameLen] != ':' && Name[NameLen] != ';') {
        NameLen++;
    }
    
    for (size_t i = 0; i < Context -> VariableCount; i++) {
        const char *VarName = Context -> Variables[i].Name;

        size_t VarLen = 0;

        while (VarName[VarLen] != '\0' && VarName[VarLen] != ' ' && VarName[VarLen] != '\t' && VarName[VarLen] != '\n' && VarName[VarLen] != '\r' && VarName[VarLen] != '(' && VarName[VarLen] != ')' && VarName[VarLen] != '[' && VarName[VarLen] != ']' && VarName[VarLen] != '<' && VarName[VarLen] != '>' && VarName[VarLen] != ',' && VarName[VarLen] != ':' && VarName[VarLen] != ';') {
            VarLen++;
        }
        
        if (NameLen == VarLen && memcmp(Name, VarName, NameLen) == 0) {
            return Context -> Variables[i].Value;
        }
    }
    
    return NULL;
}

void DeclareVariable(IRGenContext *Context, const char *Name, IRValue *Value) {
    Context -> Variables = realloc(Context -> Variables, sizeof(*Context -> Variables) * (Context -> VariableCount + 1));
    Context -> Variables[Context -> VariableCount].Name = ExtractName(Name);
    Context -> Variables[Context -> VariableCount].Value = Value;
    Context -> VariableCount++;
}

const char *GenerateLabel(IRGenContext *Context, const char *Prefix) {
    char *Label = malloc(64);

    snprintf(Label, 64, "%s_%d", Prefix, Context -> LabelCount++);

    return Label;
}

IRValue *GenerateExpression(IRGenContext *Context, ASTExpression *Expression) {
    if (!Expression) return NULL;
    
    switch (Expression -> Kind) {
        case EXPR_LITERAL: {
            IRValue *Temp = IRCreateTemp(IR_TYPE_INT);

            if (Expression -> Literal.String) {
                Temp -> Kind = VALUE_CONST;
                Temp -> Type = IR_TYPE_PTR;
                Temp -> Label = Expression -> Literal.String;
            } else {
                IRInstruction *Instruction = IRCreateConstInst(Temp, Expression -> Literal.Int);

                IRAddInstruction(Context -> CurrentFunction, Instruction);
            }

            return Temp;
        }
        
        case EXPR_IDENTIFIER: {
            IRValue *Variable = LookupVariable(Context, Expression -> Identifier);
            if (!Variable) {
                fprintf(stderr, "IR Error: Undefined variable\n");
                
                return NULL;
            }
            
            IRValue *Temp = IRCreateTemp(IR_TYPE_INT);
            IRInstruction *Load = IRCreateLoad(Temp, Variable);

            IRAddInstruction(Context -> CurrentFunction, Load);

            return Temp;
        }
        
        case EXPR_BINARY: {
            IRValue *Left = GenerateExpression(Context, Expression -> Binary.Left);
            IRValue *Right = GenerateExpression(Context, Expression -> Binary.Right);
            
            IRValue *Result = IRCreateTemp(IR_TYPE_INT);
            IROpcode Op;
            
            switch (Expression -> Binary.Op) {
                case OP_ADD: Op = IR_ADD; break;
                case OP_SUB: Op = IR_SUB; break;
                case OP_MUL: Op = IR_MUL; break;
                case OP_DIV: Op = IR_DIV; break;
                case OP_EQ:  Op = IR_EQ; break;
                case OP_NE:  Op = IR_NE; break;
                case OP_LT:  Op = IR_LT; break;
                case OP_LE:  Op = IR_LE; break;
                case OP_GT:  Op = IR_GT; break;
                case OP_GE:  Op = IR_GE; break;
                case OP_AND: Op = IR_AND; break;
                case OP_OR:  Op = IR_OR; break;

                default:
                    fprintf(stderr, "IR Error: Unknown binary operator\n");

                    return NULL;
            }
            
            IRInstruction *Instruction = IRCreateBinary(Result, Left, Right, Op);

            IRAddInstruction(Context -> CurrentFunction, Instruction);

            return Result;
        }
        
        case EXPR_UNARY: {
            IRValue *Operand = GenerateExpression(Context, Expression -> Unary.Operand);
            IRValue *Result = IRCreateTemp(IR_TYPE_INT);
            
            IROpcode Op = (Expression -> Unary.Op == TOKEN_MINUS) ? IR_NEG : IR_NOT;
            IRInstruction *Instruction = IRCreateUnary(Result, Operand, Op);

            IRAddInstruction(Context -> CurrentFunction, Instruction);

            return Result;
        }
        
        case EXPR_CALL: {
            IRValue **Args = malloc(sizeof(IRValue*) * Expression -> Call.ArgCount);

            for (size_t i = 0; i < Expression -> Call.ArgCount; i++) {
                Args[i] = GenerateExpression(Context, Expression -> Call.Args[i]);
            }
            
            const char *FunctionName = ExtractName(Expression -> Call.Callee -> Identifier);

            if (strcmp(FunctionName, "output") == 0) {
                FunctionName = MangleOutputName(Expression -> Call.Args, Expression -> Call.ArgCount);
            }
            
            IRValue *Result = IRCreateTemp(IR_TYPE_INT);
            IRInstruction *Call = IRCreateCall(Result, FunctionName, Args, Expression -> Call.ArgCount);

            IRAddInstruction(Context -> CurrentFunction, Call);
            
            return Result;
        }
        
        case EXPR_ARRAY_LITERAL: {
            IRValue *Size = IRCreateConst(Expression -> Array.Count);
            IRValue *Array = IRCreateTemp(IR_TYPE_PTR);
            
            IRInstruction *Alloc = IRCreateArrayAlloc(Array, Size);

            IRAddInstruction(Context -> CurrentFunction, Alloc);
            
            for (size_t i = 0; i < Expression -> Array.Count; i++) {
                IRValue *Element = GenerateExpression(Context, Expression -> Array.Elements[i]);
                IRValue *Index = IRCreateConst(i);
                IRValue *Address = IRCreateTemp(IR_TYPE_PTR);
                
                IRInstruction *CalcAddr = IRCreateArrayIndex(Address, Array, Index);

                IRAddInstruction(Context -> CurrentFunction, CalcAddr);
                
                IRInstruction *Store = IRCreateStore(Address, Element);

                IRAddInstruction(Context -> CurrentFunction, Store);
            }
            
            return Array;
        }
        
        case EXPR_INDEX: {
            IRValue *Array = GenerateExpression(Context, Expression -> Index.Target);
            IRValue *Index = GenerateExpression(Context, Expression -> Index.Index);
            
            IRValue *Address = IRCreateTemp(IR_TYPE_PTR);
            IRInstruction *CalcAddr = IRCreateArrayIndex(Address, Array, Index);

            IRAddInstruction(Context -> CurrentFunction, CalcAddr);
            
            IRValue *Result = IRCreateTemp(IR_TYPE_INT);
            IRInstruction *Load = IRCreateLoad(Result, Address);

            IRAddInstruction(Context -> CurrentFunction, Load);
            
            return Result;
        }
        
        default:
            fprintf(stderr, "IR Error: Unhandled expression kind\n");

            return NULL;
    }
}

void GenerateStatement(IRGenContext *Context, ASTStatement *Statement) {
    if (!Statement) return;
    
    switch (Statement -> Kind) {
        case STMT_VAR_DECL: {
            IRValue *Variable = LookupVariable(Context, Statement -> VarDecl.Name);
            
            if (Statement -> VarDecl.Initializer) {
                IRValue *Init = GenerateExpression(Context, Statement -> VarDecl.Initializer);
                IRInstruction *Store = IRCreateStore(Variable, Init);

                IRAddInstruction(Context -> CurrentFunction, Store);
            }

            break;
        }
        
        case STMT_ASSIGN: {
            IRValue *Value = GenerateExpression(Context, Statement -> Assign.Value);
            
            if (Statement -> Assign.Target -> Kind == EXPR_IDENTIFIER) {
                IRValue *Variable = LookupVariable(Context, Statement -> Assign.Target -> Identifier);
                IRInstruction *Store = IRCreateStore(Variable, Value);

                IRAddInstruction(Context -> CurrentFunction, Store);
            } else if (Statement -> Assign.Target -> Kind == EXPR_INDEX) {
                IRValue *Array = GenerateExpression(Context, Statement -> Assign.Target -> Index.Target);
                IRValue *Index = GenerateExpression(Context, Statement -> Assign.Target -> Index.Index);
                
                IRValue *Address = IRCreateTemp(IR_TYPE_PTR);
                IRInstruction *CalcAddr = IRCreateArrayIndex(Address, Array, Index);

                IRAddInstruction(Context -> CurrentFunction, CalcAddr);
                
                IRInstruction *Store = IRCreateStore(Address, Value);

                IRAddInstruction(Context -> CurrentFunction, Store);
            }

            break;
        }
        
        case STMT_IF: {
            const char *ThenLabel = GenerateLabel(Context, "if_then");
            const char *EndLabel = GenerateLabel(Context, "if_end");
            
            IRValue *Cond = GenerateExpression(Context, Statement -> If.Condition);
            IRInstruction *Branch = IRCreateIfFalseJump(Cond, EndLabel);

            IRAddInstruction(Context -> CurrentFunction, Branch);
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(ThenLabel));

            for (size_t i = 0; i < Statement -> If.ThenCount; i++) {
                GenerateStatement(Context, Statement -> If.ThenBlock[i]);
            }
            
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(EndLabel));

            break;
        }
        
        case STMT_WHILE: {
            const char *CondLabel = GenerateLabel(Context, "while_cond");
            const char *BodyLabel = GenerateLabel(Context, "while_body");
            const char *EndLabel = GenerateLabel(Context, "while_end");
            
            const char *OldBreak = Context -> BreakLabel;
            const char *OldContinue = Context -> ContinueLabel;

            Context -> BreakLabel = EndLabel;
            Context -> ContinueLabel = CondLabel;
            
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(CondLabel));

            IRValue *Cond = GenerateExpression(Context, Statement -> While.Condition);
            IRInstruction *Branch = IRCreateIfFalseJump(Cond, EndLabel);

            IRAddInstruction(Context -> CurrentFunction, Branch);
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(BodyLabel));

            for (size_t i = 0; i < Statement -> While.Count; i++) {
                GenerateStatement(Context, Statement -> While.Body[i]);
            }

            IRAddInstruction(Context -> CurrentFunction, IRCreateJump(CondLabel));
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(EndLabel));
            
            Context -> BreakLabel = OldBreak;
            Context -> ContinueLabel = OldContinue;

            break;
        }
        
        case STMT_FOR: {
            const char *CondLabel = GenerateLabel(Context, "for_cond");
            const char *BodyLabel = GenerateLabel(Context, "for_body");
            const char *IncrLabel = GenerateLabel(Context, "for_incr");
            const char *EndLabel = GenerateLabel(Context, "for_end");
            
            const char *OldBreak = Context -> BreakLabel;
            const char *OldContinue = Context -> ContinueLabel;

            Context -> BreakLabel = EndLabel;
            Context -> ContinueLabel = IncrLabel;
            
            IRValue *Iterator = IRCreateVar(Statement -> For.Iterator, IR_TYPE_INT);

            DeclareVariable(Context, Statement -> For.Iterator, Iterator);

            IRValue *Start = GenerateExpression(Context, Statement -> For.Start);

            IRAddInstruction(Context -> CurrentFunction, IRCreateStore(Iterator, Start));
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(CondLabel));

            IRValue *End = GenerateExpression(Context, Statement -> For.End);
            IRValue *IterVal = IRCreateTemp(IR_TYPE_INT);

            IRAddInstruction(Context -> CurrentFunction, IRCreateLoad(IterVal, Iterator));
            
            IRValue *Cond = IRCreateTemp(IR_TYPE_INT);

            IRAddInstruction(Context -> CurrentFunction, IRCreateBinary(Cond, IterVal, End, IR_LE));
            IRAddInstruction(Context -> CurrentFunction, IRCreateIfFalseJump(Cond, EndLabel));
            
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(BodyLabel));

            for (size_t i = 0; i < Statement -> For.Count; i++) {
                GenerateStatement(Context, Statement -> For.Body[i]);
            }
            
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(IncrLabel));

            IRValue *IterVal2 = IRCreateTemp(IR_TYPE_INT);

            IRAddInstruction(Context -> CurrentFunction, IRCreateLoad(IterVal2, Iterator));

            IRValue *One = IRCreateConst(1);
            IRValue *Incr = IRCreateTemp(IR_TYPE_INT);
            IRInstruction *Inc = IRCreateConstInst(One, 1);

            IRAddInstruction(Context -> CurrentFunction, Inc);
            IRAddInstruction(Context -> CurrentFunction, IRCreateBinary(Incr, IterVal2, One, IR_ADD));
            IRAddInstruction(Context -> CurrentFunction, IRCreateStore(Iterator, Incr));
            IRAddInstruction(Context -> CurrentFunction, IRCreateJump(CondLabel));
            
            IRAddInstruction(Context -> CurrentFunction, IRCreateLabel(EndLabel));
            
            Context -> BreakLabel = OldBreak;
            Context -> ContinueLabel = OldContinue;

            break;
        }
        
        case STMT_RETURN: {
            IRValue *ReturnValue = NULL;

            if (Statement -> Return.Value) {
                ReturnValue = GenerateExpression(Context, Statement -> Return.Value);
            }

            IRInstruction *Return = IRCreateReturn(ReturnValue);

            IRAddInstruction(Context -> CurrentFunction, Return);

            break;
        }
        
        case STMT_CALL: {
            GenerateExpression(Context, Statement -> ExpressionStmt.Expression);

            break;
        }
        
        case STMT_BLOCK: {
            for (size_t i = 0; i < Statement -> Block.SubprogramCount; i++) {
                IRGenerateFunction(Context, Statement -> Block.Subprograms[i]);
            }
            
            for (size_t i = 0; i < Statement -> Block.Count; i++) {
                GenerateStatement(Context, Statement -> Block.Statements[i]);
            }

            break;
        }
        
        default:
            fprintf(stderr, "IR Warning: Unhandled statement kind\n");

            break;
    }
}

void IRGenerateFunction(IRGenContext *Context, ASTSubprogram *Subprogram) {
    const char *FunctionName = ExtractName(Subprogram -> Name);
    
    IRFunction *Function = IRCreateFunction(FunctionName, IR_TYPE_INT);
    IRFunction *OldFunction = Context -> CurrentFunction;

    size_t OldVarCount = Context -> VariableCount;
    
    Context -> CurrentFunction = Function;
    
    for (size_t i = 0; i < Subprogram -> ParamCount; i++) {
        IRValue *Parameter = IRCreateVar(Subprogram -> Params[i].Name, IR_TYPE_INT);

        Parameter -> Kind = VALUE_PARAM;

        DeclareVariable(Context, Subprogram -> Params[i].Name, Parameter);
    }
    
    for (size_t i = 0; i < Subprogram -> BodyCount; i++) {
        DeclareVariablesInStatement(Context, Subprogram -> Body[i]);
    }

    for (size_t i = 0; i < Subprogram -> NestedSubprogramCount; i++) {
        IRGenerateFunction(Context, Subprogram -> NestedSubprograms[i]);
    }
    
    for (size_t i = 0; i < Subprogram -> BodyCount; i++) {
        GenerateStatement(Context, Subprogram -> Body[i]);
    }
    
    IRAddFunction(Context -> Program, Function);
    
    Context -> CurrentFunction = OldFunction;
    Context -> VariableCount = OldVarCount;
}

IRProgram *GenerateIR(ASTProgram *Program) {
    IRGenContext *Context = CreateIRGenContext();
    
    for (size_t i = 0; i < Program -> SubprogramCount; i++) {
        IRGenerateFunction(Context, Program -> Subprograms[i]);
    }
    
    if (Program -> StatementCount > 0) {
        IRFunction *MainFunc = IRCreateFunction("_start", IR_TYPE_VOID);

        Context -> CurrentFunction = MainFunc;
        
        for (size_t i = 0; i < Program -> StatementCount; i++) {
            GenerateStatement(Context, Program -> Statements[i]);
        }
        
        IRAddFunction(Context -> Program, MainFunc);
    }
    
    IRProgram *Result = Context -> Program;

    DestroyIRGenContext(Context);
    
    return Result;
}
