#ifndef AST_H
#define AST_H

#include <stdint.h>

typedef enum {
    AST_PROGRAM,
    AST_SUBPROGRAM,
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_REPEAT,
    AST_FOR,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_CALL,
    AST_EXPR_LITERAL,
    AST_EXPR_IDENTIFIER,
    AST_EXPR_BINARY,
    AST_EXPR_UNARY,
    ASTType_NAMED,
    ASTType_ARRAY
} ASTNodeType;

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_ARRAY
} ASTTypeKind;

typedef enum {
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_LITERAL,
    EXPR_IDENTIFIER,
    EXPR_CALL,
    EXPR_ARRAY_LITERAL
} ASTExpressionKind;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,

    OP_AND,
    OP_OR,
    OP_NOT,

    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_BIT_NOT,
    
    OP_SHIFT_LEFT,
    OP_SHIFT_RIGHT,

    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE
} ASTOperator;

typedef enum {
    STMT_VAR_DECL,
    STMT_ASSIGN,
    STMT_IF,
    STMT_WHILE,
    STMT_REPEAT,
    STMT_FOR,
    STMT_RETURN,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_CALL,
    STMT_BLOCK
} ASTStmtKind;

typedef enum {
    SUBPROGRAM_METHOD,
    SUBPROGRAM_PROCEDURE,
    SUBPROGRAM_FUNCTION
} ASTSubprogramKind;

typedef struct ASTStatement ASTStatement;

typedef struct ASTType {
    ASTTypeKind Kind;
    struct ASTType *ElementType;

    const char *Name;
} ASTType;

typedef struct ASTExpression {
    ASTExpressionKind Kind;

    union {
        const char *Identifier;

        struct {
            struct ASTExpression *Left;
            struct ASTExpression *Right;

            ASTOperator Op;
        } Binary;

        struct {
            ASTOperator Op;
            struct ASTExpression *Operand;
        } Unary;

        struct {
            double Number;
            uint64_t Int;

            char *String;
            char Char;

            int Bool;
        } Literal;

        struct {
            struct ASTExpression *Callee;
            struct ASTExpression **Args;

            size_t ArgCount;
        } Call;

        struct {
            struct ASTExpression **Elements;
            size_t Count;
        } Array;
    };
} ASTExpression;

struct ASTStatement {
    ASTStmtKind Kind;

    union {
        struct {
            const char *Name;

            ASTType *Type;
            ASTExpression *Initializer;

            int Mutable;
        } VarDecl;

        struct {
            ASTExpression *Target;
            ASTExpression *Value;
        } Assign;

        struct {
            ASTExpression *Condition;
            ASTStatement **ThenBlock;
            size_t ThenCount;
                
            struct {
                ASTExpression *Condition;
                ASTStatement **Block;
                size_t Count;
            } *ElseIfs;

            size_t ElseIfCount;
                
            ASTStatement **ElseBlock;
            size_t ElseCount;
        } If;

        struct {
            ASTExpression *Condition;
            ASTStatement **Body;
            size_t Count;
        } While;
        
        struct {
            ASTStatement **Body;
            size_t Count;
            ASTExpression *Until;
        } Repeat;

        struct {
            const char *Iterator;
            ASTExpression *Start;
            ASTExpression *End;
            ASTStatement **Body;

            size_t Count;
        } For;

        struct {
            ASTExpression *Value;
        } Return;

        struct {
            ASTExpression *CallExpression;
        } Call;

        struct {
            ASTExpression *Expression
        } ExpressionStmt;

        struct {
            ASTStatement **Statements;
            size_t Count;
        } Block;
    };
};

typedef struct ASTSubprogram {
    ASTSubprogramKind Kind;

    const char *Name;

    struct {
        const char *Name;
        ASTType *Type;
    } *Params;

    size_t ParamCount;
    ASTType *ReturnType;

    ASTStatement **Body;
    size_t BodyCount;
} ASTSubprogram;

typedef struct ASTProgram {
    ASTSubprogram **Subprograms;
    size_t SubprogramCount;

    ASTStatement **Statements;
    size_t StatementCount;
} ASTProgram;

#endif
