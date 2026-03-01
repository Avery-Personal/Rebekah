#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "AST.h"

#define PARSER_TRACE 0

typedef struct {
    TokenStream *Tokens;
    Token *Current;

    int HasError;
} Parser;

typedef struct {
    const char *FunctionName;

    int Depth;
} TraceEntry;

int IsSyncToken(TokenType Type);
int GetOperatorPrecedence(TokenType Type);
ASTOperator TokenToOperator(TokenType Type);

Parser CreateParser(TokenStream *Tokens);

ASTProgram *MergePrograms(ASTProgram *Prelude, ASTProgram *User);

ASTProgram *ParseProgram(Parser *_Parser);
ASTSubprogram *ParseSubprogram(Parser *_Parser);

ASTStatement *ParseStatement(Parser *_Parser);
ASTStatement *ParseBlock(Parser *_Parser);
ASTStatement *ParseVariableDeclaration(Parser *_Parser, int Mutability);

ASTStatement *ParseIfStatement(Parser *_Parser);
ASTStatement *ParseWhileStatement(Parser *_Parser);
ASTStatement *ParseRepeatStatement(Parser *_Parser);
ASTStatement *ParseForStatement(Parser *_Parser);
ASTStatement *ParseReturnStatement(Parser *_Parser);

ASTExpression *ParseExpression(Parser *_Parser);
ASTExpression *ParseBinary(Parser *_Parser, int Precedence);
ASTExpression *ParseUnary(Parser *_Parser);
ASTExpression *ParsePrimary(Parser *_Parser);
ASTExpression *ParsePostfix(Parser *_Parser);
ASTExpression *ParseArrayLiteral(Parser *_Parser);

ASTType *ParseType(Parser *_Parser);

Token *ParserPeek(Parser *_Parser);
Token *ParserPrevious(Parser *_Parser);
Token *ParserAdvance(Parser *_Parser);

int ParserMatch(Parser *_Parser, TokenType Type);
int ParserCheck(Parser *_Parser, TokenType Type);
int ParserCheckNext(Parser *_Parser, TokenType Type);

static void ParserError(Parser *_Parser, const char *Message);

static void TraceEnter(const char *FunctionName, Parser *_Parser);
static void TraceExit(const char *FunctionName, Parser *_Parser);
static void TraceToken(const char *Action, Parser *_Parser);

#endif
