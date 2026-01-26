#ifndef TOKENS_H
#define TOKENS_H

#include <stdint.h>

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_FLOAT,
    TOKEN_INTEGER,
    TOKEN_STRING,
    TOKEN_CHARACTER,
    TOKEN_ARRAY,

    TOKEN_CHAR_LITERAL,
    TOKEN_STRING_LITERAL,

    TOKEN_TYPE,
    TOKEN_AS,

    TOKEN_MUTABLE,
    TOKEN_CONSTANT,
    TOKEN_METHOD,
    TOKEN_PROCEDURE,
    TOKEN_FUNCTION,
    TOKEN_BEGIN,
    TOKEN_IS,
    TOKEN_END,
    TOKEN_RETURN,
    TOKEN_WHILE,
    TOKEN_REPEAT,
    TOKEN_FOR,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_ELSEIF,
    TOKEN_THEN,
    TOKEN_UNTIL,
    TOKEN_DO,
    TOKEN_IN,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_TRUE,
    TOKEN_FALSE,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_SLASH_SLASH,
    TOKEN_PERCENT,
    TOKEN_CARET,
    TOKEN_HASH,
    TOKEN_PIPE,
    TOKEN_QUESTION,
    TOKEN_DOT,
    TOKEN_DOT_DOT,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_LT,
    TOKEN_LE,
    TOKEN_GT,
    TOKEN_GE,
    TOKEN_ARROW,
    TOKEN_ASSIGN,
    
    TOKEN_BIT_AND,
    TOKEN_BIT_OR,
    TOKEN_BIT_XOR,
    TOKEN_BIT_NOT,
    TOKEN_SHIFT_LEFT,
    TOKEN_SHIFT_RIGHT,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOUBLE_COLON,

    TOKEN_NULL,
    TOKEN_VOID,
    TOKEN_NEWLINE,

    TOKEN_EOF,
    TOKEN_ERROR,
} TokenType;

typedef struct {
    TokenType Type;

    const char *Start;
    uint32_t Length;

    uint32_t Line;
    uint32_t Column;

    union {
        double Number;
        uint64_t Int;

        char *String;
    } Literal;
} Token;

typedef struct {
    Token *Data;

    size_t Count;
    size_t Capacity;

    size_t Cursor;
} TokenStream;

typedef struct {
    const char *Message;

    uint32_t Line;
    uint32_t Column;
} LexerError;

typedef struct {
    const char *Source;

    size_t Length;
    size_t Cursor;

    uint32_t Line;
    uint32_t Column;

    LexerError Error;
    int HasError;
} Lexer;

#endif
