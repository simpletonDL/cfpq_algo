#include "index.h"
#include "algorithms.h"
#include "../utils/helpers.h"
#include "../utils/arr.h"

GrB_Matrix * cfpq_cpu_4(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size) {
    // create index type and operations
    GrB_Type IndexType;
    check_info(GrB_Type_new(&IndexType, sizeof(Index)));

    GrB_BinaryOp IndexType_Add, IndexType_Mul;
    check_info(GrB_BinaryOp_new(&IndexType_Add, Index_Add, IndexType, IndexType, IndexType));
    check_info(GrB_BinaryOp_new(&IndexType_Mul, Index_Mul, IndexType, IndexType, IndexType));

    GrB_Monoid IndexType_Monoid;
    GrB_Info info = GrB_Monoid_new(&IndexType_Monoid, IndexType_Add, (void *) &Index_Identity);
    check_info(info);

    GrB_Semiring IndexType_Semiring;
    check_info(GrB_Semiring_new(&IndexType_Semiring, IndexType_Monoid, IndexType_Mul));


    // Create matrices
    uint64_t nonterm_count = array_len(grammar->nontermMapper.arr);
//    GrB_Matrix matrices[nonterm_count];
    GrB_Matrix *matrices = malloc(nonterm_count * sizeof(GrB_Matrix));

    for (uint64_t i = 0; i < nonterm_count; ++i) {
        check_info(GrB_Matrix_new(&matrices[i], IndexType, graph_size, graph_size));
    }

    for (int i = 0; i < relations_count; i++) {
        const char *terminal = relations_names[i];
        MapperIndex terminal_id = ItemMapper_GetPlaceIndex((ItemMapper *) &grammar->tokenMapper, terminal);

        if (terminal_id != array_len(grammar->tokenMapper.arr)) {
            for (int j = 0; j < grammar->simple_rules_count; j++) {
                const SimpleRule *simpleRule = &grammar->simple_rules[j];
                if (simpleRule->r == terminal_id) {
                    IndexMatrix_Init(&matrices[simpleRule->l], &relations[i]);
                }
            }
        }
    }

    bool matrices_is_changed = true;
    while(matrices_is_changed) {
        response->iteration_count++;
        matrices_is_changed = false;

        for (int i = 0; i < grammar->complex_rules_count; ++i) {
            MapperIndex nonterm1 = grammar->complex_rules[i].l;
            MapperIndex nonterm2 = grammar->complex_rules[i].r1;
            MapperIndex nonterm3 = grammar->complex_rules[i].r2;

            GrB_Matrix m_old;
            GrB_Matrix_dup(&m_old, matrices[nonterm1]);

            GrB_mxm(matrices[nonterm1], GrB_NULL, IndexType_Add, IndexType_Semiring,
                    matrices[nonterm2], matrices[nonterm3], GrB_NULL);

            GrB_Index nvals_new, nvals_old;
            GrB_Matrix_nvals(&nvals_new, matrices[nonterm1]);
            GrB_Matrix_nvals(&nvals_old, m_old);

            if (nvals_new != nvals_old) {
                matrices_is_changed = true;
            }
//            IndexMatrix_Show(&matrices[0]);

            GrB_Matrix_free(&m_old);
            GrB_free(&m_old);
        }
    }

//    MapperIndex nonterm = ItemMapper_GetPlaceIndex((ItemMapper *) &grammar->nontermMapper, "s");
//    printf("%s %d\n", grammar->nontermMapper.items[nonterm], nonterm);
//    IndexMatrix_Show(&matrices[0]);
//    IndexMatrices_GetPath(matrices, grammar, 0, 7, 0);

    CfpqResponse_Init(response);
    for (int i = 0; i < array_len(grammar->nontermMapper.arr); i++) {
        GrB_Index nvals;
        const char* nonterm;

        GrB_Matrix_nvals(&nvals, matrices[i]) ;
        nonterm = ItemMapper_Map((ItemMapper *) &grammar->nontermMapper, i);
        CfpqResponse_Append(response, nonterm, nvals);

//        GrB_Matrix_free(&matrices[i]) ;
    }
    GrB_Semiring_free(&IndexType_Semiring);
    GrB_Monoid_free(&IndexType_Monoid);

    return matrices;


//    Index index;
//    Index_InitIdentity(&index);
//    GrB_Matrix_extractElement((void *) &index, matrices[nonterm], 31, 31);
//
//    Index_Show(&index);
//    IndexMatrices_GetPath(matrices, grammar, 0, 0, nonterm);
}