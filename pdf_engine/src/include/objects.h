#ifndef PDF_OBJECT_H
#define PDF_OBJECT_H

#include "token.h"

#include <stddef.h>

typedef enum { PDF_INT, PDF_FLOAT, PDF_NAME, PDF_STRING, PDF_ARRAY, PDF_DICT } PdfObjectType;

typedef struct PdfObject PdfObject;

struct PdfObject {
    PdfObjectType type;
    union {
        int i;
        double f;
        char* str;
        struct {
            PdfObject** items;
            size_t count;
        } array;
        struct {
            char** keys;
            PdfObject** values;
            size_t count;
        } dict;
    };
};

PdfObject* create_pdf_object();
void free_pdf_object(PdfObject* obj);
PdfObject *parse_array(PdfFile *pdf);
PdfObject* parse_dict(PdfFile *pdf);
PdfObject* parse_object(PdfFile *pdf);

#endif PDF_OBJECT_H