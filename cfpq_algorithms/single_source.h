#pragma once

#include "../grammar/grammar.h"
#include "../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../graph/graph.h"

typedef struct {
	Grammar *grammar;
	GrB_Matrix source_nonterm[MAX_GRAMMAR_SIZE];
	GrB_Matrix nonterm[MAX_GRAMMAR_SIZE];
	GrB_Matrix input[MAX_GRAMMAR_SIZE];
} SingleSourceIndex;

SingleSourceIndex* SingleSourceIndex_New(Grammar *grammar, size_t graph_size);

typedef struct {
	double constract_source_time;
	double source_mul_time;
	double right_part_mul_time;
	double simple_rule_time;
	double add_to_left_source_time;
	double add_to_right_source_time;
	double add_to_index_source_time;
} SingleSourceResponse;

void SingleSourceResponse_Init(SingleSourceResponse *response);
void *SingleSourceIndex_InitSimpleRules(SingleSourceIndex *index, GraphRepr *g);