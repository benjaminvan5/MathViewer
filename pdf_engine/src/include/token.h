#ifndef TOKEN_H
#define TOKEN_H
#include "lexer.h"

typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_NAME,
    TOKEN_STRING,
    TOKEN_ARRAY_START,
    TOKEN_ARRAY_END,
    TOKEN_DICT_START,
    TOKEN_DICT_END,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    union {
        int i;
        double f;
        char* str;
    };
} Token;

void free_token(Token* t);
Token parse_number(PdfFile* pdf);
Token parse_name(PdfFile* pdf);
Token parse_string_literal(PdfFile* pdf);
Token parse_hex_string(PdfFile* pdf);
Token parse_array_start(PdfFile* pdf);
Token parse_array_end(PdfFile* pdf);
Token parse_dict_start(PdfFile* pdf);
Token parse_dict_end(PdfFile* pdf);
Token next_token(PdfFile* pdf);

#endif TOKEN_H