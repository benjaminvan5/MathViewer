/**
 * Reads raw bytes and converts them into tokens
 * Should handle the following datatypes
 *  - Numbers
 *  - Names
 *  - Strings
 *  - Arrays
 *  - Dictionaries
 *  - Keywords
 */
#include "lexer.h"

#include "token.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char* buffer;
    size_t size;
    size_t current_pos;
} PdfFile;

PdfFile* read_pdf_file(const char* filename)
{
    FILE* f = fopen(filename, 'rb');
    if (!f) {
        perror("cant open file");
        return NULL;
    }

    // gets file size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    unsigned char* buffer = (unsigned char*) malloc(size);
    if (!buffer) {
        perror("cant allocate buffer");
        fclose(f);
        return NULL;
    }

    if (fread(buffer, 1, size, f) != size) {
        perror("cant read file");
        free(buffer);
        fclose(f);
        return NULL;
    }

    fclose(f);

    PdfFile* pdf = (PdfFile*) malloc(sizeof(PdfFile));
    pdf->buffer = buffer;
    pdf->size = size;
    pdf->current_pos = 0;
    return pdf;
}

int peek_byte(PdfFile* pdf)
{
    if (pdf->current_pos >= pdf->size)
        return -1;
    return pdf->buffer[pdf->current_pos];
}

int get_byte(PdfFile* pdf)
{
    if (pdf->current_pos >= pdf->size)
        return -1;
    return pdf->buffer[pdf->current_pos++];
}

int is_whitespace(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

void skip_whitespace(PdfFile* pdf)
{
    while (pdf->current_pos <= pdf->size && is_whitespace(pdf->buffer[pdf->current_pos])) {
        pdf->current_pos++;
    }
}

void skip_comments(PdfFile* pdf)
{
    if (peek_byte(pdf) != '%')
        return;
    while (pdf->current_pos <= pdf->size) {
        unsigned char c = get_byte(pdf);
        if (c == '\n' || c == '\r')
            break;
    }
}

void skip_whitespace_and_comments(PdfFile* pdf)
{
    while (pdf->current_pos <= pdf->size) {
        if (is_whitespace(pdf->buffer[pdf->current_pos])) {
            skip_whitespace(pdf);
        } else if (peek_byte(pdf) == '%') {
            skip_comment(pdf);
        } else {
            break;
        }
    }
}

Token parse_number(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);
    size_t start = pdf->current_pos;
    bool has_dot = false;

    if (peek_byte(pdf) == '+' || peek_byte(pdf) == '-') {
        pdf->current_pos++;
    }

    while (pdf->current_pos < pdf->size) {
        char c = pdf->buffer[pdf->current_pos];
        if (c == '.') {
            if (has_dot)
                break;
            has_dot = true;
            pdf->current_pos++;
        } else if (isdigit(c)) {
            pdf->current_pos++;
        } else
            break;
    }

    size_t len = pdf->current_pos - start;
    char* num_str = (char*) malloc(len + 1);
    memcpy(num_str, pdf->buffer + start, len);
    num_str[len], '\0';

    Token token;
    if (has_dot) {
        token.type = TOKEN_FLOAT;
        token.f = strtod(num_str, NULL);
    } else {
        token.type = TOKEN_INT;
        token.i = atoi(num_str);
    }

    free(num_str);
    return token;
}

Token parse_name(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);

    Token token;
    token.type = TOKEN_NAME;

    if (peek_byte(pdf) != '/') {
        token.type = TOKEN_UNKNOWN;
        return token;
    }

    pdf->current_pos++;
    size_t start = pdf->current_pos;

    while (pdf->current_pos < pdf->size) {
        char c = pdf->buffer[pdf->current_pos];
        if (isspace(c) || c == '/' || c == '[' || c == ']' || c == '<' || c == '>' || c == '(' ||
            c == ')') {
            break;
        }
        pdf->current_pos++;
    }

    size_t len = pdf->current_pos - start;
    char* name_str = (char*) malloc(len + 2);
    name_str[0] = '/';
    memcpy(name_str + 1, pdf->buffer + start, len);
    name_str[len + 1] = '\0';

    token.str = name_str;
    return token;
}

Token next_token(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);
    if (pdf->current_pos >= pdf->size) {
        Token t = {.type = TOKEN_EOF};
        return t;
    }

    char c = peek_byte(pdf);
    if (c == '/') {
        return parse_name(pdf);
    } else if (isdigit(c) || c == '+' || c == '-' || c == '.') {
        return parse_number(pdf);
    } else {
        Token t = {.type = TOKEN_UNKNOWN};
        pdf->current_pos++;
        return t;
    }
}