#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"

static TraceEntry TraceStack[256];
static int TraceDepth = 0;

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

ASTProgram *MergePrograms(ASTProgram *Prelude, ASTProgram *User) {
    ASTProgram *Merged = calloc(1, sizeof(ASTProgram));

    size_t idx = 0;

    Merged -> SubprogramCount = Prelude -> SubprogramCount + User -> SubprogramCount;
    Merged -> Subprograms = calloc(Merged -> SubprogramCount, sizeof(ASTSubprogram *));

    for (size_t i = 0; i < Prelude -> SubprogramCount; i++)
        Merged -> Subprograms[idx++] = Prelude -> Subprograms[i];

    for (size_t i = 0; i < User -> SubprogramCount; i++)
        Merged -> Subprograms[idx++] = User -> Subprograms[i];

    Merged -> StatementCount = Prelude -> StatementCount + User -> StatementCount;
    Merged -> Statements = calloc(Merged -> StatementCount, sizeof(ASTStatement *));

    idx = 0;

    for (size_t i = 0; i < Prelude -> StatementCount; i++)
        Merged -> Statements[idx++] = Prelude -> Statements[i];

    for (size_t i = 0; i < User -> StatementCount; i++)
        Merged -> Statements[idx++] = User -> Statements[i];

    return Merged;
}

ASTProgram *ParseProgram(Parser *_Parser) {
    ASTProgram *Program = calloc(1, sizeof(ASTProgram));

    TraceEnter("ParseProgram", _Parser);

    while (!ParserCheck(_Parser, TOKEN_EOF)) {
        if (ParserCheck(_Parser, TOKEN_METHOD) || ParserCheck(_Parser, TOKEN_PROCEDURE) || ParserCheck(_Parser, TOKEN_FUNCTION)) {
            Program -> Subprograms = realloc(Program -> Subprograms, sizeof(ASTSubprogram*) * (Program -> SubprogramCount + 1));
            Program -> Subprograms[Program -> SubprogramCount++] = ParseSubprogram(_Parser);
        } else {
            ASTStatement *Statement = ParseStatement(_Parser);
            if (Statement) {
                Program -> Statements = realloc(Program -> Statements, sizeof(ASTStatement*) * (Program -> StatementCount + 1));
                Program -> Statements[Program -> StatementCount++] = Statement;
            }
        }
    }

    TraceExit("ParseProgram", _Parser);

    return Program;
}

ASTSubprogram *ParseSubprogram(Parser *_Parser) {
    ASTSubprogram *Function = calloc(1, sizeof(ASTSubprogram));

    TraceEnter("ParseSubprogram", _Parser);

    Function -> Kind = ParserAdvance(_Parser) -> Type;

    TraceToken("Advanced", _Parser);

    if (!ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
        ParserError(_Parser, "expected subprogram name");
        TraceExit("ParseSubprogram", _Parser);

        return Function;
    }

    TraceToken("Matched IDENTIFIER", _Parser);

    Function -> Name = ParserPrevious(_Parser) -> Start;

    if (ParserMatch(_Parser, TOKEN_LPAREN)) {
        TraceToken("Matched LPAREN", _Parser);

        while (!ParserCheck(_Parser, TOKEN_RPAREN) && !ParserCheck(_Parser, TOKEN_EOF)) {
            if (!ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
                ParserError(_Parser, "expected parameter name");

                break;
            }

            TraceToken("Matched param IDENTIFIER", _Parser);

            const char *ParamName = ParserPrevious(_Parser) -> Start;

            if (!ParserMatch(_Parser, TOKEN_COLON)) {
                ParserError(_Parser, "expected ':' after parameter name");

                break;
            }

            TraceToken("Matched COLON", _Parser);

            ASTType *ParamType = ParseType(_Parser);

            Function -> Params = realloc(Function -> Params, sizeof(*Function -> Params) * (Function -> ParamCount + 1));
            
            Function -> Params[Function -> ParamCount].Name = ParamName;
            Function -> Params[Function -> ParamCount].Type = ParamType;
            Function -> ParamCount++;

            if (!ParserMatch(_Parser, TOKEN_COMMA))
                break;

            TraceToken("Matched COMMA", _Parser);
        }

        if (!ParserMatch(_Parser, TOKEN_RPAREN)) {
            ParserError(_Parser, "expected ')' after parameters");
        }
    }

    if (ParserMatch(_Parser, TOKEN_ARROW)) {
        TraceToken("Matched ARROW", _Parser);

        Function -> ReturnType = ParseType(_Parser);
    }

    if (!ParserMatch(_Parser, TOKEN_IS)) {
        ParserError(_Parser, "expected 'is' before subprogram body");
    }

    TraceToken("Matched IS", _Parser);

    ASTStatement *Block = ParseBlock(_Parser);

    Function -> Body = Block -> Block.Statements;
    Function -> BodyCount = Block -> Block.Count;

    free(Block);

    TraceExit("ParseSubprogram", _Parser);

    return Function;
}

