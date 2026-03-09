#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Semantics.h"

ASTType *INTEGER_TYPE;
ASTType *VOID_TYPE;
ASTType *ANY_TYPE;

static int StringsEqual(const char *A, size_t LenA, const char *B, size_t LenB) {
    if (A == NULL || B == NULL) return A == B;
    if (LenA != LenB) return 0;
    
    return memcmp(A, B, LenA) == 0;
}

static size_t GetTokenLength(const char *String) {
    if (String == NULL) return 0;
    
    size_t Len = 0;
    while (String[Len] != '\0' && String[Len] != ' ' && String[Len] != '\t' && String[Len] != '\n' && String[Len] != '\r' && String[Len] != '(' && String[Len] != ')' && String[Len] != '[' && String[Len] != ']' && String[Len] != '<' && String[Len] != '>' && String[Len] != ',' && String[Len] != ':' && String[Len] != ';') {
        Len++;
    }
    
    return Len;
}

SemanticAnalyzer *CreateSemanticAnalyzer(void) {
    SemanticAnalyzer *Analyzer = calloc(1, sizeof(SemanticAnalyzer));
    
    Analyzer -> CurrentScope = NULL;
    Analyzer -> ScopeDepth = 0;
    Analyzer -> HasError = 0;
    Analyzer -> ErrorCount = 0;
    Analyzer -> CurrentFunction = NULL;
    Analyzer -> InLoop = 0;
    
    EnterScope(Analyzer);

    VOID_TYPE = calloc(1, sizeof(ASTType));
    INTEGER_TYPE = calloc(1, sizeof(ASTType));
    ANY_TYPE = calloc(1, sizeof(ASTType));

    VOID_TYPE -> Name = "void";
    INTEGER_TYPE -> Name = "Integer";
    ANY_TYPE -> Name = "Any";

    DeclareSymbol(Analyzer, SYMBOL_FUNCTION, "output", VOID_TYPE, 0);
    DeclareSymbol(Analyzer, SYMBOL_FUNCTION, "input", ANY_TYPE, 0);
    
    return Analyzer;
}

void DestroySemanticAnalyzer(SemanticAnalyzer *Analyzer) {
    while (Analyzer -> CurrentScope != NULL) {
        ExitScope(Analyzer);
    }
    
    free(Analyzer);
}

void EnterScope(SemanticAnalyzer *Analyzer) {
    Scope *NewScope = calloc(1, sizeof(Scope));
    
    NewScope -> Symbols = NULL;
    NewScope -> Parent = Analyzer -> CurrentScope;
    NewScope -> Depth = Analyzer -> ScopeDepth++;
    
    Analyzer -> CurrentScope = NewScope;
}

void ExitScope(SemanticAnalyzer *Analyzer) {
    if (Analyzer -> CurrentScope == NULL) return;
    
    Scope *OldScope = Analyzer -> CurrentScope;

    Analyzer -> CurrentScope = OldScope -> Parent;
    Analyzer -> ScopeDepth--;
    
    Symbol *Current = OldScope -> Symbols;

    while (Current != NULL) {
        Symbol *Next = Current -> Next;

        free(Current);

        Current = Next;
    }
    
    free(OldScope);
}

Symbol *DeclareSymbol(SemanticAnalyzer *Analyzer, SymbolKind Kind, const char *Name, ASTType *Type, int Mutable) {
    Symbol *Existing = LookupSymbolInCurrentScope(Analyzer, Name);
    if (Existing != NULL) {
        char ErrorMsg[256];

        size_t NameLen = GetTokenLength(Name);

        snprintf(ErrorMsg, sizeof(ErrorMsg), "symbol '%.*s' is already declared in this scope", (int)NameLen, Name);
        SemanticError(Analyzer, ErrorMsg);

        return NULL;
    }
    
    Symbol *NewSymbol = calloc(1, sizeof(Symbol));

    NewSymbol -> Kind = Kind;
    NewSymbol -> Name = Name;
    NewSymbol -> Type = Type;
    NewSymbol -> Mutable = Mutable;
    NewSymbol -> ScopeDepth = Analyzer -> CurrentScope -> Depth;
    NewSymbol -> Next = Analyzer -> CurrentScope -> Symbols;
    
    Analyzer -> CurrentScope -> Symbols = NewSymbol;
    
    return NewSymbol;
}

