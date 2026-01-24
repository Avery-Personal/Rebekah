#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"

int GetOperatorPrecedence(TokenType Type) {
    switch (Type) {
        case TOKEN_OR: return 1;
        case TOKEN_AND: return 2;
        case TOKEN_EQ:
        case TOKEN_NE:
        case TOKEN_LT:
        case TOKEN_LE:
        case TOKEN_GT:
        case TOKEN_GE: return 3;
        case TOKEN_PLUS:
        case TOKEN_MINUS: return 4;
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_SLASH_SLASH:
        case TOKEN_PERCENT: return 5;
        case TOKEN_CARET: return 6;

        default: return 0;
    }
}

Parser CreateParser(TokenStream *Tokens) {
    Parser _Parser = {0};

    _Parser.Tokens = Tokens;
    _Parser.Current = &Tokens -> Data[0];

    _Parser.HasError = 0;

    return _Parser;
}

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

    ASTStatement *Block = ParseBlock(_Parser);

    Function -> Body = Block -> Block.Statements;
    Function -> BodyCount = Block -> Block.Count;

    free(Block);

    return Function;
}

ASTStatement *ParseStatement(Parser *_Parser) {
    if (ParserMatch(_Parser, TOKEN_BEGIN)) return ParseBlock(_Parser);
    if (ParserMatch(_Parser, TOKEN_IF)) return ParseIfStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_WHILE)) return ParseWhileStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_REPEAT)) return ParseRepeatStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_FOR)) return ParseForStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_RETURN)) return ParseReturnStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_MUTABLE) || ParserMatch(_Parser, TOKEN_CONSTANT)) return ParseVariableDeclaration(_Parser);

    ASTStatement *Statement = calloc(1, sizeof(ASTStatement));
    Statement -> ExpressionStmt.Expression = ParseExpression(_Parser);

    return Statement;
}

ASTStatement *ParseBlock(Parser *_Parser) {
    ASTStatement *Block = calloc(1, sizeof(ASTStatement));

    Block -> Kind = STMT_BLOCK;
    

    while (!ParserCheck(_Parser, TOKEN_END) && !ParserCheck(_Parser, TOKEN_EOF))
        Block -> Block.Statements[Block -> Block.Count++] = ParseStatement(_Parser);

    ParserMatch(_Parser, TOKEN_END);

    return Block;
}

ASTStatement *ParseVariableDeclaration(Parser *_Parser) {
    ASTStatement *Variable = calloc(1, sizeof(ASTStatement));
    Variable -> VarDecl.Mutable = ParserMatch(_Parser, TOKEN_MUTABLE);

    ParserMatch(_Parser, TOKEN_IDENTIFIER);
    Variable -> VarDecl.Name = ParserPrevious(_Parser)->Start;

    if (ParserMatch(_Parser, TOKEN_COLON))
        Variable -> VarDecl.Type = ParseType(_Parser);

    if (ParserMatch(_Parser, TOKEN_ASSIGN))
        Variable -> VarDecl.Initializer = ParseExpression(_Parser);

    return Variable;
}

ASTStatement *ParseIfStatement(Parser *_Parser) {
    ASTStatement *If = calloc(1, sizeof(ASTStatement));

    If -> If.Condition = ParseExpression(_Parser);

    ParserMatch(_Parser, TOKEN_THEN);

    ASTStatement *ThenBlock = ParseBlock(_Parser);

    If -> If.ThenBlock = ThenBlock -> Block.Statements;
    If -> If.ThenCount = ThenBlock -> Block.Count;

    free(ThenBlock);

    return If;
}

ASTStatement *ParseWhileStatement(Parser *_Parser) {
    ASTStatement *While = calloc(1, sizeof(ASTStatement));

    While -> While.Condition = ParseExpression(_Parser);

    ParserMatch(_Parser, TOKEN_DO);

    ASTStatement *Body = ParseBlock(_Parser);

    While -> While.Body = Body -> Block.Statements;
    While -> While.Count = Body -> Block.Count;

    free(Body);

    return While;
}

ASTStatement *ParseRepeatStatement(Parser *_Parser) {
    ASTStatement *Repeat = calloc(1, sizeof(ASTStatement));

    ASTStatement *Body = ParseBlock(_Parser);

    Repeat -> Repeat.Body = Body -> Block.Statements;
    Repeat -> Repeat.Count = Body -> Block.Count;

    free(Body);

    ParserMatch(_Parser, TOKEN_UNTIL);

    Repeat -> Repeat.Until = ParseExpression(_Parser);

    return Repeat;
}

ASTStatement *ParseForStatement(Parser *_Parser) {
    ASTStatement *For = calloc(1, sizeof(ASTStatement));

    if (!ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
        ParserError(_Parser, "expected iterator name");

        return For;
    }

    For -> For.Iterator = ParserPrevious(_Parser) -> Start;
    ParserMatch(_Parser, TOKEN_ASSIGN);

    For -> For.Start = ParseExpression(_Parser);
    ParserMatch(_Parser, TOKEN_DOT_DOT);

    For -> For.End = ParseExpression(_Parser);
    ParserMatch(_Parser, TOKEN_DO);

    ASTStatement *Body = ParseBlock(_Parser);

    For -> For.Body = Body -> Block.Statements;
    For -> For.Count = Body -> Block.Count;

    free(Body);

    return For;
}

ASTStatement *ParseReturnStatement(Parser *_Parser) {
    ASTStatement *Return = calloc(1, sizeof(ASTStatement));

    if (!ParserCheck(_Parser, TOKEN_END))
        Return -> Return.Value = ParseExpression(_Parser);

    return Return;
}

ASTExpression *ParseExpression(Parser *_Parser) {
    return ParseBinary(_Parser, 0);
}

