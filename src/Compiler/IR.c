#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IR.h"

static int TempCounter = 0;
static int LabelCounter = 0;

char *ExtractName(const char *Source) {
    if (!Source) return NULL;
    
    size_t Len = 0;

    while (Source[Len] != '\0' && 
           Source[Len] != ' ' && 
           Source[Len] != '\t' && 
           Source[Len] != '\n' && 
           Source[Len] != '\r' &&
           Source[Len] != '(' &&
           Source[Len] != ')' &&
           Source[Len] != '[' &&
           Source[Len] != ']' &&
           Source[Len] != '<' &&
           Source[Len] != '>' &&
           Source[Len] != ',' &&
           Source[Len] != ':' &&
           Source[Len] != ';') {
        Len++;
    }
    
    char *Name = malloc(Len + 1);

    memcpy(Name, Source, Len);

    Name[Len] = '\0';

    return Name;
}

IRValue *IRCreateTemp(IRTypeKind Type) {
    IRValue *Value = malloc(sizeof(IRValue));

    Value -> Kind = VALUE_TEMP;
    Value -> Type = Type;
    Value -> TempID = TempCounter++;

    return Value;
}

IRValue *IRCreateVar(const char *Name, IRTypeKind Type) {
    IRValue *Value = malloc(sizeof(IRValue));

    Value -> Kind = VALUE_VAR;
    Value -> Type = Type;
    Value -> Name = ExtractName(Name);
    Value -> TempID = -1;

    return Value;
}

IRValue *IRCreateConst(int64_t ConstValue) {
    IRValue *Value = malloc(sizeof(IRValue));

    Value -> Kind = VALUE_CONST;
    Value -> Type = IR_TYPE_INT;
    Value -> IntValue = ConstValue;
    Value -> TempID = -1;

    return Value;
}

IRInstruction *IRCreateConstInst(IRValue *Destination, int64_t Value) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = IR_CONST;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = NULL;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;
    
    Destination -> IntValue = Value;
    
    return Instruction;
}

IRInstruction *IRCreateMove(IRValue *Destination, IRValue *Source) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = IR_MOVE;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = Source;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRInstruction *IRCreateBinary(IRValue *Destination, IRValue *LHS, IRValue *RHS, IROpcode Op) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = Op;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = LHS;
    Instruction -> Source2 = RHS;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRInstruction *IRCreateUnary(IRValue *Destination, IRValue *Source, IROpcode Op) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = Op;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = Source;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRInstruction *IRCreateLoad(IRValue *Destination, IRValue *Address) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = IR_LOAD;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = Address;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRInstruction *IRCreateStore(IRValue *Address, IRValue *Value) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = IR_STORE;
    Instruction -> Destination = Address;
    Instruction -> Source1 = Value;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRInstruction *IRCreateAddr(IRValue *Destination, IRValue *Variable) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = IR_ADDR;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = Variable;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRInstruction *IRCreateArrayAlloc(IRValue *Destination, IRValue *Size) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = IR_ARRAY_ALLOC;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = Size;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRInstruction *IRCreateArrayIndex(IRValue *Destination, IRValue *Array, IRValue *Index) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = IR_ARRAY_INDEX;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = Array;
    Instruction -> Source2 = Index;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRInstruction *IRCreateLabel(const char *Label) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));
    IRValue *LabelValue = malloc(sizeof(IRValue));
    
    LabelValue -> Kind = VALUE_LABEL;
    LabelValue -> Label = Label;
    
    Instruction -> Op = IR_LABEL;
    Instruction -> Destination = NULL;
    Instruction -> Source1 = NULL;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = LabelValue;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;
    
    return Instruction;
}

IRInstruction *IRCreateJump(const char *Label) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));
    IRValue *LabelValue = malloc(sizeof(IRValue));
    
    LabelValue -> Kind = VALUE_LABEL;
    LabelValue -> Label = Label;
    
    Instruction -> Op = IR_JMP;
    Instruction -> Destination = NULL;
    Instruction -> Source1 = NULL;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = LabelValue;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;
    
    return Instruction;
}

IRInstruction *IRCreateIfJump(IRValue *Condition, const char *Label) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));
    IRValue *LabelValue = malloc(sizeof(IRValue));
    
    LabelValue -> Kind = VALUE_LABEL;
    LabelValue -> Label = Label;
    
    Instruction -> Op = IR_JMP_IF;
    Instruction -> Destination = NULL;
    Instruction -> Source1 = Condition;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = LabelValue;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;
    
    return Instruction;
}

