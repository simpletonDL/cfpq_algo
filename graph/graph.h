#pragma once

#include <stdio.h>

#include "../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../grammar/grammar.h"
#include "../grammar/item_mapper.h"

#define MAX_GRAPH_RELATION_TYPES 50000
#define MAX_GRAPH_SIZE 100000

typedef struct {
    MapperIndex count;
    char items[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN];
} EntryMapper;

typedef struct {
    GrB_Matrix relations[MAX_GRAPH_RELATION_TYPES];

    EntryMapper edges;
    EntryMapper nodes;
} GraphRepr;

void GraphRepr_Init(GraphRepr* g);

void GraphRepr_InsertEdge(GraphRepr* g, const char *v, const char *edge, const char *to);

void GraphRepr_Load(GraphRepr *g, FILE *f);
