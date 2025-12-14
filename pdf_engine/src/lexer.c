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

void free_pdf(PdfFile* pdf)
{
    if (!pdf)
        return;
    if (pdf->buffer)
        free(pdf->buffer);
    free(pdf);
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

void free_token(Token* t)
{
    if (!t)
        return;
    if ((t->type == TOKEN_NAME || t->type == TOKEN_STRING) && t->str) {
        free(t->str);
        t->str = NULL;
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

Token parse_string_literal(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);

    Token token;
    token.type = TOKEN_UNKNOWN;
    if (peek_byte(pdf) != '(')
        return token;

    pdf->current_pos++;
    size_t start = pdf->current_pos;

    int parenthesis_level = 1;
    size_t buf_size = 256;
    size_t len = 0;
    char* buffer = malloc(buf_size);

    while (pdf->current_pos < pdf->size && parenthesis_level > 0) {
        char c = get_byte(pdf);

        if (c == '(') {
            parenthesis_level++;
        } else if (c == ')') {
            parenthesis_level--;
            if (parenthesis_level == 0)
                break;
        } else if (c == '\\') {
            char next = get_byte(pdf);
            switch (next) {
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case '\\':
                    c = '\\';
                    break;
                case '(':
                    c = '(';
                    break;
                case ')':
                    c = ')';
                    break;
                default:
                    c = next;
                    break;
            }
        }

        if (len + 1 >= buf_size) {
            buf_size *= 2;
            buffer = realloc(buffer, buf_size);
        }
        buffer[len++] = c;
    }

    buffer[len] = '\0';
    token.type = TOKEN_NAME;
    token.str = buffer;
    return token;
}

Token parse_hex_string(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);

    Token token;
    token.type = TOKEN_UNKNOWN;
    if (peek_byte(pdf) != '<')
        return token;

    pdf->current_pos++;
    size_t buf_size = 256;
    size_t len = 0;
    char* buffer = malloc(buf_size);

    while (pdf->current_pos < pdf->size) {
        char c = get_byte(pdf);
        if (c == '<')
            break;
        if (isspace(c))
            continue;

        char hex[3] = {c, get_byte(pdf), '\0'};
        buffer[len++] = (char) strtol(hex, NULL, 16);

        if (len >= buf_size) {
            buf_size *= 2;
            buffer = realloc(buffer, buf_size);
        }
    }

    buffer[len] = '\0';
    token.type = TOKEN_NAME;
    token.str = buffer;
    return token;
}

Token parse_array_start(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);
    Token token = {.type = TOKEN_UNKNOWN};
    if (peek_byte(pdf) == '[') {
        pdf->current_pos++;
        token.type = TOKEN_ARRAY_START;
    }
    return token;
}

Token parse_array_end(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);
    Token token = {.type = TOKEN_UNKNOWN};
    if (peek_byte(pdf) == ']') {
        pdf->current_pos++;
        token.type = TOKEN_ARRAY_END;
    }
    return token;
}

Token parse_dict_start(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);
    Token token = {.type = TOKEN_UNKNOWN};
    if (peek_byte(pdf) == '<' && pdf->buffer[pdf->current_pos + 1] == '<') {
        pdf->current_pos += 2;
        token.type = TOKEN_DICT_START;
    }

    return token;
}


Token parse_dict_end(PdfFile* pdf)
{
    skip_whitespace_and_comments(pdf);
    Token token = {.type = TOKEN_UNKNOWN};
    if (peek_byte(pdf) == '>' && pdf->buffer[pdf->current_pos + 1] == '>') {
        pdf->current_pos += 2;
        token.type = TOKEN_DICT_END;
    }

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
    if (c == '/')
        return parse_name(pdf);
    else if (isdigit(c) || c == '+' || c == '-' || c == '.')
        return parse_number(pdf);
    else if (c == '(')
        return parse_string_literal(pdf);
    else if (c == '<') {
        if (pdf->buffer[pdf->current_pos + 1] == '<')
            return parse_dict_start(pdf);
        else
            return parse_hex_string(pdf);
    } else if (c == '>') {
        if (pdf->buffer[pdf->current_pos + 1] == '>')
            return parse_dict_end(pdf);
    } else if (c == '[')
        return parse_array_start(pdf);
    else if (c == ']')
        return parse_array_end(pdf);
    else {
        Token t = {.type = TOKEN_UNKNOWN};
        pdf->current_pos++;
        return t;
    }
}