#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

// -------------------------
// PDF File Struct
// -------------------------
typedef struct {
    unsigned char *buffer;
    size_t size;
    size_t current_position;
} PdfFile;

// -------------------------
// File I/O Functions
// -------------------------

/**
 * Reads PDF file into memory.
 * Returns pointer to the PdfFile or NULL on fail
 */
PdfFile* read_pdf_file(const char* filename);

/**
 * Frees memory allocated for a PdfFile
 */
void free_pdf(PdfFile *pdf);

// -------------------------
// Byte Reading Helpers
// -------------------------

/**
 * Peeks at current byte without advancing position
 * Returns -1 if EOF
 */
int peek_byte(PdfFile *pdf);

/**
 * Gets current byte and advances position
 * Returns -1 if EOF
 */
int get_byte(PdfFile *pdf);

#endif