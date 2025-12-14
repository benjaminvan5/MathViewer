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