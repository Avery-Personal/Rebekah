#include "../Lexer/Lexer.h"
#include "AST.h"

ASTProgram *ParseProgram(Lexer *_Lexer);
ASTSubprogram* ParseSubprogram(Lexer *_Lexer);

ASTStatement* ParseStatement(Lexer *_Lexer);
ASTStatement* ParseBlock(Lexer *_Lexer);

ASTType* ParseType(Lexer *_Lexer);