Symbol *LookupSymbol(SemanticAnalyzer *Analyzer, const char *Name) {
    Scope *CurrentScope = Analyzer -> CurrentScope;

    size_t NameLen = GetTokenLength(Name);
    
    while (CurrentScope != NULL) {
        Symbol *Current = CurrentScope -> Symbols;

        while (Current != NULL) {
            size_t CurrentLen = GetTokenLength(Current -> Name);

            if (StringsEqual(Current -> Name, CurrentLen, Name, NameLen)) {
                return Current;
            }

            Current = Current -> Next;
        }

        CurrentScope = CurrentScope -> Parent;
    }
    
    return NULL;
}

Symbol *LookupSymbolInCurrentScope(SemanticAnalyzer *Analyzer, const char *Name) {
    if (Analyzer -> CurrentScope == NULL) return NULL;
    
    Symbol *Current = Analyzer -> CurrentScope -> Symbols;

    size_t NameLen = GetTokenLength(Name);

    while (Current != NULL) {
        size_t CurrentLen = GetTokenLength(Current -> Name);

        if (StringsEqual(Current -> Name, CurrentLen, Name, NameLen)) {
            return Current;
        }

        Current = Current -> Next;
    }
    
    return NULL;
}

int TypesEqual(ASTType *A, ASTType *B) {
    if (A == NULL || B == NULL) return A == B;
    
    if (A -> Kind == TYPE_ARRAY || B -> Kind == TYPE_ARRAY) {
        if (A -> Kind != B -> Kind) return 0;

        return TypesEqual(A -> ElementType, B -> ElementType);
    }

    if (A -> Name != NULL && B -> Name != NULL) {
        size_t LenA = GetTokenLength(A -> Name);
        size_t LenB = GetTokenLength(B -> Name);

        return StringsEqual(A -> Name, LenA, B -> Name, LenB);
    }

    return A -> Kind == B -> Kind;
}

const char *TypeToString(ASTType *Type) {
    if (Type == NULL) return "void";
    
    static char Buffer[256];
    
    switch (Type -> Kind) {
        case TYPE_ARRAY: {
            const char *ElementString = TypeToString(Type -> ElementType);

            snprintf(Buffer, sizeof(Buffer), "Array<%s>", ElementString);

            return Buffer;
        }

        default:
            if (Type -> Name) {
                size_t NameLen = GetTokenLength(Type -> Name);

                snprintf(Buffer, sizeof(Buffer), "%.*s", (int) NameLen, Type -> Name);

                return Buffer;
            }

            switch (Type -> Kind) {
                case TYPE_BOOL: return "Boolean";
                case TYPE_INT: return "Integer";
                case TYPE_FLOAT: return "Float";
                case TYPE_CHAR: return "Character";
                case TYPE_STRING: return "String";
                case TYPE_VOID: return "void";

                default: return "Unknown";
            }
    }
}

