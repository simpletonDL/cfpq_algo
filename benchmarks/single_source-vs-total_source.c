#include <assert.h>
#include "../graph/graph.h"
#include "../cfpq_algorithms/algorithms.h"
#include "../cfpq_algorithms/response.h"
#include "../utils/simple_timer.h"
#include "single_source-vs-total_source.h"
#include "../utils/arr.h"


// 1000 -> 64.8, 10000 -Ю 46.7
void singe_source_vs_total_source(const char *graph_file, const char *grammar_file, int batch_size) {

	// Initialize GraphBLAS
	GrB_init(GrB_NONBLOCKING);

	// Load graph
	FILE* f = fopen(graph_file, "r");
	assert(f != NULL);

	GraphRepr g;
	GraphRepr_Load(&g, f);
	fclose(f);
	printf("Max node id: %d\n", g.max_node_id);

	// Load grammar
	f = fopen(grammar_file, "r");
	assert(f != NULL);

	Grammar grammar;
	Grammar_Load(&grammar, f);
	fclose(f);

	// Create index
	SingleSourceIndex *index_smart = SingleSourceIndex_New(&grammar, MAX_GRAPH_SIZE);
	SingleSourceIndex *index_smart_opt = SingleSourceIndex_New(&grammar, MAX_GRAPH_SIZE);
	SingleSourceIndex *index_brute = SingleSourceIndex_New(&grammar, MAX_GRAPH_SIZE);

	SingleSourceIndex_InitSimpleRules(index_smart_opt, &g);
	// Create source
	GrB_Vector source;
	GrB_Vector_new(&source, GrB_BOOL, MAX_GRAPH_SIZE);
	int source_nvals = 0;

	double timer[2];

	double *single_source_iter_times = array_new(double, 10);
	double *constract_source_times = array_new(double, 10);
	double *source_mul_times = array_new(double, 10);
	double *right_part_mul_times = array_new(double, 10);
	double *simple_rule_times = array_new(double, 10);
	double *add_to_left_source_times = array_new(double, 10);
	double *add_to_right_source_times = array_new(double, 10);
	double *add_to_index_source_times = array_new(double, 10);

	double *single_source_stupied_iter_times = array_new(double, 10);

	double *single_source_opt_iter_times = array_new(double, 10);
	{
		for (int i = 0; i <= g.max_node_id; ++i) {
			if (i % batch_size == 0) {
				printf("Iter: %d\n", i);
			}
			if (source_nvals == batch_size) {
				double iter_time;
				{
					simple_tic(timer);
					SingleSourceResponse response = cfpq_cpu_single_source(index_smart, &g, source, "s");
					iter_time = simple_toc(timer);
					array_append(single_source_iter_times, iter_time);
					array_append(constract_source_times, response.constract_source_time);
					array_append(source_mul_times, response.source_mul_time);
					array_append(right_part_mul_times, response.right_part_mul_time);
					array_append(simple_rule_times, response.simple_rule_time);
					array_append(add_to_left_source_times, response.add_to_left_source_time);
					array_append(add_to_right_source_times, response.add_to_right_source_time);
					array_append(add_to_index_source_times, response.add_to_index_source_time);
				}
				{
					SingleSourceIndex *new_index = SingleSourceIndex_New(&grammar, MAX_GRAPH_SIZE);
					simple_tic(timer);
					cfpq_cpu_single_source_stupied(new_index, &g, source, "s");
					iter_time = simple_toc(timer);
					array_append(single_source_stupied_iter_times, iter_time);

					for (int j = 0; j < array_len(new_index->grammar->nontermMapper.arr); ++j) {
						GrB_eWiseAdd(index_brute->nonterm[j], NULL, NULL, GrB_LOR,
									 index_brute->nonterm[j], new_index->nonterm[j], NULL);
						GrB_free(&new_index->nonterm[j]);
					}
				}
				{
					simple_tic(timer);
					cfpq_cpu_single_source_opt(index_smart_opt, &g, source, "s");
					iter_time = simple_toc(timer);
					array_append(single_source_opt_iter_times, iter_time);
				}
				GrB_Vector_clear(source);
				source_nvals = 0;
			}
			GrB_Vector_setElement_BOOL(source, true, i);
			source_nvals++;
		}
		if (source_nvals != 0) {
			double iter_time;
			{
				simple_tic(timer);
				SingleSourceResponse response = cfpq_cpu_single_source(index_smart, &g, source, "s");
				iter_time = simple_toc(timer);
				array_append(single_source_iter_times, iter_time);
				array_append(constract_source_times, response.constract_source_time);
				array_append(source_mul_times, response.source_mul_time);
				array_append(right_part_mul_times, response.right_part_mul_time);
				array_append(simple_rule_times, response.simple_rule_time);
				array_append(add_to_left_source_times, response.add_to_left_source_time);
				array_append(add_to_right_source_times, response.add_to_right_source_time);
				array_append(add_to_index_source_times, response.add_to_index_source_time);
				source_nvals = 0;
			}
			{
				SingleSourceIndex *new_index = SingleSourceIndex_New(&grammar, MAX_GRAPH_SIZE);
				simple_tic(timer);
				cfpq_cpu_single_source_stupied(new_index, &g, source, "s");
				iter_time = simple_toc(timer);
				array_append(single_source_stupied_iter_times, iter_time);

				for (int j = 0; j < array_len(new_index->grammar->nontermMapper.arr); ++j) {
					GrB_eWiseAdd(index_brute->nonterm[j], NULL, NULL, GrB_LOR,
								 index_brute->nonterm[j], new_index->nonterm[j], NULL);
					GrB_free(&new_index->nonterm[j]);
				}
			}
			{
				simple_tic(timer);
				cfpq_cpu_single_source_opt(index_smart_opt, &g, source, "s");
				iter_time = simple_toc(timer);
				array_append(single_source_opt_iter_times, iter_time);
			}
			GrB_Vector_clear(source);
		}
	}

	printf("Smart control sum:\n");
	for (int i = 0; i < array_len(index_smart->grammar->nontermMapper.arr); ++i) {
		GrB_Index nvals;
		GrB_Matrix_nvals(&nvals, index_smart->nonterm[i]);
		GrB_Index source_nvals_nonterm;
		GrB_Matrix_nvals(&source_nvals_nonterm, index_smart->source_nonterm[i]);
		printf("Nonterm %s: %lu / %lu\n", index_smart->grammar->nontermMapper.arr[i], nvals, source_nvals_nonterm);
	}

	printf("Smart opt control sum:\n");
	for (int i = 0; i < array_len(index_smart_opt->grammar->nontermMapper.arr); ++i) {
		GrB_Index nvals;
		GrB_Matrix_nvals(&nvals, index_smart_opt->nonterm[i]);
		GrB_Index source_nvals_nonterm;
		GrB_Matrix_nvals(&source_nvals_nonterm, index_smart_opt->source_nonterm[i]);
		printf("Nonterm %s: %lu / %lu\n", index_smart_opt->grammar->nontermMapper.arr[i], nvals, source_nvals_nonterm);
	}

	printf("Brute control sum:\n");
	for (int i = 0; i < array_len(index_smart->grammar->nontermMapper.arr); ++i) {
		GrB_Index nvals;
		GrB_Matrix_nvals(&nvals, index_brute->nonterm[i]);
		printf("Nonterm %s: %lu\n", index_smart->grammar->nontermMapper.arr[i], nvals);
	}

	printf("Iter i: smart vs smart_opt vs brute | make src, INP*R1, R1*R2, A->a, INP+R1_INP, INP+R2_INP, INP->index\n");
 	for (int j = 0; j < array_len(single_source_iter_times); ++j) {
 		printf("Iter %d: %f vs %f vs %f | %f %f %f %f %f %f %f\n",
 				j,
 				single_source_iter_times[j],
 				single_source_opt_iter_times[j],
 				single_source_stupied_iter_times[j],
			    constract_source_times[j],
			    source_mul_times[j],
				right_part_mul_times[j],
				simple_rule_times[j],
				add_to_left_source_times[j],
				add_to_right_source_times[j],
				add_to_index_source_times[j]
			    );
 	}
	double total_single_source_time = 0;
	double total_single_source_opt_time = 0;
	double total_single_source_stupied_time = 0;
	for (int k = 0; k < array_len(single_source_iter_times); ++k) {
		total_single_source_time += single_source_iter_times[k];
		total_single_source_opt_time += single_source_opt_iter_times[k];
		total_single_source_stupied_time += single_source_stupied_iter_times[k];
	}
	printf("smart: %f, smart_opt: %f, brute: %f\n",
			total_single_source_time,
			total_single_source_opt_time,
			total_single_source_stupied_time);

	FILE *f_res = fopen("result.csv", "w");
	fputs("iter,smart,brute\n", f_res);
	for (int k = 0; k < array_len(single_source_iter_times); ++k) {
		fprintf(f_res, "%d,%f,%f\n", k, single_source_iter_times[k], single_source_stupied_iter_times[k]);
	}
	fprintf(f_res, "total,%f,%f", total_single_source_time, total_single_source_stupied_time);
	fclose(f_res);

	// Total source
	CfpqResponse response;
	double total_time_total_source;

	double total_timer[2];
	simple_tic(total_timer);
    cfpq_cpu_1(&grammar, &response, g.relations, g.edges.arr, array_len(g.edges.arr), MAX_GRAPH_SIZE);
	total_time_total_source = simple_toc(total_timer);

	for (int i = 0; i < response.count; ++i) {
        printf("Nonterm %s: %lu\n", response.nonterms[i], response.control_sums[i]);
    }
	printf("Time: %f\n", total_time_total_source);
}
