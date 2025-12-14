#ifndef TOKEN_H
#define TOKEN_H
#include "lexer.h"

typedef enum { TOKEN_INT, TOKEN_FLOAT, TOKEN_NAME, TOKEN_EOF, TOKEN_UNKNOWN } TokenType;

typedef struct {
    TokenType type;
    union {
        int i;
        double f;
        char* str;
    };
} Token;

Token parse_number(PdfFile* pdf);
Token parse_number(PdfFile* pdf);
Token next_token(PdfFile* pdf);

#endif TOKEN_H