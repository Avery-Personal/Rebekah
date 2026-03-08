#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "VirtualMachine.h"
#include "../Compiler/IR.h"

const char *OpcodeName(uint8_t Op) {
    switch (Op) {
        case BC_OP_NOP: return "NOP";
        case BC_OP_CONST: return "CONST";
        case BC_OP_MOV: return "MOV";
        case BC_OP_LOAD: return "LOAD";
        case BC_OP_STORE: return "STORE";
        case BC_OP_LEA: return "LEA";

        case BC_OP_GLOAD: return "GLOAD";
        case BC_OP_GSTORE: return "GSTORE";

        case BC_OP_ADD: return "ADD";
        case BC_OP_SUB: return "SUB";
        case BC_OP_MUL: return "MUL";
        case BC_OP_DIV: return "DIV";
        case BC_OP_MOD: return "MOD";
        case BC_OP_NEG: return "NEG";

        case BC_OP_CMP_EQ: return "CMP_EQ";
        case BC_OP_CMP_NE: return "CMP_NE";
        case BC_OP_CMP_LT: return "CMP_LT";
        case BC_OP_CMP_GT: return "CMP_GT";
        case BC_OP_CMP_LE: return "CMP_LE";
        case BC_OP_CMP_GE: return "CMP_GE";

        case BC_OP_AND: return "AND";
        case BC_OP_OR: return "OR";
        case BC_OP_NOT: return "NOT";

        case BC_OP_JMP: return "JMP";
        case BC_OP_JMP_IF: return "JMP_IF";
        case BC_OP_JMP_IFN: return "JMP_IFN";

        case BC_OP_CALL: return "CALL";
        case BC_OP_RET: return "RET";

        case BC_OP_PRINT: return "PRINT";

        case BC_OP_HALT: return "HALT";

        default: return "UNKNOWN";
    }
}

static uint64_t NativeOutput(uint64_t *Arguments, uint8_t *ArgumentTypes, size_t ArgumentCount) {
    for (size_t i = 0; i < ArgumentCount; i++) {
        if (i > 0)
            printf(" ");

        if (ArgumentTypes[i] == IR_TYPE_PTR) {
            printf("%s", (const char *)(uintptr_t) Arguments[i]);
        } else {
            printf("%lld", (int64_t) Arguments[i]);
        }
    }

    printf("\n");

    return 0;
}

static uint64_t NativeInput(uint64_t *Arguments, uint8_t *ArgumentTypes, size_t ArgumentCount) {
    char Buffer[512];
    char *End;

    if (ArgumentCount > 0) {
        if (ArgumentTypes[0] == IR_TYPE_PTR) {
            printf("%s", (const char *)(uintptr_t) Arguments[0]);

            fflush(stdout);
        }
    }

    if (!fgets(Buffer, sizeof(Buffer), stdin))
        return 0;

    size_t Len = strlen(Buffer);
    if (Len > 0 && Buffer[Len - 1] == '\n')
        Buffer[Len - 1] = '\0';

    int64_t Value = strtoll(Buffer, &End, 10);

    if (*End == '\0') {
        return (uint64_t) Value;
    }

    char *String = strdup(Buffer);

    return (uint64_t) String;
}

static void RegisterNative(BytecodeProgram *Program, const char *Name, NativeFunction Function, uint16_t ArgumentCount) {
    Program -> Natives = realloc(Program -> Natives, sizeof(NativeFunctionEntry) * (Program -> NativeCount + 1));

    NativeFunctionEntry *Entry = &Program -> Natives[Program -> NativeCount++];

    Entry -> Name = Name;
    Entry -> Function = Function;
    Entry -> Type = FUNCTION_NATIVE;
    Entry -> ArgumentCount = ArgumentCount;
}

static uint16_t AssignRegister(IRValue *Value, RegisterEntry *Map, size_t *MapCount, uint16_t *NextRegister) {
    for (size_t i = 0; i < *MapCount; i++) {
        if (Map[i].Value == Value)
            return Map[i].Register;
    }

    uint16_t Register = (*NextRegister)++;

    Map[*MapCount].Value = Value;
    Map[*MapCount].Register = Register;

    (*MapCount)++;

    return Register;
}

int IsGlobal(IRValue *Value, IRValue **GlobalVariables, size_t GlobalVariableCount) {
    for (size_t g = 0; g < GlobalVariableCount; g++)
        if (GlobalVariables[g] == Value) return 1;

    return 0;
}