ASTType *GetExpressionType(SemanticAnalyzer *Analyzer, ASTExpression *Expression) {
    if (Expression == NULL) return NULL;
    
    switch (Expression -> Kind) {
        case EXPR_LITERAL: {
            ASTType *Type = calloc(1, sizeof(ASTType));

            switch (Expression -> Literal.LiteralKind) {
                case TYPE_FLOAT: Type -> Name = "Float"; Type -> Kind = TYPE_FLOAT;  break;
                case TYPE_BOOL: Type -> Name = "Boolean"; Type -> Kind = TYPE_BOOL; break;
                case TYPE_STRING: Type -> Name = "String"; Type -> Kind = TYPE_STRING;  break;
                case TYPE_CHAR: Type -> Name = "Character"; Type -> Kind = TYPE_CHAR; break;

                default: Type -> Name = "Integer"; Type -> Kind = TYPE_INT; break;
            }
            
            return Type;
        }
        
        case EXPR_IDENTIFIER: {
            Symbol *_Symbol = LookupSymbol(Analyzer, Expression -> Identifier);
            if (_Symbol == NULL) {
                char ErrorMsg[256];

                size_t NameLen = GetTokenLength(Expression -> Identifier);

                snprintf(ErrorMsg, sizeof(ErrorMsg), "undefined variable '%.*s'", (int)NameLen, Expression -> Identifier);
                SemanticError(Analyzer, ErrorMsg);

                return NULL;
            }

            return _Symbol -> Type;
        }
        
        case EXPR_BINARY: {
            ASTType *LeftType = GetExpressionType(Analyzer, Expression -> Binary.Left);
            ASTType *RightType = GetExpressionType(Analyzer, Expression -> Binary.Right);
            
            if (LeftType == NULL || RightType == NULL) return NULL;
            if (!TypesEqual(LeftType, RightType) && RightType != ANY_TYPE) {
                char ErrorMsg[256];

                snprintf(ErrorMsg, sizeof(ErrorMsg), "type mismatch in binary operation: '%s' and '%s'", TypeToString(LeftType), TypeToString(RightType));
                SemanticError(Analyzer, ErrorMsg);
            }
            
            return LeftType;
        }
        
        case EXPR_UNARY: {
            return GetExpressionType(Analyzer, Expression -> Unary.Operand);
        }
        
        case EXPR_CALL: {
            if (Expression -> Call.Callee -> Kind != EXPR_IDENTIFIER) {
                SemanticError(Analyzer, "can only call functions by name");

                return NULL;
            }
            
            Symbol *_Symbol = LookupSymbol(Analyzer, Expression -> Call.Callee -> Identifier);
            if (_Symbol == NULL) {
                char ErrorMsg[256];

                size_t NameLen = GetTokenLength(Expression -> Call.Callee -> Identifier);

                snprintf(ErrorMsg, sizeof(ErrorMsg), "undefined function '%.*s'", (int)NameLen, Expression -> Call.Callee -> Identifier);
                SemanticError(Analyzer, ErrorMsg);

                return NULL;
            }
            
            if (_Symbol -> Kind != SYMBOL_FUNCTION && _Symbol -> Kind != SYMBOL_METHOD && _Symbol -> Kind != SYMBOL_PROCEDURE) {
                char ErrorMsg[256];

                size_t NameLen = GetTokenLength(Expression -> Call.Callee -> Identifier);

                snprintf(ErrorMsg, sizeof(ErrorMsg), "'%.*s' is not a function", (int)NameLen, Expression -> Call.Callee -> Identifier);
                SemanticError(Analyzer, ErrorMsg);

                return NULL;
            }
            
            return _Symbol -> Type;
        }
        
        case EXPR_INDEX: {
            ASTType *TargetType = GetExpressionType(Analyzer, Expression -> Index.Target);
            if (TargetType == NULL) return NULL;

            if (TargetType -> Kind != TYPE_ARRAY) {
                SemanticError(Analyzer, "cannot index non-array type");

                return NULL;
            }
            
            return TargetType -> ElementType;
        }
        
        case EXPR_ARRAY_LITERAL: {
            if (Expression -> Array.Count == 0) {
                SemanticError(Analyzer, "cannot infer type of empty array literal");
                
                return NULL;
            }
            
            ASTType *ElementType = GetExpressionType(Analyzer, Expression -> Array.Elements[0]);
            
            for (size_t i = 1; i < Expression -> Array.Count; i++) {
                ASTType *Type = GetExpressionType(Analyzer, Expression -> Array.Elements[i]);
                if (!TypesEqual(ElementType, Type)) {
                    SemanticError(Analyzer, "array literal elements must all have the same type");
                }
            }
            
            ASTType *ArrayType = calloc(1, sizeof(ASTType));

            ArrayType -> Kind = TYPE_ARRAY;
            ArrayType -> ElementType = ElementType;

            return ArrayType;
        }
        
        default:
            return NULL;
    }
}

void AnalyzeType(SemanticAnalyzer *Analyzer, ASTType *Type) {
    if (Type == NULL) return;
    
    switch (Type -> Kind) {
        case TYPE_ARRAY:
            AnalyzeType(Analyzer, Type -> ElementType);

            break;
            
        default:
            break;
    }
}

