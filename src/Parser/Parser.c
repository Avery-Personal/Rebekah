#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"

ASTProgram *ParseProgram(Lexer *_Lexer) {

}

Token *ParserPeek(Parser *_Parser) {
    return _Parser -> Current;
}

Token* ParserPrevious(Parser *_Parser) {
    return &_Parser -> Tokens -> Data[_Parser -> Tokens -> Cursor - 1];
}

Token* ParserAdvance(Parser *_Parser) {
    if (_Parser -> Current->Type != TOKEN_EOF)
        _Parser -> Current = &_Parser -> Tokens -> Data[++_Parser -> Tokens -> Cursor];

    return ParserPrevious(_Parser);
}

int ParserCheck(Parser *_Parser, TokenType Type) {
    return _Parser -> Current -> Type == Type;
}

int ParserMatch(Parser *_Parser, TokenType Type) {
    if (ParserCheck(_Parser, Type)) {
        ParserAdvance(_Parser);

        return 1;
    } 

    return 0;
}

void ParserError(Parser *_Parser, const char *Message) {
    fprintf(stderr, "[Parse Error] Line %u:%u >> %s\n", _Parser -> Current -> Line, _Parser -> Current -> Column, Message);

    _Parser -> HasError = 1;
}