IRInstruction *IRCreateIfFalseJump(IRValue *Condition, const char *Label) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));
    IRValue *LabelValue = malloc(sizeof(IRValue));
    
    LabelValue -> Kind = VALUE_LABEL;
    LabelValue -> Label = Label;
    
    Instruction -> Op = IR_JMP_IF_FALSE;
    Instruction -> Destination = NULL;
    Instruction -> Source1 = Condition;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = LabelValue;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;
    
    return Instruction;
}

IRInstruction *IRCreateCall(IRValue *Destination, const char *FunctionName, IRValue **Args, size_t ArgCount) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));
    IRValue *FuncValue = malloc(sizeof(IRValue));
    
    FuncValue -> Kind = VALUE_LABEL;
    FuncValue -> Label = FunctionName;
    
    Instruction -> Op = IR_CALL;
    Instruction -> Destination = Destination;
    Instruction -> Source1 = FuncValue;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = Args;
    Instruction -> ArgCount = ArgCount;
    
    return Instruction;
}

IRInstruction *IRCreateReturn(IRValue *Value) {
    IRInstruction *Instruction = malloc(sizeof(IRInstruction));

    Instruction -> Op = IR_RETURN;
    Instruction -> Destination = Value;
    Instruction -> Source1 = NULL;
    Instruction -> Source2 = NULL;
    Instruction -> Extra = NULL;
    Instruction -> Args = NULL;
    Instruction -> ArgCount = 0;

    return Instruction;
}

IRProgram *IRCreateProgram(void) {
    IRProgram *Program = malloc(sizeof(IRProgram));

    Program -> Functions = NULL;
    Program -> FunctionCount = 0;

    return Program;
}

IRFunction *IRCreateFunction(const char *Name, IRTypeKind ReturnType) {
    IRFunction *Function = malloc(sizeof(IRFunction));

    Function -> Name = Name;
    Function -> ReturnType = ReturnType;
    Function -> Params = NULL;
    Function -> ParamCount = 0;
    Function -> Instructions = NULL;
    Function -> InstructionCount = 0;
    Function -> Locals = NULL;
    Function -> LocalCount = 0;

    return Function;
}

void IRAddInstruction(IRFunction *Function, IRInstruction *Instruction) {
    Function -> Instructions = realloc(Function -> Instructions, sizeof(IRInstruction*) * (Function -> InstructionCount + 1));
    Function -> Instructions[Function -> InstructionCount++] = Instruction;
}

void IRAddFunction(IRProgram *Program, IRFunction *Function) {
    Program -> Functions = realloc(Program -> Functions, sizeof(IRFunction*) * (Program -> FunctionCount + 1));
    Program -> Functions[Program -> FunctionCount++] = Function;
}

const char *IROpcodeName(IROpcode Op) {
    switch (Op) {
        case IR_NOP: return "NOP";
        case IR_CONST: return "CONST";
        case IR_MOVE: return "MOVE";
        case IR_ADD: return "ADD";
        case IR_SUB: return "SUB";
        case IR_MUL: return "MUL";
        case IR_DIV: return "DIV";
        case IR_MOD: return "MOD";
        case IR_NEG: return "NEG";
        case IR_EQ: return "EQ";
        case IR_NE: return "NE";
        case IR_LT: return "LT";
        case IR_LE: return "LE";
        case IR_GT: return "GT";
        case IR_GE: return "GE";
        case IR_AND: return "AND";
        case IR_OR: return "OR";
        case IR_NOT: return "NOT";
        case IR_LOAD: return "LOAD";
        case IR_STORE: return "STORE";
        case IR_ADDR: return "ADDR";
        case IR_ARRAY_ALLOC: return "ARRAY_ALLOC";
        case IR_ARRAY_INDEX: return "ARRAY_INDEX";
        case IR_CALL: return "CALL";
        case IR_PARAM: return "PARAM";
        case IR_RETURN: return "RETURN";
        case IR_LABEL: return "LABEL";
        case IR_JMP: return "JMP";
        case IR_JMP_IF: return "JMP_IF";
        case IR_JMP_IF_FALSE: return "JMP_IF_FALSE";

        default: return "UNKNOWN";
    }
}