void AnalyzeExpression(SemanticAnalyzer *Analyzer, ASTExpression *Expression) {
    if (Expression == NULL) return;
    
    switch (Expression -> Kind) {
        case EXPR_LITERAL:
            break;
            
        case EXPR_IDENTIFIER: {
            break;
        }
        
        case EXPR_BINARY:
            ASTType *LeftType = GetExpressionType(Analyzer, Expression -> Binary.Left);
            ASTType *RightType = GetExpressionType(Analyzer, Expression -> Binary.Right);
            
            if (LeftType == NULL || RightType == NULL) return;
            if (!TypesEqual(LeftType, RightType)) {
                char LeftTypeString[256];
                char RightTypeString[256];

                char ErrorMsg[512];
                
                snprintf(LeftTypeString, sizeof(LeftTypeString), "%s", TypeToString(LeftType));
                snprintf(RightTypeString, sizeof(RightTypeString), "%s", TypeToString(RightType));
                
                snprintf(ErrorMsg, sizeof(ErrorMsg), "type mismatch in binary operation: '%s' and '%s'", LeftTypeString, RightTypeString);
                SemanticError(Analyzer, ErrorMsg);
            }

            break;
            
        case EXPR_UNARY:
            AnalyzeExpression(Analyzer, Expression -> Unary.Operand);
            
            break;
            
        case EXPR_CALL: {
            AnalyzeExpression(Analyzer, Expression -> Call.Callee);
            
            for (size_t i = 0; i < Expression -> Call.ArgCount; i++) {
                AnalyzeExpression(Analyzer, Expression -> Call.Args[i]);
            }
            
            if (Expression -> Call.Callee -> Kind == EXPR_IDENTIFIER) {
                Symbol *_Symbol = LookupSymbol(Analyzer, Expression -> Call.Callee -> Identifier);

                if (_Symbol != NULL && (_Symbol -> Kind == SYMBOL_FUNCTION || _Symbol -> Kind == SYMBOL_METHOD || _Symbol -> Kind == SYMBOL_PROCEDURE)) {
                    if (StringsEqual(_Symbol -> Name, GetTokenLength(_Symbol -> Name), "input", 5)) {
                        if (Expression -> Call.ArgCount > 1) {
                            SemanticError(Analyzer, "input() takes at most 1 argument");
                        }
                    }
                }
            }

            break;
        }
        
        case EXPR_INDEX:
            AnalyzeExpression(Analyzer, Expression -> Index.Target);
            AnalyzeExpression(Analyzer, Expression -> Index.Index);
            
            GetExpressionType(Analyzer, Expression);

            break;
            
        case EXPR_ARRAY_LITERAL:
            for (size_t i = 0; i < Expression -> Array.Count; i++) {
                AnalyzeExpression(Analyzer, Expression -> Array.Elements[i]);
            }
            
            GetExpressionType(Analyzer, Expression);
            
            break;
    }
}

