#include "objects.h"

#include "token.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PdfObject* create_pdf_object()
{
    PdfObject* obj = malloc(sizeof(PdfObject));
    memset(obj, 0, sizeof(PdfObject));
    return obj;
}

void free_pdf_object(PdfObject* obj)
{
    if (!obj)
        return;

    switch (obj->type) {
        case PDF_NAME:
        case PDF_STRING:
            if (obj->str)
                free(obj->str);
            break;
        case PDF_ARRAY:
            for (size_t i = 0; i < obj->array.count; i++) {
                free_pdf_object(obj->array.items[i]);
            }
            free(obj->array.items);
            break;
        case PDF_DICT:
            for (size_t i = 0; i < obj->dict.count; i++) {
                free_pdf_object(obj->dict.values[i]);
            }
            free(obj->dict.keys);
            free(obj->dict.values);
            break;
        default:
            break;
    }

    free(obj);
}

PdfObject* parse_array(PdfFile* pdf)
{
    Token t = next_token(pdf);
    if (t.type != TOKEN_ARRAY_START)
        return NULL;

    PdfObject* arr = create_pdf_object();
    arr->type = PDF_ARRAY;
    arr->array.items = NULL;
    arr->array.count = 0;

    while (true) {
        Token peek = next_token(pdf);
        if (peek.type == TOKEN_ARRAY_END)
            break;

        pdf->current_pos -= 1;
        PdfObject* item = parse_object(pdf);
        if (!item)
            break;
        arr->array.items = realloc(arr->array.items, sizeof(PdfObject*) * arr->array.count + 1);
        arr->array.items[arr->array.count++] = item;
    }

    return arr;
}

PdfObject* parse_dict(PdfFile* pdf)
{
    Token t = next_token(pdf);
    if (t.type != TOKEN_DICT_START)
        return NULL;

    PdfObject* dict = create_pdf_object();
    dict->type = PDF_DICT;
    dict->dict.keys = NULL;
    dict->dict.values = NULL;
    dict->dict.count = 0;

    while (true) {
        Token key = next_token(pdf);
        if (key.type == TOKEN_DICT_END)
            break;
        if (key.type != TOKEN_NAME) {
            printf("expected a key mate");
            break;
        }

        PdfObject* value = parse_object(pdf);
        dict->dict.keys = realloc(dict->dict.keys, sizeof(char*) * (dict->dict.count + 1));
        dict->dict.values = realloc(dict->dict.values, sizeof(PdfObject*) * (dict->dict.count + 1));

        dict->dict.keys[dict->dict.count] = strdup(key.str);
        free_token(&key);
        dict->dict.values[dict->dict.count++] = value;
    }

    return dict;
}

PdfObject* parse_object(PdfFile* pdf)
{
    Token t = next_token(pdf);
    PdfObject* obj = create_pdf_object();

    switch (t.type) {
        case TOKEN_INT:
            obj->type = PDF_INT;
            obj->i = t.i;
            break;
        case TOKEN_FLOAT:
            obj->type = PDF_FLOAT;
            obj->f = t.f;
            break;
        case TOKEN_NAME:
            obj->type = PDF_NAME;
            obj->str = strdup(t.str);
            free_token(&t);
            break;
        case TOKEN_STRING:
            obj->type = PDF_STRING;
            obj->str = strdup(t.str);
            free_token(&t);
            break;
        case TOKEN_ARRAY_START:
            pdf->current_pos -= 1;
            obj = parse_array(pdf);
            break;
        case TOKEN_DICT_START:
            pdf->current_pos -= 2;
            obj = parse_dict(pdf);
            break;
        default:
            free(obj);
            return NULL;
    }

    return obj;
}