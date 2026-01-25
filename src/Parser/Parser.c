#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"

int IsSyncToken(TokenType Type) {
    switch (Type) {
        case TOKEN_END:
        case TOKEN_BEGIN:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_FOR:
        case TOKEN_REPEAT:
        case TOKEN_RETURN:
        case TOKEN_MUTABLE:
        case TOKEN_CONSTANT:
        case TOKEN_METHOD:
        case TOKEN_PROCEDURE:
        case TOKEN_FUNCTION:
        case TOKEN_EOF: return 1;

        default: return 0;
    }
}

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

ASTOperator TokenToOperator(TokenType Type) {
    switch(Type) {
        case TOKEN_PLUS: return OP_ADD;
        case TOKEN_MINUS: return OP_SUB;
        case TOKEN_STAR: return OP_MUL;
        case TOKEN_SLASH: return OP_DIV;
        case TOKEN_EQ: return OP_EQ;
        case TOKEN_NE: return OP_NE;
        case TOKEN_LT: return OP_LT;
        case TOKEN_LE: return OP_LE;
        case TOKEN_GT: return OP_GT;
        case TOKEN_GE: return OP_GE;
        case TOKEN_AND: return OP_AND;
        case TOKEN_OR: return OP_OR;
        case TOKEN_NOT: return OP_NOT;
        case TOKEN_SHIFT_LEFT: return OP_SHIFT_LEFT;
        case TOKEN_SHIFT_RIGHT: return OP_SHIFT_RIGHT;

        default: return OP_ADD;
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
            if (Statement) {
                Program -> Subprograms[Program -> SubprogramCount++] = ParseSubprogram(_Parser);
                Program -> Statements[Program -> StatementCount++] = Statement;
            }
        }
    }

    return Program;
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
        if (!ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
            ParserError(_Parser, "expected parameter name");

            break;
        }

        if (!ParserMatch(_Parser, TOKEN_COLON)) {
            ParserError(_Parser, "expected ':' after parameter name");

            break;
        }

        const char *ParamName = ParserPrevious(_Parser) -> Start;
        ASTType *ParamType = ParseType(_Parser);

        Function -> Params = realloc(Function -> Params, sizeof(*Function -> Params) * (Function -> ParamCount + 1));
        
        Function -> Params[Function -> ParamCount].Name = ParamName;
        Function -> Params[Function -> ParamCount].Type = ParamType;
        Function -> ParamCount++;

        ParserMatch(_Parser, TOKEN_COMMA);
    }

    ParserMatch(_Parser, TOKEN_RPAREN);

    if (ParserMatch(_Parser, TOKEN_ARROW)) {
        Function->ReturnType = ParseType(_Parser);
    }

    if (!ParserMatch(_Parser, TOKEN_IS)) {
        ParserError(_Parser, "expected 'is' before subprogram body");
    }

    ASTStatement *Block = ParseBlock(_Parser);

    Function -> Body = Block -> Block.Statements;
    Function -> BodyCount = Block -> Block.Count;

    free(Block);

    return Function;
}

ASTStatement *ParseStatement(Parser *_Parser) {
    if (ParserCheck(_Parser, TOKEN_IDENTIFIER) && ParserCheckNext(_Parser, TOKEN_ASSIGN)) {
        ASTStatement *Statement = calloc(1, sizeof(ASTStatement));
        ASTExpression *Target = ParsePrimary(_Parser);

        Statement -> Kind = STMT_ASSIGN;

        if (Target -> Kind != EXPR_IDENTIFIER && Target -> Kind != EXPR_INDEX) {
            ParserError(_Parser, "invalid assignment target");

            return NULL;
        }

        if (!ParserMatch(_Parser, TOKEN_ASSIGN)) {
            ParserError(_Parser, "expected ':=' after target");
            
            return NULL;
        }
        
        ASTExpression *Value = ParseExpression(_Parser);

        Statement -> Assign.Target = Target;
        Statement -> Assign.Value  = Value;

        return Statement;
    }

    if (ParserMatch(_Parser, TOKEN_MUTABLE) || ParserMatch(_Parser, TOKEN_CONSTANT)) {
        return ParseVariableDeclaration(_Parser);
    }

    if (ParserCheck(_Parser, TOKEN_IDENTIFIER) && ParserCheckNext(_Parser, TOKEN_COLON)) {
        return ParseVariableDeclaration(_Parser);
    }

    if (ParserCheck(_Parser, TOKEN_METHOD) || ParserCheck(_Parser, TOKEN_PROCEDURE) || ParserCheck(_Parser, TOKEN_FUNCTION)) {
        ParserError(_Parser, "subprograms are only allowed at block level");

        return NULL;
    }

    if (ParserMatch(_Parser, TOKEN_BEGIN)) return ParseBlock(_Parser);
    if (ParserMatch(_Parser, TOKEN_IF)) return ParseIfStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_WHILE)) return ParseWhileStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_REPEAT)) return ParseRepeatStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_FOR)) return ParseForStatement(_Parser);
    if (ParserMatch(_Parser, TOKEN_RETURN)) return ParseReturnStatement(_Parser);

    ASTStatement *Statement = calloc(1, sizeof(ASTStatement));

    Statement -> ExpressionStmt.Expression = ParseExpression(_Parser);

    if (_Parser -> HasError) {
        free(Statement);

        return NULL;
    }

    return Statement;
}

