#ifndef PAGE_TREE_H
#define PAGE_TREE_H

#include "objects.h"

typedef struct PdfPage PdfPage;

struct PdfPage {
    PdfObject *dict;
    struct PdfPage **children;
    size_t child_count;
};

PdfPage *parse_page_tree(PdfObject *root);
void free_page_tree(PdfPage *page);

#endif PAGE_TREE_H