ASTStatement *ParseStatement(Parser *_Parser) {
    TraceEnter("ParseStatement", _Parser);
    
    if (ParserCheck(_Parser, TOKEN_IDENTIFIER) && ParserCheckNext(_Parser, TOKEN_ASSIGN)) {
        ASTStatement *Statement = calloc(1, sizeof(ASTStatement));
        ASTExpression *Target = ParsePrimary(_Parser);

        Statement -> Kind = STMT_ASSIGN;

        TraceToken("Detected assignment", _Parser);

        if (Target -> Kind != EXPR_IDENTIFIER && Target -> Kind != EXPR_INDEX) {
            ParserError(_Parser, "invalid assignment target");
            TraceExit("ParseStatement", _Parser);

            return NULL;
        }

        if (!ParserMatch(_Parser, TOKEN_ASSIGN)) {
            ParserError(_Parser, "expected ':=' after target");
            TraceExit("ParseStatement", _Parser);
            
            return NULL;
        }
        
        ASTExpression *Value = ParseExpression(_Parser);

        Statement -> Assign.Target = Target;
        Statement -> Assign.Value  = Value;

        TraceExit("ParseStatement", _Parser);

        return Statement;
    }

    if (ParserMatch(_Parser, TOKEN_MUTABLE) || ParserMatch(_Parser, TOKEN_CONSTANT)) {
        TraceToken("Matched MUTABLE/CONSTANT", _Parser);
        TraceExit("ParseStatement", _Parser);

        return ParseVariableDeclaration(_Parser, 1);
    }

    if (ParserCheck(_Parser, TOKEN_IDENTIFIER) && ParserCheckNext(_Parser, TOKEN_COLON)) {
        TraceToken("Detected variable declaration", _Parser);
        TraceExit("ParseStatement", _Parser);
        
        return ParseVariableDeclaration(_Parser, 0);
    }

    if (ParserCheck(_Parser, TOKEN_METHOD) || ParserCheck(_Parser, TOKEN_PROCEDURE) || ParserCheck(_Parser, TOKEN_FUNCTION)) {
        ParserError(_Parser, "subprograms are only allowed at block level");
        TraceExit("ParseStatement", _Parser);

        return NULL;
    }

    if (ParserMatch(_Parser, TOKEN_IF)) {
        TraceToken("Matched IF", _Parser);
        TraceExit("ParseStatement", _Parser);

        return ParseIfStatement(_Parser);
    }

    if (ParserMatch(_Parser, TOKEN_WHILE)) {
        TraceToken("Matched WHILE", _Parser);
        TraceExit("ParseStatement", _Parser);

        return ParseWhileStatement(_Parser);
    }

    if (ParserMatch(_Parser, TOKEN_REPEAT)) {
        TraceToken("Matched REPEAT", _Parser);
        TraceExit("ParseStatement", _Parser);

        return ParseRepeatStatement(_Parser);
    }

    if (ParserMatch(_Parser, TOKEN_FOR)) {
        TraceToken("Matched FOR", _Parser);
        TraceExit("ParseStatement", _Parser);
    
        return ParseForStatement(_Parser);
    }

    if (ParserMatch(_Parser, TOKEN_RETURN)) {
        TraceToken("Matched RETURN", _Parser);
        TraceExit("ParseStatement", _Parser);

        return ParseReturnStatement(_Parser);
    }

    TraceToken("Falling through to expression statement", _Parser);

    ASTStatement *Statement = calloc(1, sizeof(ASTStatement));

    Statement -> Kind = STMT_CALL;
    Statement -> ExpressionStmt.Expression = ParseExpression(_Parser);

    if (_Parser -> HasError) {
        TraceExit("ParseStatement", _Parser);

        free(Statement);

        return NULL;
    }

    TraceExit("ParseStatement", _Parser);

    return Statement;
}

ASTStatement *ParseBlock(Parser *_Parser) {
    ASTStatement *Block = calloc(1, sizeof(ASTStatement));

    Block -> Kind = STMT_BLOCK;

    TraceEnter("ParseBlock", _Parser);

    if (!ParserMatch(_Parser, TOKEN_BEGIN)) {
        ParserError(_Parser, "expected 'begin' to start block");
        TraceToken("FAILED to match BEGIN", _Parser);

        return Block;
    } else {
        TraceToken("Matched BEGIN", _Parser);
    }
    
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
    TraceToken("Matched END", _Parser);

    ParserMatch(_Parser, TOKEN_IDENTIFIER);
    TraceToken("Matched end identifier", _Parser);

    TraceExit("ParseBlock", _Parser);

    return Block;
}