BytecodeFunction *LowerFunction(IRFunction *_IRFunction, IRValue **GlobalVariables, size_t GlobalVariableCount) {
    BytecodeFunction *Function = calloc(1, sizeof(BytecodeFunction));

    Function -> Name = _IRFunction -> Name;
    Function -> InstructionCount = _IRFunction -> InstructionCount;
    Function -> Code = calloc(Function -> InstructionCount, sizeof(BytecodeInstruction));

    LabelEntry *Labels = calloc(_IRFunction -> InstructionCount, sizeof(LabelEntry));
    size_t LabelCount = 0;

    RegisterEntry RegisterMap[512];
    size_t RegisterMapCount = 0;
    uint16_t NextRegister = 0;

    typedef struct {
        IRValue *Variable;
        uint16_t Index;
    } VariableEntry;

    VariableEntry VariableMap[256];
    size_t VariableCount = 0;
    uint16_t NextGlobal = 0;

    size_t ByteIndex = 0;

    for (size_t i = 0; i < _IRFunction -> InstructionCount; i++) {
        IRInstruction *Instruction = _IRFunction -> Instructions[i];

        if (Instruction -> Op == IR_LABEL) {
            Labels[LabelCount].Label = Instruction -> Extra -> Label;
            Labels[LabelCount].InstructionIndex = ByteIndex;

            LabelCount++;
        } else {
            ByteIndex++;
        }
    }

    ByteIndex = 0;

    for (size_t i = 0; i < _IRFunction -> InstructionCount; i++) {
        IRInstruction *Instruction = _IRFunction -> Instructions[i];

        if (Instruction -> Op == IR_LABEL)
            continue;

        BytecodeInstruction *Output = &Function -> Code[ByteIndex];

        switch (Instruction -> Op) {
            case IR_STORE: {
                IRValue *Variable = Instruction -> Destination;

                uint16_t Source = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);

                int GlobalIndex = -1;

                for (size_t i = 0; i < GlobalVariableCount; i++) {
                    if (GlobalVariables[i] == Variable) {
                        GlobalIndex = (int) i;

                        break;
                    }
                }

                if (GlobalIndex >= 0) {
                    Output -> Opcode = BC_OP_GSTORE;
                    Output -> A = (uint16_t) GlobalIndex;
                    Output -> B = Source;
                } else {
                    uint16_t LocalIndex = 0;

                    int Found = 0;

                    for (size_t v = 0; v < VariableCount; v++) {
                        if (VariableMap[v].Variable == Variable) {
                            LocalIndex = VariableMap[v].Index; Found = 1; break;
                        }
                    }

                    if (!Found) {
                        LocalIndex = NextGlobal++;

                        VariableMap[VariableCount].Variable = Variable;
                        VariableMap[VariableCount].Index = LocalIndex;

                        VariableCount++;
                    }

                    Output -> Opcode = BC_OP_STORE;
                    Output -> A = LocalIndex;
                    Output -> B = Source;
                }
                
                break;
            }

            case IR_LOAD: {
                IRValue *Variable = Instruction -> Source1;

                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);

                int GlobalIndex = -1;

                for (size_t i = 0; i < GlobalVariableCount; i++) {
                    if (GlobalVariables[i] == Variable) {
                        GlobalIndex = (int) i;

                        break;
                    }
                }

                if (GlobalIndex >= 0) {
                    Output -> Opcode = BC_OP_GLOAD;
                    Output -> A = Destination;
                    Output -> B = (uint16_t) GlobalIndex;
                } else {
                    uint16_t LocalIndex = 0;

                    int Found = 0;

                    for (size_t v = 0; v < VariableCount; v++) {
                        if (VariableMap[v].Variable == Variable) {
                            LocalIndex = VariableMap[v].Index;
                            Found = 1;
                            
                            break;
                        }
                    }

                    if (!Found) {
                        LocalIndex = NextGlobal++;

                        VariableMap[VariableCount].Variable = Variable;
                        VariableMap[VariableCount].Index = LocalIndex;

                        VariableCount++;
                    }

                    Output -> Opcode = BC_OP_LOAD;
                    Output -> A = Destination;
                    Output -> B = LocalIndex;
                }

                break;
            }

            case IR_CONST: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CONST;
                Output -> A = Destination;

                if (!Instruction -> Source1) {
                    printf("IR_CONST has NULL Source1\n");

                    exit(1);
                }

                Function -> Constants = realloc(Function -> Constants, sizeof(uint64_t) * (Function -> ConstantCount + 1));
                Function -> Constants[Function -> ConstantCount] = Instruction -> Source1 -> IntValue;

                Output -> B = Function -> ConstantCount;

                Function -> ConstantCount++;
                
                break;
            }

            case IR_MOVE: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_MOV;

                Output -> A = Destination;
                Output -> B = Source;

                break;
            }
            
            case IR_ADD: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_ADD;
                
                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_SUB: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_SUB;
                
                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_MUL: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_MUL;
                
                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_DIV: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_DIV;
                
                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_MOD: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_MOD;
                
                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_NEG: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_NEG;
                
                Output -> A = Destination;
                Output -> B = Source;

                break;
            }

            case IR_EQ: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CMP_EQ;

                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_NE: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CMP_NE;

                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_LT: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CMP_LT;

                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_LE: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CMP_LE;

                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_GT: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CMP_GT;

                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_GE: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CMP_GE;

                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }
            
            case IR_AND: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_AND;

                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_OR: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source1 = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source2 = AssignRegister(Instruction -> Source2, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_OR;

                Output -> A = Destination;
                Output -> B = Source1;
                Output -> C = Source2;

                break;
            }

            case IR_NOT: {
                uint16_t Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);
                uint16_t Source  = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_NOT;

                Output -> A = Destination;
                Output -> B = Source;

                break;
            }

            case IR_JMP: {
                Output -> Opcode = BC_OP_JMP;

                const char *Target = Instruction -> Extra -> Label;

                for (size_t l = 0; l < LabelCount; l++) {
                    if (strcmp(Labels[l].Label, Target) == 0) {
                        Output -> A = (uint16_t) Labels[l].InstructionIndex;

                        break;
                    }
                }

                break;
            }

            case IR_JMP_IF: {
                Output -> Opcode = BC_OP_JMP_IF;

                uint16_t Condition = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);

                const char *Target = Instruction -> Extra -> Label;

                for (size_t l = 0; l < LabelCount; l++) {
                    if (strcmp(Labels[l].Label, Target) == 0) {
                        Output -> A = (uint16_t) Labels[l].InstructionIndex;

                        break;
                    }
                }

                Output -> B = Condition;

                break;
            }

            case IR_JMP_IF_FALSE: {
                Output -> Opcode = BC_OP_JMP_IFN;

                uint16_t Condition = AssignRegister(Instruction -> Source1, RegisterMap, &RegisterMapCount, &NextRegister);

                const char *Target = Instruction -> Extra -> Label;

                for (size_t l = 0; l < LabelCount; l++) {
                    if (strcmp(Labels[l].Label, Target) == 0) {
                        Output -> A = (uint16_t) Labels[l].InstructionIndex;
                        
                        break;
                    }
                }

                Output -> B = Condition;

                break;
            }

            case IR_CALL: {
                const char *FunctionName = Instruction -> Source1 -> Label;

                CallArgumentInfo Info;

                Info.FunctionName = FunctionName;
                Info.InstructionIndex = ByteIndex;
                Info.ArgumentCount = (uint16_t) Instruction -> ArgCount;
                Info.ArgumentTypes = malloc(Instruction -> ArgCount);

                uint16_t FirstArgumentRegister = 0;

                for (size_t a = 0; a < Instruction -> ArgCount; a++) {
                    uint16_t Register = AssignRegister(Instruction -> Args[a], RegisterMap, &RegisterMapCount, &NextRegister);
                    if (a == 0)
                        FirstArgumentRegister = Register;

                    Info.ArgumentTypes[a] = (uint8_t)Instruction -> Args[a] -> Type;
                }

                Function -> CallArguments = realloc(Function -> CallArguments, sizeof(CallArgumentInfo) * (Function -> CallArgumentCount + 1));
                Function -> CallArguments[Function -> CallArgumentCount++] = Info;

                uint16_t Destination = 0;
                if (Instruction -> Destination)
                    Destination = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);

                Output -> Opcode = BC_OP_CALL;
                Output -> A = Destination;
                Output -> B = FirstArgumentRegister;
                Output -> C = (uint8_t) Instruction -> ArgCount;

                break;
            }

            case IR_RETURN: {
                Output -> Opcode = BC_OP_RET;

                if (Instruction -> Destination) {
                    uint16_t Source = AssignRegister(Instruction -> Destination, RegisterMap, &RegisterMapCount, &NextRegister);

                    Output -> A = Source;
                } else {
                    Output -> A = 0;
                }

                break;
            }

            default: break;
        }

        ByteIndex++;
    }

    Function -> GlobalCount = NextGlobal;
    Function -> RegisterCount = NextRegister;

    Function -> InstructionCount = ByteIndex;

    free(Labels);

    return Function;
}

