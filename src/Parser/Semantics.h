#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "Parser.h"

typedef enum {
    SYMBOL_VAR,
    SYMBOL_PARAM,
    SYMBOL_SUBPROGRAM
} SymbolKind;

typedef enum {
    SEM_ERROR,
    SEM_WARNING,
    SEM_NOTE
} SemanticSeverity;

typedef struct {
    SemanticSeverity Severity;
    
    const char *Message;
    const char *Detail;

    unsigned Line;
    unsigned Column;
} SemanticError;

typedef struct {
    SemanticError *Errors;

    int Count;
    int HadFatal;
} SemanticContext;

typedef struct Symbol {
    const char *Name;

    SymbolKind Kind;
    ASTType *Type;
    
    int Mutable;

    ASTSubprogram *Subprogram;
} Symbol;

typedef struct Scope {
    struct Scope *Parent;

    Symbol **Symbols;
    int Count;
} Scope;

void AnalyzeProgram(ASTProgram *Program);
void AnalyzeSubprogram(Scope *Global, ASTSubprogram *Function);

void AnalyzeStatement(Scope *_Scope, ASTStatement *Statement);
ASTType *AnalyzeExpression(Scope *_Scope, ASTExpression *Expression);

Scope *CreateScope(Scope *Parent);
void AddSymbol(Scope *_Scope, Symbol *_Symbol);
Symbol *FindSymbol(Scope *_Scope, const char *Name);

#endif