ASTStatement *ParseVariableDeclaration(Parser *_Parser, int Mutability) {
    ASTStatement *Variable = calloc(1, sizeof(ASTStatement));

    Variable -> Kind = STMT_VAR_DECL;

    if (Mutability) {
        Token *Previous = ParserPrevious(_Parser);

        Variable -> VarDecl.Mutable = (Previous -> Type == TOKEN_MUTABLE);
    } else {
        Variable -> VarDecl.Mutable = 0;
    }

    if (!ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
        ParserError(_Parser, "expected variable name");

        return Variable;
    }

    Variable -> VarDecl.Name = ParserPrevious(_Parser) -> Start;

    if (!ParserMatch(_Parser, TOKEN_COLON)) {
        ParserError(_Parser, "expected ':' after variable name");
        
        return Variable;
    }

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
    TraceEnter("ParseExpression", _Parser);
    TraceExit("ParseExpression", _Parser);

    return ParseBinary(_Parser, 0);
}

ASTExpression *ParseBinary(Parser *_Parser, int Precedence) {
    ASTExpression *Left = ParseUnary(_Parser);

    TraceEnter("ParseBinary", _Parser);

    while (1) {
        Token *OperatorToken = ParserPeek(_Parser);

        int OperatorPrecedence = GetOperatorPrecedence(OperatorToken -> Type);
        if (OperatorPrecedence <= 0 || OperatorPrecedence < Precedence)
            break;

        ParserAdvance(_Parser);
        TraceToken("Binary operator", _Parser);

        ASTExpression *Right = ParseBinary(_Parser, OperatorPrecedence + 1);
        ASTExpression *BinaryExpression = calloc(1, sizeof(ASTExpression));

        BinaryExpression -> Kind = EXPR_BINARY;
        BinaryExpression -> Binary.Left = Left;
        BinaryExpression -> Binary.Right = Right;
        BinaryExpression -> Binary.Op = TokenToOperator(OperatorToken -> Type);

        Left = BinaryExpression;
    }

    TraceExit("ParseBinary", _Parser);

    return Left;
}

ASTExpression *ParseUnary(Parser *_Parser) {
    TraceEnter("ParseUnary", _Parser);
    
    if (ParserMatch(_Parser, TOKEN_MINUS) || ParserMatch(_Parser, TOKEN_NOT)) {
        ASTExpression *Expression = calloc(1, sizeof(ASTExpression));

        Expression -> Kind = EXPR_UNARY;
        Expression -> Unary.Op = ParserPrevious(_Parser) -> Type;
        Expression -> Unary.Operand = ParseUnary(_Parser);
        
        TraceToken("Matched unary operator", _Parser);
        TraceExit("ParseUnary", _Parser);

        return Expression;
    }

    TraceExit("ParseUnary", _Parser);

    return ParsePostfix(_Parser);
}

ASTExpression *ParsePrimary(Parser *_Parser) {
    ASTExpression *Expression = calloc(1, sizeof(ASTExpression));

    TraceEnter("ParsePrimary", _Parser);

    if (ParserMatch(_Parser, TOKEN_NUMBER)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Number = ParserPrevious(_Parser) -> Literal.Number;

        TraceToken("Matched NUMBER", _Parser);
        TraceExit("ParsePrimary", _Parser);

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_INTEGER)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Int = ParserPrevious(_Parser) -> Literal.Int;

        TraceToken("Matched INTEGER", _Parser);
        TraceExit("ParsePrimary", _Parser);

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_STRING_LITERAL)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.String = ParserPrevious(_Parser) -> Literal.String;

        TraceToken("Matched STRING_LITERAL", _Parser);
        TraceExit("ParsePrimary", _Parser);

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_CHAR_LITERAL)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Char = (char) ParserPrevious(_Parser) -> Literal.Int;

        TraceToken("Matched CHAR_LITERAL", _Parser);
        TraceExit("ParsePrimary", _Parser);

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_TRUE)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Bool = 1;

        TraceToken("Matched TRUE", _Parser);
        TraceExit("ParsePrimary", _Parser);

        return Expression;
    } else if (ParserMatch(_Parser, TOKEN_FALSE)) {
        Expression -> Kind = EXPR_LITERAL;
        Expression -> Literal.Bool = 0;

        TraceToken("Matched FALSE", _Parser);
        TraceExit("ParsePrimary", _Parser);

        return Expression;
    }

    if (ParserMatch(_Parser, TOKEN_IDENTIFIER)) {
        Expression -> Kind = EXPR_IDENTIFIER;
        Expression -> Identifier = ParserPrevious(_Parser) -> Start;

        TraceToken("Matched IDENTIFIER", _Parser);
        TraceExit("ParsePrimary", _Parser);

        return Expression;
    }

    if (ParserMatch(_Parser, TOKEN_LPAREN)) {
        TraceToken("Matched LPAREN", _Parser);

        free(Expression);

        Expression = ParseExpression(_Parser);

        ParserMatch(_Parser, TOKEN_RPAREN);

        TraceToken("Matched RPAREN", _Parser);
        TraceExit("ParsePrimary", _Parser);

        return Expression;
    }

    if (ParserCheck(_Parser, TOKEN_LBRACKET)) {
        TraceToken("Detected LBRACKET", _Parser);

        free(Expression);

        TraceExit("ParsePrimary", _Parser);

        return ParseArrayLiteral(_Parser);
    }

    TraceToken("ERROR - No valid primary expression", _Parser);
    ParserError(_Parser, "invalid expression");

    Expression -> Kind = EXPR_LITERAL;

    TraceExit("ParsePrimary", _Parser);

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

        ParserMatch(_Parser, TOKEN_COMMA);
    }

    if (!ParserMatch(_Parser, TOKEN_RBRACKET)) {
        ParserError(_Parser, "expected ']' at end of array literal");
    }

    return ArrayLiteral;
}

