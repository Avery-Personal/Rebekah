#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "Parser.h"

typedef enum {
    SYMBOL_VAR,
    SYMBOL_PARAM,
    SYMBOL_FUNCTION,
    SYMBOL_METHOD,
    SYMBOL_PROCEDURE
} SymbolKind;

typedef struct Symbol {
    const char *Name;

    SymbolKind Kind;
    ASTType *Type;
    
    int Mutable;

    int ScopeDepth;
    struct Symbol *Next;
} Symbol;

typedef struct Scope {
    struct Scope *Parent;
    
    Symbol *Symbols;
    int Depth;
} Scope;

typedef struct SemanticAnalyzer {
    Scope *CurrentScope;
    int ScopeDepth;

    int HasError;
    int ErrorCount;
    
    ASTSubprogram *CurrentFunction;
    int InLoop;
} SemanticAnalyzer;

SemanticAnalyzer *CreateSemanticAnalyzer(void);
void DestroySemanticAnalyzer(SemanticAnalyzer *Analyzer);

int AnalyzeProgram(SemanticAnalyzer *Analyzer, ASTProgram *Program);
void AnalyzeSubprogram(SemanticAnalyzer *Analyzer, ASTSubprogram *Subprogram);

void AnalyzeStatement(SemanticAnalyzer *Analyzer, ASTStatement *Statement);
void AnalyzeExpression(SemanticAnalyzer *Analyzer, ASTExpression *Expression);
void AnalyzeType(SemanticAnalyzer *Analyzer, ASTType *Type);

void EnterScope(SemanticAnalyzer *Analyzer);
void ExitScope(SemanticAnalyzer *Analyzer);

Symbol *DeclareSymbol(SemanticAnalyzer *Analyzer, SymbolKind Kind, const char *Name, ASTType *Type, int Mutable);
Symbol *LookupSymbol(SemanticAnalyzer *Analyzer, const char *Name);
Symbol *LookupSymbolInCurrentScope(SemanticAnalyzer *Analyzer, const char *Name);

int TypesEqual(ASTType *A, ASTType *B);
const char *TypeToString(ASTType *Type);
ASTType *GetExpressionType(SemanticAnalyzer *Analyzer, ASTExpression *Expr);

void SemanticError(SemanticAnalyzer *Analyzer, const char *Message);
void SemanticWarning(SemanticAnalyzer *Analyzer, const char *Message);

#endif