ASTExpression *ParseBinary(Parser *_Parser, int Precedence) {
    ASTExpression *Left = ParseUnary(_Parser);

    while (1) {
        Token *OperatorToken = ParserPeek(_Parser);

        int OperatorPrecedence = GetOperatorPrecedence(OperatorToken -> Type);
        if (OperatorPrecedence < Precedence)
            break;

        ParserAdvance(_Parser);

        ASTExpression *Right = ParseBinary(_Parser, Precedence + 1);

        ASTExpression *BinaryExpression = calloc(1, sizeof(ASTExpression));
        BinaryExpression -> Binary.Left = Left;
        BinaryExpression -> Binary.Right = Right;
        BinaryExpression -> Binary.Op = OperatorToken -> Type;

        Left = BinaryExpression;
    }

    return Left;
}

ASTExpression *ParseUnary(Parser *_Parser) {
    if (ParserMatch(_Parser, TOKEN_MINUS) || ParserMatch(_Parser, TOKEN_NOT)) {
        ASTExpression *Expression = calloc(1, sizeof(ASTExpression));

        Expression -> Kind = EXPR_UNARY;
        Expression -> Unary.Op = ParserPrevious(_Parser) -> Type;
        Expression -> Unary.Operand = ParseUnary(_Parser);

        return Expression;
    }

    return ParsePostfix(_Parser);
}

ASTExpression *ParsePrimary(Parser *_Parser) {
    ASTExpression *Expression = calloc(1, sizeof(ASTExpression));

    if (ParserMatch(_Parser, TOKEN_NUMBER)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Number = ParserPrevious(_Parser) -> Literal.Number;

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_INTEGER)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Int = ParserPrevious(_Parser) -> Literal.Int;

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_STRING_LITERAL)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.String = ParserPrevious(_Parser) -> Literal.String;

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_CHAR_LITERAL)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Char = (char) ParserPrevious(_Parser) -> Literal.Int;

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_TRUE)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Bool = 1;

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_FALSE)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Bool = 0;

        return Expression;
    }

    if (ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
        Expression -> Kind = EXPR_IDENTIFIER;
        Expression -> Identifier = ParserPrevious(_Parser) -> Start;

        return Expression;
    }

    if (ParserMatch(_Parser, TOKEN_LPAREN)) {
        Expression = ParseExpression(_Parser);

        ParserMatch(_Parser, TOKEN_RPAREN);

        return Expression;
    }

    if (ParserMatch(_Parser, TOKEN_LBRACKET)) {
        return ParseArrayLiteral(_Parser);
    }

    ParserError(_Parser, "invalid expression");

    return Expression;
}

ASTExpression *ParseArrayLiteral(Parser *_Parser) {
    ASTExpression *ArrayLiteral = calloc(1, sizeof(ASTExpression));

    ArrayLiteral -> Kind = EXPR_ARRAY_LITERAL;
    ArrayLiteral -> Array.Elements = NULL;
    ArrayLiteral -> Array.Count = 0;

    if (ParserMatch(_Parser, TOKEN_RBRACKET)) {
        return ArrayLiteral;
    }

    while (!ParserCheck(_Parser, TOKEN_EOF)) {
        ASTExpression *Element = NULL;

        if (ParserCheck(_Parser, TOKEN_LBRACKET)) {
            ParserAdvance(_Parser);
            Element = ParseArrayLiteral(_Parser);
        } else {
            Element = ParseExpression(_Parser);
        }

        ArrayLiteral -> Array.Elements = realloc(ArrayLiteral -> Array.Elements, sizeof(ASTExpression *) * (ArrayLiteral -> Array.Count + 1));
        ArrayLiteral -> Array.Elements[ArrayLiteral -> Array.Count++] = Element;

        if (ParserMatch(_Parser, TOKEN_COMMA))
            continue;

        if (ParserCheck(_Parser, TOKEN_RBRACKET))
            break;

        ParserError(_Parser, "expected ',' or ']' in array literal");
        ParserAdvance(_Parser);
    }

    return ArrayLiteral;
}

ASTExpression *ParsePostfix(Parser *_Parser) {
    ASTExpression *Expression = ParsePrimary(_Parser);

    while (1) {
        if (ParserMatch(_Parser, TOKEN_LBRACKET)) {
            ASTExpression *IndexExpr = ParseExpression(_Parser);
            ASTExpression *Index = calloc(1, sizeof(ASTExpression));

            if (!ParserMatch(_Parser, TOKEN_RBRACKET)) {
                ParserError(_Parser, "expected ']' after index expression");

                return Expression;
            }

            Index -> Kind = EXPR_INDEX;
            Index -> Index.Target = Expression;
            Index -> Index.Index = IndexExpr;

            Expression = Index;

            continue;
        }
        
        break;
    }

    return Expression;
}

ASTType *ParseType(Parser *_Parser) {
    ASTType *Type = calloc(1, sizeof(ASTType));

    if (ParserMatch(_Parser, TOKEN_ARRAY)) {
        ParserMatch(_Parser, TOKEN_LT);

        Type -> ElementType = ParseType(_Parser);

        ParserMatch(_Parser, TOKEN_GT);

        Type -> Kind = TYPE_ARRAY;

        return Type;
    }

    if (ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
        Type -> Name = ParserPrevious(_Parser) -> Start;

        return Type;
    }

    ParserError(_Parser, "invalid type");

    return Type;
}

Token *ParserPeek(Parser *_Parser) {
    return _Parser -> Current;
}

Token *ParserPrevious(Parser *_Parser) {
    return &_Parser -> Tokens -> Data[_Parser -> Tokens -> Cursor - 1];
}

Token *ParserAdvance(Parser *_Parser) {
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