ASTStatement *ParseBlock(Parser *_Parser) {
    ASTStatement *Block = calloc(1, sizeof(ASTStatement));

    Block -> Kind = STMT_BLOCK;
    
    while (!ParserCheck(_Parser, TOKEN_END) && !ParserCheck(_Parser, TOKEN_EOF)) {
        if (ParserCheck(_Parser, TOKEN_METHOD) || ParserCheck(_Parser, TOKEN_PROCEDURE) || ParserCheck(_Parser, TOKEN_FUNCTION)) {
            ASTSubprogram *Subprogram = ParseSubprogram(_Parser);

            Block -> Block.Subprograms = realloc(Block -> Block.Subprograms, sizeof(ASTSubprogram*) * (Block -> Block.SubprogramCount + 1));
            Block -> Block.Subprograms[Block -> Block.SubprogramCount++] = Subprogram;

            continue;
        }

        ASTStatement *Statement = ParseStatement(_Parser);
        if (Statement) {
            Block -> Block.Statements = realloc(Block -> Block.Statements, sizeof(ASTStatement*) * (Block -> Block.Count + 1));
            Block -> Block.Statements[Block -> Block.Count++] = Statement;
        }
    }

    ParserMatch(_Parser, TOKEN_END);
    ParserMatch(_Parser, TOKEN_IDENTIFIER);

    return Block;
}

ASTStatement *ParseVariableDeclaration(Parser *_Parser) {
    ASTStatement *Variable = calloc(1, sizeof(ASTStatement));

    Variable -> Kind = STMT_VAR_DECL;
    Variable -> VarDecl.Mutable = 0;

    if (!ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
        ParserError(_Parser, "expected variable name");

        return Variable;
    }

    Variable -> VarDecl.Name = ParserPrevious(_Parser) -> Start;

    if (ParserMatch(_Parser, TOKEN_COLON))
        Variable -> VarDecl.Type = ParseType(_Parser);

    if (ParserMatch(_Parser, TOKEN_ASSIGN)) {
        if (ParserCheck(_Parser, TOKEN_LBRACKET)) { 
            Variable -> VarDecl.Initializer = ParseArrayLiteral(_Parser);
        } else {
            Variable -> VarDecl.Initializer = ParseExpression(_Parser);
        }
    }

    return Variable;
}

ASTStatement *ParseIfStatement(Parser *_Parser) {
    ASTStatement *If = calloc(1, sizeof(ASTStatement));

    If -> Kind = STMT_IF;
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

    While -> Kind = STMT_WHILE;
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

    Repeat -> Kind = STMT_REPEAT;

    Repeat -> Repeat.Body = Body -> Block.Statements;
    Repeat -> Repeat.Count = Body -> Block.Count;

    free(Body);

    ParserMatch(_Parser, TOKEN_UNTIL);

    Repeat -> Repeat.Until = ParseExpression(_Parser);

    return Repeat;
}