ASTExpression *ParsePostfix(Parser *_Parser) {
    ASTExpression *Expression = ParsePrimary(_Parser);

    TraceEnter("ParsePostfix", _Parser);

    while (1) {
        if (ParserMatch(_Parser, TOKEN_LPAREN)) {
            ASTExpression *Call = calloc(1, sizeof(ASTExpression));

            Call -> Kind = EXPR_CALL;
            Call -> Call.Callee = Expression;
            Call -> Call.Args = NULL;
            Call -> Call.ArgCount = 0;

            TraceToken("Matched LPAREN for call", _Parser);

            if (!ParserCheck(_Parser, TOKEN_RPAREN)) {
                do {
                    ASTExpression *Arguments = ParseExpression(_Parser);

                    Call -> Call.Args = realloc(Call -> Call.Args, sizeof(ASTExpression*) * (Call -> Call.ArgCount + 1));
                    Call -> Call.Args[Call -> Call.ArgCount++] = Arguments;
                } while (ParserMatch(_Parser, TOKEN_COMMA));
            }
    
            ParserMatch(_Parser, TOKEN_RPAREN);
            TraceToken("Matched RPAREN for call", _Parser);

            Expression = Call;
        }

        if (ParserMatch(_Parser, TOKEN_LBRACKET)) {
            ASTExpression *Index = calloc(1, sizeof(ASTExpression));
            ASTExpression *IndexExpr = ParseExpression(_Parser);

            TraceToken("Matched LBRACKET for index", _Parser);

            if (!ParserMatch(_Parser, TOKEN_RBRACKET)) {
                ParserError(_Parser, "expected ']' after index expression");
                TraceExit("ParsePostfix", _Parser);

                return Expression;
            }

            Index -> Kind = EXPR_INDEX;
            Index -> Index.Target = Expression;
            Index -> Index.Index = IndexExpr;

            Expression = Index;

            TraceToken("Matched RBRACKET for index", _Parser);

            continue;
        }
        
        break;
    }

    TraceExit("ParsePostfix", _Parser);

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

    if (Line == PreviousLine && Column == PreviousColumn || ParserCheck(_Parser, TOKEN_EOF)) {
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

static void TraceEnter(const char *FunctionName, Parser *_Parser) {
    #if PARSER_TRACE
        TraceStack[TraceDepth].FunctionName = FunctionName;
        TraceStack[TraceDepth].Depth = TraceDepth;
        
        for (int i = 0; i < TraceDepth; i++)
            printf("  ");

        printf(">> %s (current: %s at %d:%d)\n", FunctionName, TokenTypeToString(_Parser -> Current -> Type), _Parser -> Current -> Line, _Parser -> Current -> Column);
        
        TraceDepth++;
    #endif
}

static void TraceExit(const char *FunctionName, Parser *_Parser) {
    #if PARSER_TRACE
        TraceDepth--;
        
        for (int i = 0; i < TraceDepth; i++)
            printf("  ");

        printf("<< %s (current: %s at %d:%d)\n", FunctionName, TokenTypeToString(_Parser -> Current -> Type), _Parser -> Current -> Line, _Parser -> Current -> Column);
    #endif
}

static void TraceToken(const char *Action, Parser *_Parser) {
    #if PARSER_TRACE
        for (int i = 0; i < TraceDepth; i++)
            printf("  ");

        printf("  [%s: %s at %d:%d]\n", Action, TokenTypeToString(_Parser -> Current -> Type), _Parser -> Current -> Line, _Parser -> Current -> Column);
    #endif
}
