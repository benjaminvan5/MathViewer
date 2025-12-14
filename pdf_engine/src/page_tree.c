#include "page_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PdfPage* parse_page_tree(PdfObject* node)
{
    if (!node || node->type != PDF_DICT)
        return NULL;

    PdfPage* page = malloc(sizeof(PdfPage));
    page->dict = node;
    page->children = NULL;
    page->child_count = 0;

    // search for type key. if we are in pages we recurse into kids
    for (size_t i = 0; i < node->dict.count; i++) {
        char* key = node->dict.keys[i];
        PdfObject* value = node->dict.values[i];

        if (strcmp(key, "Type") == 0 && value->type == PDF_NAME &&
            strcmp(value->str, "/Pages") == 0) {
            for (size_t j = 0; j < node->dict.count; j++) {
                if (strcmp(node->dict.keys[j], "Kids") == 0 && node->dict.values[j] == PDF_ARRAY) {
                    PdfObject* kids = node->dict.values[j];
                    page->child_count = kids->array.count;
                    page->children = malloc(sizeof(PdfPage*) * page->child_count);

                    for (size_t k = 0; k < kids->array.count; k++) {
                        page->children[k] = parge_page_tree(kids->array.items[k]);
                    }

                    break;
                }
            }
            return page;
        }
    }

    return page;
}

void free_page_tree(PdfPage *page) {
    if (!page) return;
    for (size_t i = 0; i < page->child_count; i++) {
        free_page_tree(page->children[i]);
    }
    free(page->children);
    free(page);
}