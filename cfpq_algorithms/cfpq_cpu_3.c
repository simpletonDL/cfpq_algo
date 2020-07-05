#include <assert.h>

#include "algorithms.h"
#include "../utils/item_mapper.h"
#include "../utils/simple_timer.h"
#include "../utils/arr.h"

int cfpq_cpu_3(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size) {
	double ATOP_PLUS_B_time = 0;
	double ANEW_PLUS_time = 0;
	double MUL_TIME = 0;

    // Create matrices
    uint64_t nonterm_count = array_len(grammar->nontermMapper.arr);
    GrB_Matrix A_top[nonterm_count];
    GrB_Matrix B[nonterm_count];

    for (uint64_t i = 0; i < nonterm_count; ++i) {
        GrB_Info A_top_info = GrB_Matrix_new(&A_top[i], GrB_BOOL, graph_size, graph_size);
        GrB_Info B_info = GrB_Matrix_new(&B[i], GrB_BOOL, graph_size, graph_size);

        if (A_top_info != GrB_SUCCESS || B_info !=GrB_SUCCESS)
            return 1;
    }

    // Initialize A_top with simple rules, B with empty.
    for (int i = 0; i < relations_count; i++) {
        const char *terminal = relations_names[i];
        MapperIndex terminal_id = ItemMapper_GetPlaceIndex((ItemMapper *) &grammar->tokenMapper, terminal);

        if (terminal_id != array_len(grammar->tokenMapper.arr)) {
            for (int j = 0; j < grammar->simple_rules_count; j++) {
                const SimpleRule *simpleRule = &grammar->simple_rules[j];

                if (simpleRule->r == terminal_id) {
                    GrB_eWiseAdd_Matrix_BinaryOp(A_top[simpleRule->l], NULL, NULL,
                            GrB_LOR, A_top[simpleRule->l], relations[i], NULL);
                }
            }
        }
    }

    // Create monoid and semiring
    GrB_Monoid monoid;
    GrB_Semiring semiring;

    GrB_Info info = GrB_Monoid_new_BOOL(&monoid, GrB_LOR, false);
    assert(info == GrB_SUCCESS && "GraphBlas: failed to construct the monoid\n");

    info = GrB_Semiring_new(&semiring, monoid, GrB_LAND);
    assert(info == GrB_SUCCESS && "GraphBlas: failed to construct the semiring\n");

    // Create descriptor
    GrB_Descriptor reversed_mask;
    GrB_Descriptor_new(&reversed_mask);
    GrB_Descriptor_set(reversed_mask, GrB_MASK, GrB_SCMP);

    // Super-puper algorithm
    bool matrices_is_changed = true;
    while(matrices_is_changed) {
//        printf("%d", response->iteration_count++);

        // Initialize new A_top and B matrices
        GrB_Matrix A_new[nonterm_count];
        GrB_Matrix B_new[nonterm_count];
        for (uint64_t i = 0; i < nonterm_count; ++i) {
            GrB_Matrix_new(&A_new[i], GrB_BOOL, graph_size, graph_size);
            GrB_Matrix_new(&B_new[i], GrB_BOOL, graph_size, graph_size);
            {
				double timer[2];
				simple_tic(timer);
				GrB_eWiseAdd_Matrix_BinaryOp(B_new[i], NULL, NULL, GrB_LOR, A_top[i], B[i], NULL);
				ATOP_PLUS_B_time += simple_toc(timer);
			}
        }

        for (int i = 0; i < grammar->complex_rules_count; ++i) {
            MapperIndex nonterm_l = grammar->complex_rules[i].l;
            MapperIndex nonterm_r1 = grammar->complex_rules[i].r1;
            MapperIndex nonterm_r2 = grammar->complex_rules[i].r2;

            // create product matrices
            GrB_Matrix AB, BA, AA;
            GrB_Matrix_new(&AB, GrB_BOOL, graph_size, graph_size);
            GrB_Matrix_new(&BA, GrB_BOOL, graph_size, graph_size);
            GrB_Matrix_new(&AA, GrB_BOOL, graph_size, graph_size);

            // Compute product matrices
			{
				double timer[2];
				simple_tic(timer);
				GrB_mxm(AB, B[nonterm_l], NULL, semiring,
						A_top[nonterm_r1], B[nonterm_r2], reversed_mask);
				GrB_mxm(BA, B[nonterm_l], NULL, semiring,
						B[nonterm_r1], A_top[nonterm_r2], reversed_mask);
				GrB_mxm(AA, B[nonterm_l], NULL, semiring,
						A_top[nonterm_r1], A_top[nonterm_r2], reversed_mask);
				MUL_TIME += simple_toc(timer);
			}
            // Compute total A_new
			{
				double timer[2];
				simple_tic(timer);
				GrB_eWiseAdd_Matrix_BinaryOp(A_new[nonterm_l], NULL, NULL, GrB_LOR, A_new[nonterm_l], AB, NULL);
				GrB_eWiseAdd_Matrix_BinaryOp(A_new[nonterm_l], NULL, NULL, GrB_LOR, A_new[nonterm_l], BA, NULL);
				GrB_eWiseAdd_Matrix_BinaryOp(A_new[nonterm_l], NULL, NULL, GrB_LOR, A_new[nonterm_l], AA, NULL);
				ANEW_PLUS_time += simple_toc(timer);
			}
#ifdef DEBUG
            printf("%s -> %s %s\n",
                    grammar->nontermMapper.items[nonterm_l],
                    grammar->nontermMapper.items[nonterm_r1],
                    grammar->nontermMapper.items[nonterm_r2]);
            GxB_print(A_new[nonterm_l], GxB_SHORT);
#endif
        }

        // Check existing new elements and write result to next step
        matrices_is_changed = false;
        for (uint64_t i = 0; i < nonterm_count; ++i) {
            GrB_Index nvals_new;
            GrB_Matrix_nvals(&nvals_new, A_new[i]);

            if (nvals_new != 0)
                matrices_is_changed = true;

            GrB_Matrix_dup(&A_top[i], A_new[i]);
            GrB_Matrix_free(&A_new[i]);

            GrB_Matrix_dup(&B[i], B_new[i]);
            GrB_Matrix_free(&B_new[i]);
        }

    }

    // clean and write response
    CfpqResponse_Init(response);
    for (int i = 0; i < nonterm_count; i++) {
        const char* nonterm;

        GrB_Index nvals_B, nvals_A_top;
        GrB_Matrix_nvals(&nvals_B, B[i]);
        GrB_Matrix_nvals(&nvals_A_top, A_top[i]);

        nonterm = ItemMapper_Map((ItemMapper *) &grammar->nontermMapper, i);
        CfpqResponse_Append(response, nonterm, nvals_B + nvals_A_top);

        GrB_Matrix_free(&B[i]);
        GrB_Matrix_free(&A_top[i]);
    }
    GrB_Semiring_free(&semiring);
    GrB_Monoid_free(&monoid);

    printf("Statistics:\n");
    printf("Mul time: %f\n", MUL_TIME);
    printf("A_top + B time: %f\n", ATOP_PLUS_B_time);
    printf("A_new + time: %f\n", ANEW_PLUS_time);
    printf("Sum stat: %f\n", MUL_TIME + ATOP_PLUS_B_time + ANEW_PLUS_time);

    return 0;
}