void AnalyzeStatement(SemanticAnalyzer *Analyzer, ASTStatement *Statement) {
    if (Statement == NULL) return;
    
    switch (Statement -> Kind) {
        case STMT_VAR_DECL: {
            AnalyzeType(Analyzer, Statement -> VarDecl.Type);
            DeclareSymbol(Analyzer, SYMBOL_VAR, Statement -> VarDecl.Name, Statement -> VarDecl.Type, Statement -> VarDecl.Mutable);
            
            if (Statement -> VarDecl.Initializer != NULL) {
                AnalyzeExpression(Analyzer, Statement -> VarDecl.Initializer);
                
                ASTType *InitializerType = GetExpressionType(Analyzer, Statement -> VarDecl.Initializer);
                if (InitializerType != NULL && !TypesEqual(Statement -> VarDecl.Type, InitializerType) && InitializerType != ANY_TYPE) {
                    char ExpectedType[256];
                    char ActualType[256];

                    char ErrorMsg[512];

                    snprintf(ExpectedType, sizeof(ExpectedType), "%s", TypeToString(Statement -> VarDecl.Type));
                    snprintf(ActualType, sizeof(ActualType), "%s", TypeToString(InitializerType));
                    
                    snprintf(ErrorMsg, sizeof(ErrorMsg), "type mismatch in variable initialization: expected '%s', got '%s'", ExpectedType, ActualType);
                    SemanticError(Analyzer, ErrorMsg);
                }
            }

            break;
        }
        
        case STMT_ASSIGN: {
            AnalyzeExpression(Analyzer, Statement -> Assign.Target);
            AnalyzeExpression(Analyzer, Statement -> Assign.Value);
            
            if (Statement -> Assign.Target -> Kind == EXPR_IDENTIFIER) {
                Symbol *_Symbol = LookupSymbol(Analyzer, Statement -> Assign.Target -> Identifier);
                if (_Symbol != NULL && !_Symbol -> Mutable) {
                    char ErrorMsg[256];

                    size_t NameLen = GetTokenLength(Statement -> Assign.Target -> Identifier);

                    snprintf(ErrorMsg, sizeof(ErrorMsg), "cannot assign to immutable variable '%.*s'", (int) NameLen, Statement -> Assign.Target -> Identifier);
                    SemanticError(Analyzer, ErrorMsg);
                }
            }
            
            ASTType *TargetType = GetExpressionType(Analyzer, Statement -> Assign.Target);
            ASTType *ValueType = GetExpressionType(Analyzer, Statement -> Assign.Value);
            
            if (TargetType != NULL && ValueType != NULL && !TypesEqual(TargetType, ValueType)) {
                char ExpectedType[256];
                char ActualType[256];

                char ErrorMsg[256];

                snprintf(ExpectedType, sizeof(ExpectedType), "%s", TypeToString(TargetType));
                snprintf(ActualType, sizeof(ActualType), "%s", TypeToString(ValueType));
                
                snprintf(ErrorMsg, sizeof(ErrorMsg), "type mismatch in assignment: expected '%s', got '%s'", ExpectedType, ActualType);
                SemanticError(Analyzer, ErrorMsg);
            }

            break;
        }
        
        case STMT_IF: {
            AnalyzeExpression(Analyzer, Statement -> If.Condition);
            EnterScope(Analyzer);

            for (size_t i = 0; i < Statement -> If.ThenCount; i++) {
                AnalyzeStatement(Analyzer, Statement -> If.ThenBlock[i]);
            }

            ExitScope(Analyzer);
            
            for (size_t i = 0; i < Statement -> If.ElseIfCount; i++) {
                AnalyzeExpression(Analyzer, Statement -> If.ElseIfs[i].Condition);
                EnterScope(Analyzer);

                for (size_t j = 0; j < Statement -> If.ElseIfs[i].Count; j++) {
                    AnalyzeStatement(Analyzer, Statement -> If.ElseIfs[i].Block[j]);
                }

                ExitScope(Analyzer);
            }
            
            if (Statement -> If.ElseBlock != NULL) {
                EnterScope(Analyzer);

                for (size_t i = 0; i < Statement -> If.ElseCount; i++) {
                    AnalyzeStatement(Analyzer, Statement -> If.ElseBlock[i]);
                }

                ExitScope(Analyzer);
            }

            break;
        }
        
        case STMT_WHILE: {
            AnalyzeExpression(Analyzer, Statement -> While.Condition);
            
            int WasInLoop = Analyzer -> InLoop;

            Analyzer -> InLoop = 1;
            
            EnterScope(Analyzer);

            for (size_t i = 0; i < Statement -> While.Count; i++)
                AnalyzeStatement(Analyzer, Statement -> While.Body[i]);

            ExitScope(Analyzer);
            
            Analyzer -> InLoop = WasInLoop;

            break;
        }
        
        case STMT_REPEAT: {
            int WasInLoop = Analyzer -> InLoop;

            Analyzer -> InLoop = 1;
            
            EnterScope(Analyzer);

            for (size_t i = 0; i < Statement -> Repeat.Count; i++)
                AnalyzeStatement(Analyzer, Statement -> Repeat.Body[i]);

            ExitScope(Analyzer);
            
            AnalyzeExpression(Analyzer, Statement -> Repeat.Until);
            
            Analyzer -> InLoop = WasInLoop;

            break;
        }
        
        case STMT_FOR: {
            EnterScope(Analyzer);
            
            ASTType *IntType = calloc(1, sizeof(ASTType));

            IntType -> Name = "Integer";

            DeclareSymbol(Analyzer, SYMBOL_VAR, Statement -> For.Iterator, IntType, 0);
            
            AnalyzeExpression(Analyzer, Statement -> For.Start);
            AnalyzeExpression(Analyzer, Statement -> For.End);
            
            int WasInLoop = Analyzer -> InLoop;

            Analyzer -> InLoop = 1;
            
            for (size_t i = 0; i < Statement -> For.Count; i++) {
                AnalyzeStatement(Analyzer, Statement -> For.Body[i]);
            }
            
            Analyzer -> InLoop = WasInLoop;

            ExitScope(Analyzer);

            break;
        }
        
        case STMT_RETURN: {
            if (Analyzer -> CurrentFunction == NULL) {
                SemanticError(Analyzer, "return statement outside of function");

                break;
            }
            
            if (Statement -> Return.Value != NULL) {
                AnalyzeExpression(Analyzer, Statement -> Return.Value);
                
                ASTType *ReturnType = GetExpressionType(Analyzer, Statement -> Return.Value);

                if (Analyzer -> CurrentFunction -> ReturnType != NULL && !TypesEqual(Analyzer -> CurrentFunction -> ReturnType, ReturnType)) {
                    char ExpectedType[256];
                    char ActualType[256];

                    char ErrorMsg[256];

                    snprintf(ExpectedType, sizeof(ExpectedType), "%s", TypeToString(Analyzer -> CurrentFunction -> ReturnType));
                    snprintf(ActualType, sizeof(ActualType), "%s", TypeToString(ReturnType));

                    snprintf(ErrorMsg, sizeof(ErrorMsg), "return type mismatch: expected '%s', got '%s'", ExpectedType, ActualType);
                    SemanticError(Analyzer, ErrorMsg);
                }
            } else {
                if (Analyzer -> CurrentFunction -> ReturnType != NULL) {
                    SemanticError(Analyzer, "missing return value in function that returns a value");
                }
            }

            break;
        }
        
        case STMT_BREAK:
            if (!Analyzer -> InLoop) {
                SemanticError(Analyzer, "break statement outside of loop");
            }

            break;
            
        case STMT_CONTINUE:
            if (!Analyzer -> InLoop) {
                SemanticError(Analyzer, "continue statement outside of loop");
            }

            break;
            
        case STMT_CALL:
            AnalyzeExpression(Analyzer, Statement -> ExpressionStmt.Expression);

            break;
            
        case STMT_BLOCK:
            EnterScope(Analyzer);
            
            for (int i = 0; i < Statement -> Block.SubprogramCount; i++) {
                ASTSubprogram *Subprogram = Statement -> Block.Subprograms[i];
                SymbolKind Kind;

                switch (Subprogram -> Kind) {
                    case TOKEN_METHOD: Kind = SYMBOL_METHOD; break;
                    case TOKEN_PROCEDURE: Kind = SYMBOL_PROCEDURE; break;
                    case TOKEN_FUNCTION: Kind = SYMBOL_FUNCTION; break;

                    default: Kind = SYMBOL_FUNCTION; break;
                }
                
                DeclareSymbol(Analyzer, Kind, Subprogram -> Name, Subprogram -> ReturnType, 0);
            }

            for (size_t i = 0; i < Statement -> Block.Count; i++) {
                AnalyzeStatement(Analyzer, Statement -> Block.Statements[i]);
            }
            
            for (int i = 0; i < Statement -> Block.SubprogramCount; i++) {
                ASTSubprogram *Subprogram = Statement -> Block.Subprograms[i];
                
                EnterScope(Analyzer);
                
                for (size_t j = 0; j < Subprogram -> ParamCount; j++) {
                    AnalyzeType(Analyzer, Subprogram -> Params[j].Type);
                    DeclareSymbol(Analyzer, SYMBOL_PARAM, Subprogram -> Params[j].Name, Subprogram -> Params[j].Type, 0);
                }
                
                if (Subprogram -> ReturnType != NULL) {
                    AnalyzeType(Analyzer, Subprogram -> ReturnType);
                }
                
                ASTSubprogram *PreviousFunction = Analyzer -> CurrentFunction;

                Analyzer -> CurrentFunction = Subprogram;
                
                for (size_t j = 0; j < Subprogram -> BodyCount; j++) {
                    AnalyzeStatement(Analyzer, Subprogram -> Body[j]);
                }
                
                Analyzer -> CurrentFunction = PreviousFunction;
                
                ExitScope(Analyzer);
            }

            ExitScope(Analyzer);

            break;
    }
}

