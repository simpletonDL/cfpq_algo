#pragma once

#include "response.h"
#include "../grammar/grammar.h"
#include "../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../graph/graph.h"
#include "single_source.h"

int cfpq_cpu_1(const Grammar *grammar, CfpqResponse *response,
			   const GrB_Matrix *relations, const char **relations_names,
			   size_t relations_count, size_t graph_size);

int cfpq_cpu_2(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size);

int cfpq_cpu_3(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size);

GrB_Matrix * cfpq_cpu_4(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size);

SingleSourceResponse cfpq_cpu_single_source(SingleSourceIndex *index,
											GraphRepr *graph, GrB_Vector source, const char *start_nonterm);

void cfpq_cpu_single_source_stupied(SingleSourceIndex *index,
									GraphRepr *graph, GrB_Vector source, const char *start_nonterm);

SingleSourceResponse cfpq_cpu_single_source_opt(SingleSourceIndex *index,
												GraphRepr *graph, GrB_Vector source, const char *start_nonterm);
