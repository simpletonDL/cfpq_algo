#pragma once

#include "response.h"
#include "../grammar/grammar.h"
#include "../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../graph/graph.h"

int cfpq_cpu_1(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size);

int cfpq_cpu_2(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size);

int cfpq_cpu_3(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size);