BytecodeProgram *LowerProgram(IRProgram *Program) {
    BytecodeProgram *Bytecode = calloc(1, sizeof(BytecodeProgram));

    Bytecode -> FunctionCount = Program -> FunctionCount;
    Bytecode -> Functions = calloc(Bytecode -> FunctionCount, sizeof(BytecodeFunction *));

    for (size_t i = 0; i < Program -> FunctionCount; i++) {
        Bytecode -> Functions[i] = LowerFunction(Program -> Functions[i], Program -> GlobalVariables, Program -> GlobalVariableCount);
    }

    RegisterNative(Bytecode, "output", NativeOutput, 0);
    RegisterNative(Bytecode, "input", NativeInput, 1);

    Bytecode -> EntryFunction = 0;
    Bytecode -> GlobalCount = 0;

    for (size_t i = 0; i < Bytecode -> FunctionCount; i++) {
        if (Bytecode -> Functions[i]-> Name && strcmp(Bytecode -> Functions[i] -> Name, "_start") == 0) {
            Bytecode -> EntryFunction = i;

            break;
        }
    }

    Bytecode -> GlobalCount = (uint32_t) Program -> GlobalVariableCount;

    return Bytecode;
}

VirtualMachine *VirtualMachineCreate(BytecodeProgram *Program) {
    VirtualMachine *_VirtualMachine = malloc(sizeof(VirtualMachine));

    _VirtualMachine -> Program = Program;
    _VirtualMachine -> Halted = 0;

    _VirtualMachine -> Globals = calloc(Program -> GlobalCount > 0 ? Program -> GlobalCount : 1, sizeof(uint64_t));

    BytecodeFunction *Entry = Program -> Functions[Program -> EntryFunction];
    VirtualFrame *Frame = calloc(1, sizeof(VirtualFrame));

    Frame -> Function = Entry;
    Frame -> Registers = calloc(Entry -> RegisterCount, sizeof(uint64_t));
    Frame -> InstructionPointer = 0;

    _VirtualMachine -> CurrentFrame = Frame;

    return _VirtualMachine;
}

