#include "algorithms.h"

#define BIT_EXIST(x, pos) (((x) >> (pos)) & 1ul)
#define BIT_SET(x, pos) ((x) |= (1ul << (pos)))

const Grammar* bin_op_grammar;
uint8_t left_shift_count = 0;
uint8_t right_shift_count = 0;

void set_multiply_impl(void *z_void_ptr, const void *x_void_ptr, const void *y_void_ptr) {
    // Implement z = x * y. Type of z, x, y must be uint8_t.
    uint8_t z = (uint8_t) 0;
    uint8_t x = *((uint8_t *) x_void_ptr);
    uint8_t y = *((uint8_t *) y_void_ptr);

    for (int i = 0; i < bin_op_grammar->complex_rules_count; ++i) {
        if (BIT_EXIST(x, bin_op_grammar->complex_rules[i].r1) &
            BIT_EXIST(y, bin_op_grammar->complex_rules[i].r2)) {
            BIT_SET(z, bin_op_grammar->complex_rules[i].l);
        }
    }

    uint8_t *z_ptr = z_void_ptr;
    *z_ptr = z;
}

void set_union(void *z_void_ptr, const void *x_void_ptr, const void *y_void_ptr) {
    uint8_t z = (*((uint8_t *) x_void_ptr)) | (*((uint8_t *) y_void_ptr));
    uint8_t *z_ptr = z_void_ptr;
    *z_ptr = z;
}

void left_shift(void* z_void_ptr, const void *x_void_ptr) {
    // z_void_ptr is pointer to uint8_t, x_void_ptr is pointer to bool value
    uint8_t *z_ptr = z_void_ptr;
    *z_ptr = 1ul << left_shift_count;
}

bool right_shift(const GrB_Index i, const GrB_Index j, const GrB_Index nrows, const GrB_Index ncols, const void *x, const void *thunk) {
    const uint8_t *z_ptr = x;
    if (BIT_EXIST(*z_ptr, right_shift_count) & 1ul)
        return true;
    return false;
}

bool uint8_matrix_equal(GrB_Matrix A, GrB_Matrix B) {
    // TODO: replace it by LaGraph function
    bool res;

    GrB_Index n, m;
    GrB_Matrix_nrows(&n, A);
    GrB_Matrix_ncols(&m, A);

    GrB_Matrix C;
    GrB_Matrix_new(&C, GrB_BOOL, n, m);
    GrB_eWiseMult(C, NULL, NULL, GrB_EQ_UINT8, A, B, NULL);

    GrB_Index A_nvals, B_nvals, C_nvals;
    GrB_Matrix_nvals(&A_nvals, A);
    GrB_Matrix_nvals(&B_nvals, B);
    GrB_Matrix_nvals(&C_nvals, C);

    if (A_nvals != C_nvals || B_nvals != C_nvals) {
        return false;
    }

    GrB_Monoid monoid;
    GrB_Monoid_new_BOOL(&monoid, GrB_LAND, true);
    GrB_reduce(&res, NULL, monoid, C, NULL);
    return res;
}