void AnalyzeSubprogram(SemanticAnalyzer *Analyzer, ASTSubprogram *Subprogram) {
    if (Subprogram == NULL) return;
    
    SymbolKind Kind;

    switch (Subprogram -> Kind) {
        case TOKEN_METHOD: Kind = SYMBOL_METHOD; break;
        case TOKEN_PROCEDURE: Kind = SYMBOL_PROCEDURE; break;
        case TOKEN_FUNCTION: Kind = SYMBOL_FUNCTION; break;

        default: Kind = SYMBOL_FUNCTION; break;
    }
    
    DeclareSymbol(Analyzer, Kind, Subprogram -> Name, Subprogram -> ReturnType, 0);
    
    EnterScope(Analyzer);
    
    for (size_t i = 0; i < Subprogram -> ParamCount; i++) {
        AnalyzeType(Analyzer, Subprogram -> Params[i].Type);
        DeclareSymbol(Analyzer, SYMBOL_PARAM, Subprogram -> Params[i].Name, Subprogram -> Params[i].Type, Subprogram -> Params[i].Mutable);
    }
    
    if (Subprogram -> ReturnType != NULL) {
        AnalyzeType(Analyzer, Subprogram -> ReturnType);
    }
    
    ASTSubprogram *PreviousFunction = Analyzer -> CurrentFunction;

    Analyzer -> CurrentFunction = Subprogram;
    
    for (size_t i = 0; i < Subprogram -> BodyCount; i++) {
        AnalyzeStatement(Analyzer, Subprogram -> Body[i]);
    }
    
    Analyzer -> CurrentFunction = PreviousFunction;
    
    ExitScope(Analyzer);
}