static VirtualFrame *VirtualMachineCreateFrame(VirtualMachine *_VirtualMachine, BytecodeFunction *Function, VirtualFrame *Caller) {
    VirtualFrame *Frame = (VirtualFrame *) malloc(sizeof(VirtualFrame));

    Frame -> Registers = calloc(Function -> RegisterCount, sizeof(uint64_t));
    Frame -> Globals = calloc(Function -> GlobalCount, sizeof(uint64_t));
    Frame -> Caller = Caller;
    Frame -> Function = Function;
    Frame -> InstructionPointer = 0;

    return Frame;
}

static void VirtualMachineDestroyFrame(VirtualMachine *_VirtualMachine) {
    VirtualFrame *OldFrame = _VirtualMachine -> CurrentFrame;

    if (!OldFrame)
        return;

    _VirtualMachine -> CurrentFrame = OldFrame -> Caller;

    free(OldFrame -> Registers);
    free(OldFrame);
}

void VirtualMachineExecute(VirtualMachine *_VirtualMachine) {
    if (!_VirtualMachine -> Program) return;

    BytecodeProgram *Program = _VirtualMachine -> Program;

    if (Program -> EntryFunction >= Program -> FunctionCount)
        return;

    _VirtualMachine -> CurrentFrame = VirtualMachineCreateFrame(_VirtualMachine, Program -> Functions[Program -> EntryFunction], NULL);

    while (!_VirtualMachine -> Halted && _VirtualMachine -> CurrentFrame) {
        VirtualFrame *Frame = _VirtualMachine -> CurrentFrame;
        BytecodeFunction *Function = Frame -> Function;

        if (Frame -> InstructionPointer >= Function -> InstructionCount) {
            _VirtualMachine -> Halted = 1;

            break;
        }

        BytecodeInstruction Instruction = Function -> Code[Frame -> InstructionPointer++];

        uint64_t *Register = Frame -> Registers;
        uint64_t *Global = Frame -> Globals;

        switch (Instruction.Opcode) {
            case BC_OP_NOP:
                break;

            case BC_OP_CONST:
                Register[Instruction.A] = Function -> Constants[Instruction.B];

                break;

            case BC_OP_MOV:
                Register[Instruction.A] = Register[Instruction.B];

                break;

            case BC_OP_LOAD:
                Register[Instruction.A] = Global[Instruction.B];

                break;

            case BC_OP_STORE:
                Global[Instruction.A] = Register[Instruction.B];

                break;
            
            case BC_OP_GLOAD:
                Register[Instruction.A] = _VirtualMachine -> Globals[Instruction.B];

                break;

            case BC_OP_GSTORE:
                _VirtualMachine -> Globals[Instruction.A] = Register[Instruction.B];

                break;

            case BC_OP_ADD:
                Register[Instruction.A] = Register[Instruction.B] + Register[Instruction.C];

                break;

            case BC_OP_SUB:
                Register[Instruction.A] = Register[Instruction.B] - Register[Instruction.C];

                break;

            case BC_OP_MUL:
                Register[Instruction.A] = Register[Instruction.B] * Register[Instruction.C];

                break;

            case BC_OP_DIV:
                if (Register[Instruction.C] != 0)
                    Register[Instruction.A] = Register[Instruction.B] / Register[Instruction.C];

                break;

            case BC_OP_MOD:
                if (Register[Instruction.C] != 0)
                    Register[Instruction.A] = Register[Instruction.B] % Register[Instruction.C];

                break;

            case BC_OP_NEG:
                Register[Instruction.A] = -(int64_t) Register[Instruction.B];

                break;

            case BC_OP_CMP_EQ:
                Register[Instruction.A] = (Register[Instruction.B] == Register[Instruction.C]);

                break;

            case BC_OP_CMP_NE:
                Register[Instruction.A] = (Register[Instruction.B] != Register[Instruction.C]);

                break;

            case BC_OP_CMP_LT:
                Register[Instruction.A] = (Register[Instruction.B] < Register[Instruction.C]);

                break;

            case BC_OP_CMP_GT:
                Register[Instruction.A] = (Register[Instruction.B] > Register[Instruction.C]);

                break;

            case BC_OP_CMP_LE:
                Register[Instruction.A] = (Register[Instruction.B] <= Register[Instruction.C]);

                break;

            case BC_OP_CMP_GE:
                Register[Instruction.A] = (Register[Instruction.B] >= Register[Instruction.C]);

                break;

            case BC_OP_AND:
                Register[Instruction.A] = (Register[Instruction.B] != 0 && Register[Instruction.C] != 0) ? 1 : 0;

                break;

            case BC_OP_OR:
                Register[Instruction.A] = (Register[Instruction.B] != 0 || Register[Instruction.C] != 0) ? 1 : 0;

                break;

            case BC_OP_NOT:
                Register[Instruction.A] = (Register[Instruction.B] == 0) ? 1 : 0;

                break;

            case BC_OP_JMP:
                Frame -> InstructionPointer = Instruction.A;

                break;

            case BC_OP_JMP_IF:
                if (Register[Instruction.B])
                    Frame -> InstructionPointer = Instruction.A;

                break;

            case BC_OP_JMP_IFN:
                if (!Register[Instruction.B])
                    Frame -> InstructionPointer = Instruction.A;

                break;

            case BC_OP_CALL: {
                CallArgumentInfo *ArgumentInfo = NULL;

                for (uint32_t k = 0; k < Frame -> Function -> CallArgumentCount; k++) {
                    if (Frame -> Function -> CallArguments[k].InstructionIndex == Frame -> InstructionPointer - 1) {
                        ArgumentInfo = &Frame -> Function -> CallArguments[k];

                        break;
                    }
                }

                if (!ArgumentInfo) {
                    fprintf(stderr, "[Virtual Machine Error] no call argument info found\n");

                    _VirtualMachine -> Halted = 1;

                    break;
                }

                const char *FunctionName = ArgumentInfo -> FunctionName;
                int Dispatched = 0;

                for (uint32_t n = 0; n < Program -> NativeCount; n++) {
                    NativeFunctionEntry *Entry = &Program -> Natives[n];

                    if (strcmp(Entry -> Name, FunctionName) == 0) {
                        uint64_t *Arguments = calloc(ArgumentInfo -> ArgumentCount, sizeof(uint64_t));

                        for (size_t a = 0; a < ArgumentInfo -> ArgumentCount; a++)
                            Arguments[a] = Frame -> Registers[Instruction.B + a];

                        uint64_t Result = Entry -> Function(Arguments, ArgumentInfo -> ArgumentTypes, ArgumentInfo -> ArgumentCount);

                        free(Arguments);

                        Frame -> Registers[Instruction.A] = Result;
                        
                        Dispatched = 1;

                        break;
                    }
                }

                if (!Dispatched) {
                    BytecodeFunction *Callee = NULL;

                    for (uint32_t f = 0; f < Program -> FunctionCount; f++) {
                        if (Program -> Functions[f] -> Name && strcmp(Program -> Functions[f] -> Name, FunctionName) == 0) {
                            Callee = Program -> Functions[f];

                            break;
                        }
                    }

                    if (!Callee) {
                        fprintf(stderr, "[Virtual Machine Error] undefined function '%s'\n", FunctionName);

                        _VirtualMachine -> Halted = 1;

                        break;
                    }

                    VirtualFrame *NewFrame = VirtualMachineCreateFrame(_VirtualMachine, Callee, Frame);

                    for (uint16_t i = 0; i < Instruction.C; i++)
                        NewFrame -> Registers[i] = Frame -> Registers[Instruction.B + i];

                    NewFrame -> ReturnRegister = Instruction.A;

                    _VirtualMachine -> CurrentFrame = NewFrame;
                }

                break;
            }

            case BC_OP_RET: {
                uint64_t ReturnValue = Register[Instruction.A];
                uint16_t DestinationRegister = Frame -> ReturnRegister;

                VirtualMachineDestroyFrame(_VirtualMachine);

                if (_VirtualMachine -> CurrentFrame) {
                    _VirtualMachine -> CurrentFrame -> Registers[DestinationRegister] = ReturnValue;
                } else {
                    _VirtualMachine -> Halted = 1;
                }

                break;
            }

            case BC_OP_PRINT: {
                printf("\nHello from Rebekah!\n");
            }

            case BC_OP_HALT:
                _VirtualMachine -> Halted = 1;

                break;

            default:
                printf("Unknown opcode: %d\n", Instruction.Opcode);

                _VirtualMachine -> Halted = 1;

                break;
        }
    }
}

