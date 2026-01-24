#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"

ASTProgram *ParseProgram(Parser *_Parser) {
    ASTProgram *Program = calloc(1, sizeof(ASTProgram));

    while (!ParserCheck(_Parser, TOKEN_EOF)) {
        if (ParserCheck(_Parser, TOKEN_METHOD) || ParserCheck(_Parser, TOKEN_PROCEDURE) || ParserCheck(_Parser, TOKEN_FUNCTION)) {
            Program -> Subprograms[Program -> SubprogramCount++] = ParseSubprogram(_Parser);
        } else {
            ASTStatement *Statement = ParseStatement(_Parser);

            if (Statement != NULL) {
                Program -> Statements[Program -> StatementCount++] = ParseStatement(_Parser);
            }
        }
    }
}

ASTSubprogram *ParseSubprogram(Parser *_Parser) {
    ASTSubprogram *Function = calloc(1, sizeof(ASTSubprogram));

    Function -> Kind = ParserAdvance(_Parser) -> Type;

    if (!ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
        ParserError(_Parser, "expected subprogram name");

        return Function;
    }

    Function -> Name = ParserPrevious(_Parser) -> Start;

    ParserMatch(_Parser, TOKEN_LPAREN);

    while (!ParserCheck(_Parser, TOKEN_RPAREN) && !ParserCheck(_Parser, TOKEN_EOF)) {
        ASTType *ParamType = ParseType(_Parser);

        if (!ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
            ParserError(_Parser, "expected parameter name");

            break;
        }

        const char *ParamName = ParserPrevious(_Parser) -> Start;

        Function -> Params[Function -> ParamCount].Name = ParamName;
        Function -> Params[Function -> ParamCount].Type = ParamType;
        Function -> ParamCount++;

        ParserMatch(_Parser, TOKEN_COMMA);
    }

    ParserMatch(_Parser, TOKEN_RPAREN);

    if (ParserMatch(_Parser, TOKEN_COLON)) {
        Function -> ReturnType = ParseType(_Parser);
    }

    Function -> Body = ParseBlock(_Parser);

    return Function;
}

ASTStatement *ParseStatement(Parser *_Parser) {
    if (ParserMatch(_Parser, TOKEN_BEGIN)) return ParseBlock(_Parser);
    if (ParserMatch(_Parser, TOKEN_IF)) return ParseIfStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_WHILE)) return ParseWhileStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_REPEAT)) return ParseRepeatStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_FOR)) return ParseForStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_RETURN)) return ParseReturn(_Parser);
    if (ParserMatch(_Parser, TOKEN_MUTABLE) || ParserMatch(_Parser, TOKEN_CONSTANT)) return ParseVariableDeclaration(_Parser);

    ASTStatement *Statement = calloc(1, sizeof(ASTStatement));
    Statement -> ExpressionStmt.Expression = ParseExpression(_Parser);

    return Statement;
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
