#ifndef PARSER_H
#define PARSER_H

#include "../Lexer/Lexer.h"
#include "AST.h"

typedef struct {
    TokenStream *Tokens;
    Token *Current;

    int HasError;
} Parser;

ASTProgram *ParseProgram(Parser *_Parser);
ASTSubprogram* ParseSubprogram(Parser *_Parser);

ASTStatement* ParseStatement(Parser *_Parser);
ASTStatement* ParseBlock(Parser *_Parser);
ASTStatement* ParseVariableDeclaration(Parser *_Parser);

ASTStatement* ParseIfStatement(Parser *_Parser);
ASTStatement* ParseWhileStatement(Parser *_Parser);
ASTStatement* ParseRepeatStatement(Parser *_Parser);
ASTStatement* ParseForStatement(Parser *_Parser);

ASTExpression* ParseExpression(Parser *_Parser);
ASTExpression* ParseLiteral(Parser *_Parser);
ASTExpression* ParseIdentifierOrCall(Parser *_Parser);

ASTType* ParseType(Parser *_Parser);

Token* ParserPeek(Parser *_Parser);
Token* ParserPrevious(Parser *_Parser);
Token* ParserAdvance(Parser *_Parser);

int ParserMatch(Parser *_Parser, TokenType Type);
int ParserCheck(Parser *_Parser, TokenType Type);

static void ParserError(Parser *_Parser, const char *Message);

#endif