static void IRPrintValue(IRValue *Value) {
    if (!Value) {
        printf("null");

        return;
    }
    
    switch (Value -> Kind) {
        case VALUE_TEMP:
            printf("t%d", Value -> TempID);

            break;
        case VALUE_VAR:
            printf("%%%s", Value -> Name);

            break;
        case VALUE_PARAM:
            printf("@%s", Value -> Name);

            break;
        case VALUE_CONST:
            printf("%lld", Value -> IntValue);

            break;
        case VALUE_LABEL:
            printf("%s", Value -> Label);

            break;
    }
}

void IRPrint(IRProgram *Program) {
    for (size_t i = 0; i < Program -> FunctionCount; i++) {
        IRFunction *Function = Program -> Functions[i];
        
        printf("\nFunction %s:\n", Function -> Name);
        
        for (size_t j = 0; j < Function -> InstructionCount; j++) {
            IRInstruction *Instruction = Function -> Instructions[j];
            
            switch (Instruction -> Op) {
                case IR_CONST:
                    printf("  ");
                    IRPrintValue(Instruction -> Destination);
                    printf(" = %lld\n", Instruction -> Destination -> IntValue);

                    break;
                    
                case IR_MOVE:
                    printf("  ");
                    IRPrintValue(Instruction -> Destination);
                    printf(" = ");
                    IRPrintValue(Instruction -> Source1);
                    printf("\n");

                    break;
                    
                case IR_ADD:
                case IR_SUB:
                case IR_MUL:
                case IR_DIV:
                case IR_MOD:
                case IR_EQ:
                case IR_NE:
                case IR_LT:
                case IR_LE:
                case IR_GT:
                case IR_GE:
                case IR_AND:
                case IR_OR:
                    printf("  ");
                    IRPrintValue(Instruction -> Destination);
                    printf(" = ");
                    IRPrintValue(Instruction -> Source1);
                    printf(" %s ", IROpcodeName(Instruction -> Op));
                    IRPrintValue(Instruction -> Source2);
                    printf("\n");

                    break;
                    
                case IR_NEG:
                case IR_NOT:
                    printf("  ");
                    IRPrintValue(Instruction -> Destination);
                    printf(" = %s ", IROpcodeName(Instruction -> Op));
                    IRPrintValue(Instruction -> Source1);
                    printf("\n");

                    break;
                    
                case IR_LOAD:
                    printf("  ");
                    IRPrintValue(Instruction -> Destination);
                    printf(" = LOAD ");
                    IRPrintValue(Instruction -> Source1);
                    printf("\n");

                    break;

                    
                case IR_STORE:
                    printf("  STORE ");
                    IRPrintValue(Instruction -> Destination);
                    printf(", ");
                    IRPrintValue(Instruction -> Source1);
                    printf("\n");

                    break;
                    
                case IR_CALL:
                    if (Instruction -> Destination) {
                        printf("  ");
                        IRPrintValue(Instruction -> Destination);
                        printf(" = ");
                    } else {
                        printf("  ");
                    }

                    printf("CALL %s(", Instruction -> Source1 -> Label);

                    for (size_t k = 0; k < Instruction -> ArgCount; k++) {
                        IRPrintValue(Instruction -> Args[k]);

                        if (k + 1 < Instruction -> ArgCount)
                            printf(", ");
                    }

                    printf(")\n");

                    break;
                    
                case IR_RETURN:
                    printf("  RETURN ");

                    if (Instruction -> Destination)
                        IRPrintValue(Instruction -> Destination);

                    printf("\n");

                    break;
                    
                case IR_LABEL:
                    printf("%s:\n", Instruction -> Extra -> Label);

                    break;
                    
                case IR_JMP:
                    printf("  JMP %s\n", Instruction -> Extra -> Label);

                    break;
                    
                case IR_JMP_IF:
                    printf("  JMP_IF ");
                    IRPrintValue(Instruction -> Source1);
                    printf(" %s\n", Instruction -> Extra -> Label);

                    break;
                    
                case IR_JMP_IF_FALSE:
                    printf("  JMP_IF_FALSE ");
                    IRPrintValue(Instruction -> Source1);
                    printf(" %s\n", Instruction -> Extra -> Label);

                    break;
                    
                default:
                    printf("  [%s]\n", IROpcodeName(Instruction -> Op));

                    break;
            }
        }
    }
}
