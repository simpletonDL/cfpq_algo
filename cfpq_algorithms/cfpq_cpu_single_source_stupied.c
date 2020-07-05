#include "single_source.h"
#include "../graph/graph.h"
#include "../utils/arr.h"
#include "../utils/simple_timer.h"

double constract_source_time_stupied = 0;
double source_mul_time_stupied = 0;
double right_part_mul_time_stupied = 0;
double simple_rule_time_stupied = 0;

GrB_Matrix constract_source_stupied(GrB_Vector src) {
	GrB_Index nvals;
	GrB_Vector_nvals(&nvals, src);

	GrB_Index *I = malloc(sizeof(GrB_Index) * nvals);
	bool *values = malloc(sizeof(bool) * nvals);
	GrB_Vector_extractTuples_BOOL(I, values, &nvals, src);

	GrB_Matrix res;
	GrB_Matrix_new(&res, GrB_BOOL, MAX_GRAPH_SIZE, MAX_GRAPH_SIZE);
	for (GrB_Index i = 0; i < nvals; ++i) {
		GrB_Matrix_setElement_BOOL(res, true, I[i], I[i]);
	}

	free(values);
	free(I);
	return res;
}

void cfpq_cpu_single_source_stupied(SingleSourceIndex *index,
							GraphRepr *graph, GrB_Vector source, const char *start_nonterm) {
	double constract_source_timer[2];
	double source_mul_timer[2];
	double right_part_mul_timer[2];
	double simple_rule_timer[2];

	GrB_Descriptor reduce_desc;
	GrB_Descriptor_new(&reduce_desc);
	GrB_Descriptor_set(reduce_desc, GrB_INP0, GrB_TRAN);

	GrB_Descriptor reversed_desc;
	GrB_Descriptor_new(&reversed_desc);
	GrB_Descriptor_set(reversed_desc, GrB_MASK, GrB_SCMP);

	GrB_Index old_nvals, new_nvals;

	Grammar *grammar = index->grammar;
	uint64_t nonterm_count = array_len(grammar->nontermMapper.arr);
	uint64_t term_count = array_len(grammar->tokenMapper.arr);

	MapperIndex start = ItemMapper_GetPlaceIndex((ItemMapper *) &grammar->nontermMapper, start_nonterm);

	// Init source matrices
	GrB_Matrix source_nonterm[nonterm_count];
	for (int j = 0; j < nonterm_count; ++j) {
		GrB_Matrix_new(&source_nonterm[j], GrB_BOOL, MAX_GRAPH_SIZE, MAX_GRAPH_SIZE);
	}
	GrB_wait();
	simple_tic(constract_source_timer);
	source_nonterm[start] = constract_source_stupied(source);
	GrB_wait();
	constract_source_time_stupied += simple_toc(constract_source_timer);

	bool changed = true;
	for (;changed;) {
		changed = false;

		for (int i = 0; i < grammar->complex_rules_count; ++i) {
			MapperIndex nonterm_l = grammar->complex_rules[i].l;
			MapperIndex nonterm_r1 = grammar->complex_rules[i].r1;
			MapperIndex nonterm_r2 = grammar->complex_rules[i].r2;

//			if (strcmp(grammar->nontermMapper.arr[nonterm_l], "s") == 0
//				strcmp(grammar->nontermMapper.arr[nonterm_r1], "s3") == 0 &&
//				strcmp(grammar->nontermMapper.arr[nonterm_r2], "s6") == 0
//				) {
//				printf("%s %s %s | %d %d %d\n",
//						grammar->nontermMapper.arr[nonterm_l],
//						grammar->nontermMapper.arr[nonterm_r1],
//						grammar->nontermMapper.arr[nonterm_r2],
//					   nonterm_l, nonterm_r1, nonterm_r2);
//			GxB_print(source_nonterm[nonterm_l], GxB_COMPLETE);
//			}
			GrB_Matrix_nvals(&old_nvals, source_nonterm[nonterm_r1]);
			GrB_eWiseAdd(source_nonterm[nonterm_r1], NULL, NULL, GrB_LOR,
						 source_nonterm[nonterm_r1], source_nonterm[nonterm_l], NULL);
			GrB_Matrix_nvals(&new_nvals, source_nonterm[nonterm_r1]);
			if (old_nvals != new_nvals) {
				changed = true;
			}

			GrB_Matrix A;
			GrB_Matrix_new(&A, GrB_BOOL, MAX_GRAPH_SIZE, MAX_GRAPH_SIZE);
			GrB_wait();
			simple_tic(source_mul_timer);
			GrB_mxm(A, NULL, NULL, GxB_LOR_LAND_BOOL,
					source_nonterm[nonterm_l], index->nonterm[nonterm_r1], NULL);
			GrB_wait();
			source_mul_time_stupied += simple_toc(source_mul_timer);
//			GxB_print(A, GxB_COMPLETE);

			GrB_Vector source_nonterm_r2_new;
			GrB_Vector_new(&source_nonterm_r2_new, GrB_BOOL, MAX_GRAPH_SIZE);
			GrB_reduce(source_nonterm_r2_new, NULL, NULL, GrB_LOR, A, reduce_desc);

			GrB_Matrix source_nonterm_r2_new_matrix;
			GrB_wait();
			simple_tic(constract_source_timer);
			source_nonterm_r2_new_matrix =
					constract_source_stupied(source_nonterm_r2_new);
			GrB_wait();
			constract_source_time_stupied += simple_toc(constract_source_timer);
			GrB_free(&source_nonterm_r2_new);

			GrB_Matrix_nvals(&old_nvals, source_nonterm[nonterm_r2]);
			GrB_eWiseAdd(source_nonterm[nonterm_r2], NULL, NULL, GrB_LOR,
						 source_nonterm[nonterm_r2], source_nonterm_r2_new_matrix, NULL);
			GrB_Matrix_nvals(&new_nvals, source_nonterm[nonterm_r2]);
			if (old_nvals != new_nvals) {
				changed = true;
			}
			GrB_free(&source_nonterm_r2_new_matrix);


			GrB_Matrix_nvals(&old_nvals, index->nonterm[nonterm_l]);
			GrB_wait();
			simple_tic(right_part_mul_timer);
			GrB_mxm(index->nonterm[nonterm_l], GrB_NULL, GrB_LOR, GxB_LOR_LAND_BOOL,
					A, index->nonterm[nonterm_r2], GrB_NULL);
			GrB_wait();
			right_part_mul_time_stupied += simple_toc(right_part_mul_timer);
			GrB_Matrix_nvals(&new_nvals, index->nonterm[nonterm_l]);
			if (old_nvals != new_nvals) {
				changed = true;
			}
			GrB_free(&A);
		}
		GrB_wait();
		simple_tic(simple_rule_timer);
		for (int i = 0; i < grammar->simple_rules_count; ++i) {
			MapperIndex nonterm_l = grammar->simple_rules[i].l;
			MapperIndex term_r = grammar->simple_rules[i].r;

			for (int j = 0; j < array_len(graph->edges.arr); j++) {
				const char *rel = graph->edges.arr[j];
				MapperIndex rel_id = ItemMapper_GetPlaceIndex((ItemMapper *) &grammar->tokenMapper, rel);

				if (rel_id == term_r) {
					GrB_Matrix_nvals(&old_nvals, index->nonterm[nonterm_l]);
					GrB_mxm(index->nonterm[nonterm_l], NULL, GrB_LOR, GxB_LOR_LAND_BOOL,
							source_nonterm[nonterm_l], graph->relations[j], NULL);
					GrB_Matrix_nvals(&new_nvals, index->nonterm[nonterm_l]);
					if (new_nvals != old_nvals) {
						changed = true;
					}
				}
			}
		}
		GrB_wait();
		simple_rule_time_stupied += simple_toc(simple_rule_timer);
	}

	for (int i = 0; i < nonterm_count; ++i) {
		GrB_free(&index->source_nonterm[i]);
		GrB_free(&source_nonterm[i]);
	}
}