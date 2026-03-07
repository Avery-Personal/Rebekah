#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IROptimization.h"

static void CompactInstructions(IRFunction *Function, int *Removed) {
    size_t NewCount = 0;

    for (size_t i = 0; i < Function -> InstructionCount; i++) {
        if (!Removed[i])
            Function -> Instructions[NewCount++] = Function -> Instructions[i];
    }

    Function -> InstructionCount = NewCount;
}

static int ValuesEqual(IRValue *A, IRValue *B) {
    if (A == B) return 1;
    if (!A || !B) return 0;
    if (A -> Kind != B -> Kind) return 0;

    switch (A -> Kind) {
        case VALUE_TEMP:
            return A -> TempID == B -> TempID;

        case VALUE_CONST:
            return A -> IntValue == B -> IntValue;

        case VALUE_VAR:
        case VALUE_PARAM:
            if (!A -> Name || !B -> Name) return A -> Name == B -> Name;

            return strcmp(A -> Name, B -> Name) == 0;

        case VALUE_LABEL:
            if (!A -> Label || !B -> Label) return A -> Label == B -> Label;

            return strcmp(A -> Label, B -> Label) == 0;

        default:
            return 0;
    }
}

void OptimizeConstantFolding(IRFunction *Function) {
    if (!Function || Function -> InstructionCount == 0) return;

    KnownConst *Known = calloc(CF_MAX_KNOWN, sizeof(KnownConst));

    size_t KnownCount = 0;
    int Changed = 1;

    while (Changed) {
        Changed = 0;
        KnownCount = 0;

        for (size_t i = 0; i < Function -> InstructionCount; i++) {
            IRInstruction *Instruction = Function -> Instructions[i];

            if (Instruction -> Op == IR_CONST &&
                Instruction -> Destination && Instruction -> Destination -> Kind == VALUE_TEMP && Instruction -> Source1) {

                if (KnownCount < CF_MAX_KNOWN) {
                    Known[KnownCount].TempID = Instruction -> Destination -> TempID;
                    Known[KnownCount].Value  = Instruction -> Source1 -> IntValue;

                    KnownCount++;
                }

                continue;
            }

            if (Instruction -> Op >= IR_ADD && Instruction -> Op <= IR_OR && Instruction -> Destination && Instruction -> Source1 && Instruction -> Source2) {

                int64_t V1 = 0;
                int64_t V2 = 0;

                int F1 = 0;
                int F2 = 0;

                if (Instruction -> Source1 -> Kind == VALUE_CONST) {
                    V1 = Instruction -> Source1 -> IntValue; F1 = 1;
                } else if (Instruction -> Source1 -> Kind == VALUE_TEMP) {
                    for (size_t k = 0; k < KnownCount; k++) {
                        if (Known[k].TempID == Instruction -> Source1 -> TempID) {
                            V1 = Known[k].Value; F1 = 1; break;
                        }
                    }
                }

                if (Instruction -> Source2 -> Kind == VALUE_CONST) {
                    V2 = Instruction -> Source2 -> IntValue; F2 = 1;
                } else if (Instruction -> Source2 -> Kind == VALUE_TEMP) {
                    for (size_t k = 0; k < KnownCount; k++) {
                        if (Known[k].TempID == Instruction -> Source2 -> TempID) {
                            V2 = Known[k].Value; F2 = 1; break;
                        }
                    }
                }

                if (F1 && F2) {
                    int64_t Result  = 0;
                    int CanFold = 1;

                    switch (Instruction -> Op) {
                        case IR_ADD: Result = V1 + V2; break;
                        case IR_SUB: Result = V1 - V2; break;
                        case IR_MUL: Result = V1 * V2; break;
                        case IR_DIV: if (V2) Result = V1 / V2; else CanFold = 0; break;
                        case IR_MOD: if (V2) Result = V1 % V2; else CanFold = 0; break;
                        case IR_EQ: Result = (V1 == V2); break;
                        case IR_NE: Result = (V1 != V2); break;
                        case IR_LT: Result = (V1 <  V2); break;
                        case IR_LE: Result = (V1 <= V2); break;
                        case IR_GT: Result = (V1 >  V2); break;
                        case IR_GE: Result = (V1 >= V2); break;
                        case IR_AND: Result = (V1 && V2); break;
                        case IR_OR: Result = (V1 || V2); break;
                        default: CanFold = 0; break;
                    }

                    if (CanFold) {
                        IRValue *NewConst = IRCreateConst(Result);

                        Instruction -> Op = IR_CONST;
                        Instruction -> Source1 = NewConst;
                        Instruction -> Source2 = NULL;
                        Instruction -> Destination -> IntValue = Result;

                        if (KnownCount < CF_MAX_KNOWN) {
                            Known[KnownCount].TempID = Instruction -> Destination -> TempID;
                            Known[KnownCount].Value  = Result;

                            KnownCount++;
                        }

                        Changed = 1;
                    }
                }
            }

            if ((Instruction -> Op == IR_NEG || Instruction -> Op == IR_NOT) && Instruction -> Destination && Instruction -> Source1) {
                int64_t V1 = 0;
                int F1 = 0;

                if (Instruction -> Source1 -> Kind == VALUE_CONST) {
                    V1 = Instruction -> Source1 -> IntValue; F1 = 1;
                } else if (Instruction -> Source1 -> Kind == VALUE_TEMP) {
                    for (size_t k = 0; k < KnownCount; k++) {
                        if (Known[k].TempID == Instruction -> Source1 -> TempID) {
                            V1 = Known[k].Value; F1 = 1; break;
                        }
                    }
                }

                if (F1) {
                    int64_t Result = (Instruction -> Op == IR_NEG) ? -V1 : !V1;
                    IRValue *NewConst = IRCreateConst(Result);

                    Instruction -> Op = IR_CONST;
                    Instruction -> Source1 = NewConst;
                    Instruction -> Destination -> IntValue = Result;

                    if (KnownCount < CF_MAX_KNOWN) {
                        Known[KnownCount].TempID = Instruction -> Destination -> TempID;
                        Known[KnownCount].Value  = Result;

                        KnownCount++;
                    }

                    Changed = 1;
                }
            }
        }
    }

    free(Known);
}

