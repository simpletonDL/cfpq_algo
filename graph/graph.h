#pragma once

#include <stdio.h>

#include "../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../grammar/grammar.h"
#include "../utils/item_mapper.h"

#define MAX_GRAPH_RELATION_TYPES 100
#define MAX_GRAPH_SIZE 1000000

typedef struct {
	const char **arr;
} EntryMapper;

typedef struct {
    GrB_Matrix relations[MAX_GRAPH_RELATION_TYPES];
	int max_node_id;
    EntryMapper edges;
} GraphRepr;

void GraphRepr_Init(GraphRepr* g);

void GraphRepr_InsertEdge(GraphRepr* g, int v_id, const char *edge, int to_id);

void GraphRepr_Load(GraphRepr *g, FILE *f);