void AnalyzeSubprogramBody(SemanticAnalyzer *Analyzer, ASTSubprogram *Subprogram) {
    if (Subprogram == NULL) return;

    EnterScope(Analyzer);

    for (size_t i = 0; i < Subprogram -> ParamCount; i++) {
        AnalyzeType(Analyzer, Subprogram -> Params[i].Type);
        DeclareSymbol(Analyzer, SYMBOL_PARAM, Subprogram -> Params[i].Name, Subprogram -> Params[i].Type, Subprogram -> Params[i].Mutable);
    }

    if (Subprogram -> ReturnType != NULL) {
        AnalyzeType(Analyzer, Subprogram -> ReturnType);
    }

    ASTSubprogram *PreviousFunction = Analyzer -> CurrentFunction;

    Analyzer -> CurrentFunction = Subprogram;

    for (size_t i = 0; i < Subprogram -> BodyCount; i++) {
        AnalyzeStatement(Analyzer, Subprogram -> Body[i]);
    }

    Analyzer -> CurrentFunction = PreviousFunction;

    ExitScope(Analyzer);
}

int AnalyzeProgram(SemanticAnalyzer *Analyzer, ASTProgram *Program) {
    if (Program == NULL) return 0;

    for (size_t i = 0; i < Program -> SubprogramCount; i++) {
        ASTSubprogram *Subprogram = Program -> Subprograms[i];
        SymbolKind Kind;

        switch (Subprogram -> Kind) {
            case TOKEN_METHOD: Kind = SYMBOL_METHOD; break;
            case TOKEN_PROCEDURE: Kind = SYMBOL_PROCEDURE; break;

            default: Kind = SYMBOL_FUNCTION; break;
        }

        DeclareSymbol(Analyzer, Kind, Subprogram -> Name, Subprogram -> ReturnType, 0);
    }
    
    for (size_t i = 0; i < Program -> StatementCount; i++) {
        ASTStatement *Statement = Program -> Statements[i];

        if (Statement -> Kind == STMT_VAR_DECL) {
            AnalyzeStatement(Analyzer, Statement);
        }
    }

    for (size_t i = 0; i < Program -> SubprogramCount; i++) {
        AnalyzeSubprogramBody(Analyzer, Program -> Subprograms[i]);
    }

    for (size_t i = 0; i < Program -> StatementCount; i++) {
        ASTStatement *Statement = Program -> Statements[i];

        if (Statement -> Kind != STMT_VAR_DECL) {
            AnalyzeStatement(Analyzer, Statement);
        }
    }
    
    if (Analyzer -> ErrorCount > 0) {
        fprintf(stderr, "\nSemantic analysis failed with %d error(s)\n", Analyzer -> ErrorCount);
    }
    
    return !Analyzer -> HasError;
}

void SemanticError(SemanticAnalyzer *Analyzer, const char *Message) {
    fprintf(stderr, "[Semantic Error] %s\n", Message);

    Analyzer -> HasError = 1;
    Analyzer -> ErrorCount++;
}

void SemanticWarning(SemanticAnalyzer *Analyzer, const char *Message) {
    fprintf(stderr, "[Semantic Warning] %s\n", Message);
}
