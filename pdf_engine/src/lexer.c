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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char *buffer;
    size_t size;
    size_t current_pos;
} PdfFile;

PdfFile* read_pdf_file(const char* filename) {
    FILE *f = fopen(filename, 'rb');
    if (!f) {
        perror("cant open file");
        return NULL;
    }

    // gets file size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    unsigned char* buffer = (unsigned char*)malloc(size);
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

    PdfFile *pdf = (PdfFile*)malloc(sizeof(PdfFile));
    pdf->buffer = buffer;
    pdf->size = size;
    pdf->current_pos = 0;
    return pdf;
}

int peek_byte(PdfFile *pdf) {
    if (pdf->current_pos >= pdf->size) return -1;
    return pdf->buffer[pdf->current_pos];
}

int get_byte(PdfFile *pdf) {
    if (pdf->current_pos >= pdf->size) return -1;
    return pdf->buffer[pdf->current_pos++];
}

int is_whitespace(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

void skip_whitespace(PdfFile *pdf) {
    while (pdf->current_pos <= pdf->size && is_whitespace(pdf->buffer[pdf->current_pos])) {
        pdf->current_pos++;
    }
}

void skip_comments(PdfFile *pdf) {
    if (peek_byte(pdf) != '%') return;
    while (pdf->current_pos <= pdf->size) {
        unsigned char c = get_byte(pdf);
        if (c == '\n' || c == '\r') break;
    }
}

void skip_whitespace_and_comments(PdfFile *pdf) {
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