int cfpq_cpu_2(const Grammar *grammar, CfpqResponse *response,
               const GrB_Matrix *relations, const char relations_names[MAX_GRAPH_RELATION_TYPES][MAX_ITEM_NAME_LEN],
               size_t relations_count, size_t graph_size) {
    // Initialize global pointer for GrB_SetMultiply operator
    bin_op_grammar = grammar;

    // Create binary and unary operators
    GrB_BinaryOp GrB_SetMultiply;
    GrB_BinaryOp GrB_SetUnion;
    GrB_UnaryOp GrB_LeftShift;

    GrB_BinaryOp_new(&GrB_SetMultiply, set_multiply_impl, GrB_UINT8, GrB_UINT8, GrB_UINT8);
    GrB_BinaryOp_new(&GrB_SetUnion, set_union, GrB_UINT8, GrB_UINT8, GrB_UINT8);
    GrB_UnaryOp_new(&GrB_LeftShift, left_shift, GrB_UINT8, GrB_BOOL);

    // Create Monoid, Semiring and Matrix type
    GrB_Monoid monoid;
    GrB_Monoid_new_UINT8(&monoid, GrB_SetUnion, 0);

    GrB_Semiring semiring;
    GrB_Semiring_new(&semiring, monoid, GrB_SetMultiply);

    // Create and initialize matrix
    GrB_Matrix matrix;
    GrB_Matrix_new(&matrix, GrB_UINT8, graph_size, graph_size);

    for (int i = 0; i < relations_count; i++) {
        char terminal[MAX_ITEM_NAME_LEN];
        strcpy(terminal, relations_names[i]);

        MapperIndex terminal_id = ItemMapper_GetPlaceIndex((ItemMapper *) &grammar->tokenMapper, terminal);
        if (terminal_id != grammar->tokenMapper.count) {
            for (int j = 0; j < grammar->simple_rules_count; j++) {
                const SimpleRule *simple_rule = &grammar->simple_rules[j];
                if (simple_rule->r == terminal_id) {
                    GrB_Matrix shifted_relation;
                    GrB_Matrix_new(&shifted_relation, GrB_UINT8, graph_size, graph_size);

                    left_shift_count = simple_rule->l;
                    GrB_Matrix_apply(shifted_relation, NULL, NULL, GrB_LeftShift, relations[i], NULL);
                    GrB_eWiseAdd_Matrix_BinaryOp(matrix, NULL, NULL, GrB_SetUnion, matrix, shifted_relation, NULL);
                }
            }
        }
    }

    // Mega-super-puper algorithm
    while (true) {
        response->iteration_count++;

        GrB_Matrix matrix_old, matrix_new;
        GrB_Matrix_new(&matrix_old, GrB_UINT8, graph_size, graph_size);
        GrB_Matrix_new(&matrix_new, GrB_UINT8, graph_size, graph_size);

        GrB_Matrix_dup(&matrix_old, matrix);

#ifdef DEBUG
        printf("-------------------------------------");
        printf("before mul:\n");
        GxB_print(matrix_old, GxB_COMPLETE);
#endif

        GrB_mxm(matrix_new, NULL, NULL, semiring, matrix, matrix_old, NULL);
#ifdef DEBUG
        printf("after mul:\n");
        GxB_print(matrix_new, GxB_COMPLETE);
#endif

        GxB_select(matrix_new, NULL, NULL, GxB_NONZERO, matrix_new, NULL, NULL);
#ifdef DEBUG
        printf("after select:\n");
        GxB_print(matrix_new, GxB_COMPLETE);
#endif

        GrB_eWiseAdd_Matrix_Monoid(matrix, NULL, NULL, monoid, matrix_new, matrix_old, NULL);
        GxB_select(matrix, NULL, NULL, GxB_NONZERO, matrix, NULL, NULL);
#ifdef DEBUG
        printf("after add new to old:\n");
        GxB_print(matrix, GxB_COMPLETE);
        printf("-------------------------------------");
#endif

        if (uint8_matrix_equal(matrix_old, matrix)) {
            break;
        }
    }

    GxB_SelectOp NontermSelector;
    GxB_SelectOp_new(&NontermSelector, right_shift, GrB_UINT8, NULL);
    for (int i = 0; i < grammar->nontermMapper.count; ++i) {
        right_shift_count = i;

        GrB_Matrix nonterm_matrix;
        GrB_Matrix_new(&nonterm_matrix, GrB_UINT8, MAX_GRAPH_SIZE, MAX_GRAPH_SIZE);
        GxB_select(nonterm_matrix, NULL, NULL, NontermSelector, matrix, NULL, NULL);

        GrB_Index nonterm_count;
        GrB_Matrix_nvals(&nonterm_count, nonterm_matrix);

        CfpqResponse_Append(response, grammar->nontermMapper.items[i], nonterm_count);
    }
    return 0;
}