ASTStatement *ParseForStatement(Parser *_Parser) {
    ASTStatement *For = calloc(1, sizeof(ASTStatement));
    
    For -> Kind = STMT_FOR;

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

    Return -> Kind = STMT_RETURN;

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
        BinaryExpression -> Binary.Op = TokenToOperator(OperatorToken -> Type);

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
        free(Expression);

        Expression = ParseExpression(_Parser);

        ParserMatch(_Parser, TOKEN_RPAREN);

        return Expression;
    }

    if (ParserCheck(_Parser, TOKEN_LBRACKET)) {
        free(Expression);

        return ParseArrayLiteral(_Parser);
    }

    ParserError(_Parser, "invalid expression");

    return Expression;
}

ASTExpression *ParseArrayLiteral(Parser *_Parser) {
    if (!ParserMatch(_Parser, TOKEN_LBRACKET)) {
        ParserError(_Parser, "expected '[' to start array literal");

        return NULL;
    }

    ASTExpression *ArrayLiteral = calloc(1, sizeof(ASTExpression));

    ArrayLiteral -> Kind = EXPR_ARRAY_LITERAL;
    ArrayLiteral -> Array.Elements = NULL;
    ArrayLiteral -> Array.Count = 0;

    while (!ParserCheck(_Parser, TOKEN_RBRACKET) && !ParserCheck(_Parser, TOKEN_EOF)) {
        ASTExpression *Element = NULL;

        if (ParserCheck(_Parser, TOKEN_LBRACKET)) {
            Element = ParseArrayLiteral(_Parser);
        } else {
            Element = ParseExpression(_Parser);
        }

        ArrayLiteral -> Array.Elements = realloc(ArrayLiteral -> Array.Elements, sizeof(ASTExpression *) * (ArrayLiteral -> Array.Count + 1));
        ArrayLiteral -> Array.Elements[ArrayLiteral -> Array.Count++] = Element;

        //if (ParserMatch(_Parser, TOKEN_COMMA))
        //    continue;

        ParserMatch(_Parser, TOKEN_COMMA);
    }

    if (!ParserMatch(_Parser, TOKEN_RBRACKET)) {
        ParserError(_Parser, "expected ']' at end of array literal");
    }

    return ArrayLiteral;
}

ASTExpression *ParsePostfix(Parser *_Parser) {
    ASTExpression *Expression = ParsePrimary(_Parser);

    while (1) {
        if (ParserMatch(_Parser, TOKEN_LPAREN)) {
            ASTExpression *Call = calloc(1, sizeof(ASTExpression));

            Call -> Kind = EXPR_CALL;
            Call -> Call.Callee = Expression;
            Call -> Call.Args = NULL;
            Call -> Call.ArgCount = 0;

            if (!ParserCheck(_Parser, TOKEN_RPAREN)) {
                do {
                    ASTExpression *Arguments = ParseExpression(_Parser);

                    Call -> Call.Args = realloc(Call -> Call.Args, sizeof(ASTExpression*) * (Call -> Call.ArgCount + 1));
                    Call -> Call.Args[Call -> Call.ArgCount++] = Arguments;
                } while (ParserMatch(_Parser, TOKEN_COMMA));
            }
    
            ParserMatch(_Parser, TOKEN_RPAREN);

            Expression = Call;
        }

        if (ParserMatch(_Parser, TOKEN_LBRACKET)) {
            ASTExpression *Index = calloc(1, sizeof(ASTExpression));
            ASTExpression *IndexExpr = ParseExpression(_Parser);

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

int ParserCheckNext(Parser *_Parser, TokenType Type) {
    if (_Parser -> Current -> Type == TOKEN_EOF)
        return 0;

    return _Parser -> Tokens -> Data[_Parser -> Tokens -> Cursor + 1].Type == Type;
}

int ParserMatch(Parser *_Parser, TokenType Type) {
    if (ParserCheck(_Parser, Type)) {
        ParserAdvance(_Parser);

        return 1;
    } 

    return 0;
}

void ParserError(Parser *_Parser, const char *Message) {
    static int PreviousLine;
    static int PreviousColumn;

    int Line = _Parser -> Current -> Line;
    int Column  = _Parser -> Current -> Column;

    if (Line == PreviousLine && Column == PreviousColumn) {
        exit(0);
    }

    PreviousLine = Line;
    PreviousColumn = Column;

    fprintf(stderr, "[Parse Error] Line %u:%u >> %s\n", _Parser -> Current -> Line, _Parser -> Current -> Column, Message);

    _Parser -> HasError = 1;

    while (!IsSyncToken(_Parser -> Current -> Type)) {
        ParserAdvance(_Parser);
    }
}
