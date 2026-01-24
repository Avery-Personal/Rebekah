#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "AST.h"

typedef struct {
    TokenStream *Tokens;
    Token *Current;

    int HasError;
} Parser;

ASTProgram *ParseProgram(Lexer *_Lexer);
ASTSubprogram* ParseSubprogram(Lexer *_Lexer);

ASTStatement* ParseStatement(Lexer *_Lexer);
ASTStatement* ParseBlock(Lexer *_Lexer);
ASTStatement* ParseVariableDeclaration(Lexer *_Lexer);

ASTStatement* ParseIfStatement(Lexer *_Lexer);
ASTStatement* ParseWhileStatement(Lexer *_Lexer);
ASTStatement* ParseRepeatStatement(Lexer *_Lexer);
ASTStatement* ParseForStatement(Lexer *_Lexer);

ASTExpression* ParseExpression(Lexer *_Lexer);
ASTExpression* ParseLiteral(Lexer *_Lexer);
ASTExpression* ParseIdentifierOrCall(Lexer *_Lexer);

ASTType* ParseType(Lexer *_Lexer);

Token* ParserPeek(Parser *_Parser);
Token* ParserPrevious(Parser *_Parser);
Token* ParserAdvance(Parser *_Parser);

int ParserMatch(Parser *_Parser, TokenType Type);
int ParserCheck(Parser *_Parser, TokenType Type);

static void ParserError(Parser *_Parser, const char *Message);

#endif