void OptimizeDeadCode(IRFunction *Function) {
    if (!Function || Function -> InstructionCount == 0) return;

    int *Removed = calloc(Function -> InstructionCount, sizeof(int));
    int Dead = 0;

    for (size_t i = 0; i < Function -> InstructionCount; i++) {
        IRInstruction *Instruction = Function -> Instructions[i];

        if (Dead) {
            if (Instruction -> Op == IR_LABEL) {
                Dead = 0;
            } else {
                Removed[i] = 1;
            }
        }

        if (Instruction -> Op == IR_JMP || Instruction -> Op == IR_RETURN)
            Dead = 1;
    }

    CompactInstructions(Function, Removed);
    free(Removed);
}

void OptimizeDeadStores(IRFunction *Function) {
    if (!Function || Function -> InstructionCount == 0) return;

    int *Removed = calloc(Function -> InstructionCount, sizeof(int));

    for (size_t i = 0; i < Function -> InstructionCount; i++) {
        if (Removed[i]) continue;

        IRInstruction *Instruction = Function -> Instructions[i];

        if (Instruction -> Op != IR_STORE) continue;

        IRValue *Target = Instruction -> Destination;
        if (!Target || Target -> Kind != VALUE_VAR) continue;

        int HasLoad          = 0;
        int HasSubsequentStore = 0;

        for (size_t j = i + 1; j < Function -> InstructionCount; j++) {
            if (Removed[j]) continue;

            IRInstruction *Next = Function -> Instructions[j];

            if (Next -> Op == IR_LABEL || Next -> Op == IR_JMP || Next -> Op == IR_JMP_IF || Next -> Op == IR_JMP_IF_FALSE || Next -> Op == IR_CALL) {
                HasLoad = 1;

                break;
            }

            if (Next -> Op == IR_LOAD && Next -> Source1 == Target) {
                HasLoad = 1;

                break;
            }

            if (Next -> Op == IR_STORE && Next -> Destination == Target) {
                HasSubsequentStore = 1;

                break;
            }
        }

        if (HasSubsequentStore && !HasLoad) {
            Removed[i] = 1;
        }
    }

    CompactInstructions(Function, Removed);

    free(Removed);
}

void OptimizeCSE(IRFunction *Function) {
    if (!Function || Function -> InstructionCount == 0) return;

    CSEEntry *Table = calloc(CSE_MAX, sizeof(CSEEntry));
    size_t TableCount = 0;

    for (size_t i = 0; i < Function -> InstructionCount; i++) {
        IRInstruction *Instruction = Function -> Instructions[i];

        if (Instruction -> Op == IR_LABEL || Instruction -> Op == IR_STORE || Instruction -> Op == IR_CALL || Instruction -> Op == IR_JMP || Instruction -> Op == IR_JMP_IF || Instruction -> Op == IR_JMP_IF_FALSE || Instruction -> Op == IR_RETURN) {
            TableCount = 0;

            continue;
        }

        if (Instruction -> Op < IR_ADD || Instruction -> Op > IR_OR) continue;
        if (!Instruction -> Destination || Instruction -> Destination -> Kind != VALUE_TEMP) continue;
        if (!Instruction -> Source1 || !Instruction -> Source2) continue;

        int Found = 0;

        for (size_t k = 0; k < TableCount; k++) {
            if (Table[k].Op == Instruction -> Op &&
                ValuesEqual(Table[k].Src1, Instruction -> Source1) && ValuesEqual(Table[k].Src2, Instruction -> Source2)) {

                Instruction -> Op = IR_MOVE;
                Instruction -> Source1 = Table[k].Result;
                Instruction -> Source2 = NULL;

                Found = 1;

                break;
            }
        }

        if (!Found && TableCount < CSE_MAX) {
            Table[TableCount].Op = Instruction -> Op;
            Table[TableCount].Src1 = Instruction -> Source1;
            Table[TableCount].Src2 = Instruction -> Source2;
            Table[TableCount].Result = Instruction -> Destination;

            TableCount++;
        }
    }

    free(Table);
}

void OptimizeFunction(IRFunction *Function) {
    if (!Function) return;

    OptimizeConstantFolding(Function);
    OptimizeCSE(Function);
    OptimizeDeadStores(Function);
    OptimizeDeadCode(Function);
}

void OptimizeProgram(IRProgram *Program) {
    if (!Program) return;

    for (size_t i = 0; i < Program -> FunctionCount; i++) {
        OptimizeFunction(Program -> Functions[i]);
    }
}
