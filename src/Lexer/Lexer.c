#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Tokens.h"
#include "Lexer.h"

int IsDigit(int Character) {
    if (Character >= '0' && Character <= '9')
        return 1;
    else
        return 0;
}

int IsAlphanumericNumber(int Character) {
    if ((Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || (Character >= '0' && Character <= '9'))
        return 1;
    else
        return 0;
}

int IsAlphanumeric(int Character) {
    if ((Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z'))
        return 1;
    else
        return 0;
}

const char *TokenTypeToString(TokenType Type) {
    switch (Type) {
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_BOOLEAN: return "BOOLEAN";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_ARRAY: return "ARRAY";
        case TOKEN_CHAR_LITERAL: return "CHAR_LITERAL";
        case TOKEN_STRING_LITERAL :return "STRING_LITERAL";
        case TOKEN_NULL: return "NULL";
        case TOKEN_VOID: return "VOID";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";

        case TOKEN_TYPE: return "TYPE";
        case TOKEN_AS: return "AS";
        case TOKEN_MUTABLE: return "MUTABLE";
        case TOKEN_CONSTANT: return "CONSTANT";

        case TOKEN_METHOD: return "METHOD";
        case TOKEN_PROCEDURE: return "PROCEDURE";
        case TOKEN_FUNCTION: return "FUNCTION";
        case TOKEN_BEGIN: return "BEGIN";
        case TOKEN_END: return "END";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_IS: return "IS";
        case TOKEN_NEWLINE: return "NEWLINE";

        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_ELSEIF: return "ELSEIF";
        case TOKEN_THEN: return "THEN";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_REPEAT: return "REPEAT";
        case TOKEN_UNTIL: return "UNTIL";
        case TOKEN_FOR: return "FOR";
        case TOKEN_DO: return "DO";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";

        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_NOT: return "NOT";

        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_SLASH_SLASH: return "INT_DIV";
        case TOKEN_PERCENT: return "PERCENT";
        case TOKEN_CARET: return "CARET";

        case TOKEN_EQ: return "EQ";
        case TOKEN_NE: return "NE";
        case TOKEN_LT: return "LT";
        case TOKEN_LE: return "LE";
        case TOKEN_GT: return "GT";
        case TOKEN_GE: return "GE";

        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_QUESTION: return "QUESTION";

        case TOKEN_BIT_AND: return "BIT_AND";
        case TOKEN_BIT_OR: return "BIT_OR";
        case TOKEN_BIT_XOR: return "BIT_XOR";
        case TOKEN_BIT_NOT: return "BIT_NOT";
        case TOKEN_SHIFT_LEFT: return "SHIFT_LEFT";
        case TOKEN_SHIFT_RIGHT: return "SHIFT_RIGHT";

        case TOKEN_HASH: return "HASH";
        case TOKEN_PIPE: return "PIPE";
        case TOKEN_DOT: return "DOT";
        case TOKEN_DOT_DOT: return "RANGE";

        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_DOUBLE_COLON: return "DOUBLE_COLON";

        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";

        default: return "UNKNOWN";
    }
}

int LexerIsAtEnd(Lexer *_Lexer) {
    return _Lexer -> Cursor >= _Lexer -> Length;
}

void LexerSkipWC(Lexer *_Lexer) {
    while (!LexerIsAtEnd(_Lexer)) {
        char Character = LexerPeek(_Lexer);

        if (Character == ' ' || Character == '\t' || Character == '\r' || Character == '\n') {
            LexerNext(_Lexer);
        } else if (Character == '-' && _Lexer -> Cursor + 1 < _Lexer -> Length && _Lexer -> Source[_Lexer -> Cursor + 1] == '-') {
            LexerNext(_Lexer);
            LexerNext(_Lexer);

            while (!LexerIsAtEnd(_Lexer) && LexerPeek(_Lexer) != '\n')
                LexerNext(_Lexer);
        } else break;
    }
}

TokenType ResolveIdentifier(const char* Start, size_t Len) {
    if (Len == 7 && memcmp(Start, "MUTABLE", 7) == 0) return TOKEN_MUTABLE;
    if (Len == 5 && memcmp(Start, "CONST", 5) == 0) return TOKEN_CONSTANT;
    if (Len == 5 && memcmp(Start, "Array", 5) == 0) return TOKEN_ARRAY;
    if (Len == 7 && memcmp(Start, "Boolean", 7) == 0) return TOKEN_BOOLEAN;
    if (Len == 4 && memcmp(Start, "true", 4) == 0) return TOKEN_TRUE;
    if (Len == 5 && memcmp(Start, "false", 5) == 0) return TOKEN_FALSE;
    if (Len == 2 && memcmp(Start, "if", 2) == 0) return TOKEN_IF;
    if (Len == 2 && memcmp(Start, "in", 2) == 0) return TOKEN_IN;
    if (Len == 3 && memcmp(Start, "and", 3) == 0) return TOKEN_AND;
    if (Len == 2 && memcmp(Start, "or", 2) == 0) return TOKEN_OR;
    if (Len == 3 && memcmp(Start, "not", 3) == 0) return TOKEN_NOT;
    if (Len == 4 && memcmp(Start, "then", 4) == 0) return TOKEN_THEN;
    if (Len == 4 && memcmp(Start, "else", 4) == 0) return TOKEN_ELSE;
    if (Len == 6 && memcmp(Start, "elseif", 6) == 0) return TOKEN_ELSEIF;
    if (Len == 3 && memcmp(Start, "for", 3) == 0) return TOKEN_FOR;
    if (Len == 2 && memcmp(Start, "do", 2) == 0) return TOKEN_DO;
    if (Len == 6 && memcmp(Start, "method", 6) == 0) return TOKEN_METHOD;
    if (Len == 9 && memcmp(Start, "procedure", 9) == 0) return TOKEN_PROCEDURE;
    if (Len == 8 && memcmp(Start, "function", 8) == 0) return TOKEN_FUNCTION;
    if (Len == 5 && memcmp(Start, "begin", 5) == 0) return TOKEN_BEGIN;
    if (Len == 3 && memcmp(Start, "end", 3) == 0) return TOKEN_END;
    if (Len == 2 && memcmp(Start, "is", 2) == 0) return TOKEN_IS;
    if (Len == 6 && memcmp(Start, "return", 6) == 0) return TOKEN_RETURN;
    if (Len == 5 && memcmp(Start, "while", 5) == 0) return TOKEN_WHILE;
    if (Len == 6 && memcmp(Start, "repeat", 6) == 0) return TOKEN_REPEAT;
    if (Len == 5 && memcmp(Start, "until", 5) == 0) return TOKEN_UNTIL;
    if (Len == 5 && memcmp(Start, "break", 5) == 0) return TOKEN_BREAK;
    if (Len == 8 && memcmp(Start, "continue", 8) == 0) return TOKEN_CONTINUE;
    
    return TOKEN_IDENTIFIER;
}

Lexer LexerCreate(const char *Source) {
    Lexer _Lexer = {0};

    _Lexer.Source = Source;
    _Lexer.Length = strlen(Source);
    _Lexer.Cursor = 0;
    _Lexer.Line = 1;
    _Lexer.Column = 1;

    return _Lexer;
}

char LexerPeek(Lexer *_Lexer) {
    if (_Lexer -> Cursor >= _Lexer -> Length)
        return '\0';

    return _Lexer -> Source[_Lexer -> Cursor];
}

char LexerNext(Lexer *_Lexer) {
    char Character = LexerPeek(_Lexer);

    if (Character == '\n') {
        _Lexer -> Line++;
        _Lexer -> Column = 1;
    } else {
        _Lexer -> Column++;
    }

    if (_Lexer -> Cursor < _Lexer -> Length)
        _Lexer -> Cursor++;

    return Character;
}

static void LexerErrorAt(Lexer *_Lexer, const char *Message) {
    if (_Lexer -> HasError) return;

    _Lexer -> Error.Line = _Lexer -> Line;
    _Lexer -> Error.Column = _Lexer -> Column;

    _Lexer -> Error.Message = Message;
    _Lexer -> HasError = 1;
}

static void LexingError(Lexer *_Lexer) {
    fprintf(stderr, "[Lexer Error] Line %u:%u >> %s\n", _Lexer -> Error.Line, _Lexer -> Error.Column, _Lexer -> Error.Message);
}

Token LexerNextToken(Lexer *_Lexer) {
    LexerSkipWC(_Lexer);

    Token _Token = {0};

    _Token.Line = _Lexer -> Line;
    _Token.Column = _Lexer -> Column;

    char Character = LexerNext(_Lexer);

    if (_Lexer -> HasError) {
        Token NewToken = {0};
        NewToken.Type = TOKEN_EOF;

        return NewToken;
    }

    if (Character == '\0') {
        _Token.Type = TOKEN_EOF;
    } else if (IsAlphanumeric(Character) || Character == '_') {
        size_t Start = _Lexer -> Cursor - 1;

        while (IsAlphanumericNumber(LexerPeek(_Lexer)) || LexerPeek(_Lexer) == '_')
            LexerNext(_Lexer);

        _Token.Start = _Lexer -> Source + Start;
        _Token.Length = _Lexer -> Cursor - Start;

        _Token.Type = ResolveIdentifier(_Token.Start, _Token.Length);
    } else if (IsDigit(Character) || (Character == '.' && IsDigit(LexerPeek(_Lexer)))) {
        size_t Start = _Lexer -> Cursor - 1;

        while (IsDigit(LexerPeek(_Lexer)))
            LexerNext(_Lexer);

        if (LexerPeek(_Lexer) == '.') {
            LexerNext(_Lexer);

            while (IsDigit(LexerPeek(_Lexer)))
                LexerNext(_Lexer);
        }

        _Token.Start = _Lexer -> Source + Start;
        _Token.Length = _Lexer -> Cursor - Start;

        _Token.Type = TOKEN_NUMBER;

        _Token.Literal.Number = strtod(_Token.Start, NULL);
    } else if (Character == '"') {
        size_t Start = _Lexer -> Cursor;

        while (!LexerIsAtEnd(_Lexer)) {
            char NextCharacter = LexerNext(_Lexer);

            if (NextCharacter == '"')
                break;

            if (NextCharacter == '\n') {
                LexerErrorAt(_Lexer, "newline in string literal");
                LexingError(_Lexer);

                break;
            }

            if (NextCharacter == '\\')
                LexerNext(_Lexer);
        }

        if (LexerIsAtEnd(_Lexer)) {
            LexerErrorAt(_Lexer, "unterminated string literal");
            LexingError(_Lexer);
        }

        _Token.Start = _Lexer -> Source + Start;
        _Token.Length = _Lexer -> Cursor - Start - 1;

        _Token.Type = TOKEN_STRING_LITERAL;

        char *Copy = malloc(_Token.Length + 1);

        memcpy(Copy, _Token.Start, _Token.Length);

        Copy[_Token.Length] = '\0';

        _Token.Literal.String = Copy;
    } else if (Character == '\'') {
        char Value = LexerNext(_Lexer);

        if (LexerNext(_Lexer) != '\'') {
            LexerErrorAt(_Lexer, "invalid character literal");
            LexingError(_Lexer);
        }

        _Token.Type = TOKEN_CHAR_LITERAL;
        _Token.Literal.Int = (uint64_t) Value;
    } else {
        switch (Character) {
            case '+': _Token.Type = TOKEN_PLUS; break;
            case '-':
                if (LexerPeek(_Lexer) == '>') {
                    LexerNext(_Lexer);

                    _Token.Type = TOKEN_ARROW;
                } else {
                    _Token.Type = TOKEN_MINUS;
                }
                
                break;

            case '*': _Token.Type = TOKEN_STAR; break;
            case '/': 
                if (LexerPeek(_Lexer) == '/') {
                    LexerNext(_Lexer);

                    _Token.Type = TOKEN_SLASH_SLASH;
                } else {
                    _Token.Type = TOKEN_SLASH;
                }

                break;

            case '%': _Token.Type = TOKEN_PERCENT; break;
            case '^':
                if (LexerPeek(_Lexer) == '=') {
                    LexerNext(_Lexer);

                    _Token.Type = TOKEN_BIT_XOR;
                } else {
                    _Token.Type = TOKEN_CARET;
                }
            
                break;

            case '#': _Token.Type = TOKEN_HASH; break;
            case '|':
                if (LexerPeek(_Lexer) == '=') {
                    LexerNext(_Lexer);

                    _Token.Type = TOKEN_BIT_OR;
                } else {
                    _Token.Type = TOKEN_PIPE;
                }
                
                break;
                
            case '?': _Token.Type = TOKEN_QUESTION; break;
            case '=':
                if (LexerPeek(_Lexer) == '=') {
                    LexerNext(_Lexer);

                    _Token.Type = TOKEN_EQ;
                } else {
                    _Token.Type = TOKEN_ASSIGN;
                }

                break;

            case '~': _Token.Type = TOKEN_BIT_NOT; break;
            case '!':
                if (LexerPeek(_Lexer) == '=') {
                    LexerNext(_Lexer);
                    
                    _Token.Type = TOKEN_NE;
                } else if (LexerPeek(_Lexer) != '=') {
                    LexerErrorAt(_Lexer, "standalone '!', expected '!='");
                }

                break;

            case '<':
                if (LexerPeek(_Lexer) == '<') {
                    LexerNext(_Lexer);
                    
                    _Token.Type = TOKEN_SHIFT_LEFT;
                } else if (LexerPeek(_Lexer) == '=') {
                    LexerNext(_Lexer);
                    
                    _Token.Type = TOKEN_LE;
                } else {
                    _Token.Type = TOKEN_LT;
                }

                break;

            case '>':
                if (LexerPeek(_Lexer) == '=') {
                    LexerNext(_Lexer);
                    
                    _Token.Type = TOKEN_GE;
                } else {
                    _Token.Type = TOKEN_GT;
                }

                break;

            case '&': _Token.Type = TOKEN_BIT_AND; break;
            case '.':
                if (LexerPeek(_Lexer) == '.') {
                    LexerNext(_Lexer);

                    _Token.Type = TOKEN_DOT_DOT;
                } else {
                    _Token.Type = TOKEN_DOT;
                }

                break;

            case '(': _Token.Type = TOKEN_LPAREN; break;
            case ')': _Token.Type = TOKEN_RPAREN; break;
            case '{': _Token.Type = TOKEN_LBRACE; break;
            case '}': _Token.Type = TOKEN_RBRACE; break;
            case '[': _Token.Type = TOKEN_LBRACKET; break;
            case ']': _Token.Type = TOKEN_RBRACKET; break;
            case ',': _Token.Type = TOKEN_COMMA; break;
            case ';': _Token.Type = TOKEN_SEMICOLON; break;
            case ':':
                if (LexerPeek(_Lexer) == '=') {
                    LexerNext(_Lexer);

                    _Token.Type = TOKEN_ASSIGN;
                } else if (LexerPeek(_Lexer) == ':') {
                    LexerNext(_Lexer);
                    
                    _Token.Type = TOKEN_DOUBLE_COLON;
                } else {
                    _Token.Type = TOKEN_COLON;
                }

                break;

            default:
                _Token.Type = TOKEN_ERROR;

                LexerErrorAt(_Lexer, "unexpected character");
                LexingError(_Lexer);
        }
    }

    return _Token;
}

TokenStream Tokenize(Lexer *_Lexer) {
    TokenStream _TokenStream = {0};

    _TokenStream.Capacity = 64;
    _TokenStream.Data = malloc(sizeof(Token) * _TokenStream.Capacity);

    while (1) {
        Token _Token = LexerNextToken(_Lexer);

        if (_TokenStream.Count >= _TokenStream.Capacity) {
            _TokenStream.Capacity *= 2;
            _TokenStream.Data = realloc(_TokenStream.Data, sizeof(Token) * _TokenStream.Capacity);
        }

        // DEBUGGING USE | printf("[Lexer] Line %i:%i | %s | Symbol: %.*s\n", _Lexer -> Line, _Lexer -> Column, TokenTypeToString(_Token.Type), _Token.Length, _Token.Start ? _Token.Start : "");

        _TokenStream.Data[_TokenStream.Count++] = _Token;

        if (_Token.Type == TOKEN_EOF)
            break;
    }

    return _TokenStream;
}