void PrintInstruction(BytecodeInstruction *Instruction, uint32_t Index) {
    printf("%04u | %-8s  A=%u  B=%u  C=%u\n", Index, OpcodeName(Instruction -> Opcode), Instruction -> A, Instruction -> B, Instruction -> C);
}

void PrintFunction(BytecodeFunction *Function, uint32_t Index) {
    printf("Function %u\n", Index);
    printf("Registers: %u\n", Function -> RegisterCount);
    printf("Arguments: %u\n", Function -> ArgumentCount);
    printf("Instructions: %u\n", Function -> InstructionCount);
    printf("Constants: %u\n", Function -> ConstantCount);

    for (uint32_t i = 0; i < Function -> ConstantCount; i++) {
        printf("  CONST[%u] = %llu\n", i, (uint64_t) Function -> Constants[i]);
    }

    for (uint32_t i = 0; i < Function -> InstructionCount; i++) {
        PrintInstruction(&Function -> Code[i], i);
    }
}

void PrintProgram(BytecodeProgram *Program) {
    printf("Function Count: %u\n", Program -> FunctionCount);
    printf("Global Count: %u\n", Program -> GlobalCount);
    printf("Entry Function: %u\n", Program -> EntryFunction);

    for (uint32_t i = 0; i < Program -> FunctionCount; i++) {
        PrintFunction(Program -> Functions[i], i);
    }
}

void DumpVirtualMachineState(VirtualMachine *_VirtualMachine) {
    VirtualFrame *Frame = _VirtualMachine -> CurrentFrame;

    if (!Frame) {
        fprintf(stderr, "No active Frame.\n");

        return;
    }

    printf("IP: %u\n", Frame -> InstructionPointer);
    printf("Registers:\n");

    for (uint16_t i = 0; i < Frame -> Function -> RegisterCount; i++) {
        printf("  Register%-3u = %llu\n", i, (uint64_t) Frame -> Registers[i]);
    }
}
