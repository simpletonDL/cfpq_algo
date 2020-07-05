#include "single_source.h"
#include "../utils/arr.h"

SingleSourceIndex *SingleSourceIndex_New(Grammar *grammar, size_t graph_size) {
	SingleSourceIndex *index = malloc(sizeof(SingleSourceIndex));
	index->grammar = grammar;
	for (int i = 0; i < MAX_GRAMMAR_SIZE; i++) {
		GrB_Matrix_new(&index->source_nonterm[i], GrB_BOOL, graph_size, graph_size);
		GrB_Matrix_new(&index->nonterm[i], GrB_BOOL, graph_size, graph_size);
	}
	return index;
}

void *SingleSourceIndex_InitSimpleRules(SingleSourceIndex *index, GraphRepr *graph) {
	for (int i = 0; i < index->grammar->simple_rules_count; ++i) {
		MapperIndex nonterm_l = index->grammar->simple_rules[i].l;
		MapperIndex term_r = index->grammar->simple_rules[i].r;

		for (int j = 0; j < array_len(graph->edges.arr); j++) {
			const char *rel = graph->edges.arr[j];
			MapperIndex rel_id = ItemMapper_GetPlaceIndex((ItemMapper *) &index->grammar->tokenMapper, rel);

			if (rel_id == term_r) {
				GrB_eWiseAdd(index->nonterm[nonterm_l], NULL, NULL, GrB_LOR,
						index->nonterm[nonterm_l], graph->relations[j], NULL);
			}
		}
	}
}

void SingleSourceResponse_Init(SingleSourceResponse *response) {
	response->constract_source_time = 0;
	response->source_mul_time = 0;
	response->right_part_mul_time = 0;
	response->simple_rule_time = 0;
	response->add_to_left_source_time = 0;
	response->add_to_right_source_time = 0;
	response->add_to_index_source_time = 0;
}
