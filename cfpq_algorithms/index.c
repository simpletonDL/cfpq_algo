#include <stdio.h>
#include <assert.h>

#include "index.h"
#include "../utils/arr.h"

Index Index_Identity = {
        .left = 0,
        .right = 0,
        .middle = 0,
        .height = 0,
        .length = 0,
};

// Identity = невыводимый путь

void Index_Init(Index *index, GrB_Index left, GrB_Index right, GrB_Index middle, uint32_t height, uint32_t length) {
    index->left = left;
    index->right = right;
    index->middle = middle;
    index->height = height;
    index->length = length;
}

void Index_InitIdentity(Index *index) {
    index->left = 0;
    index->right = 0;
    index->middle = 0;
    index->height = 0;
    index->length = 0;
}

bool Index_IsIdentity(const Index *index) {
    return index->left == 0  &&
           index->right == 0 &&
           index->middle == 0 &&
           index->height == 0 &&
           index->length == 0;
}

void Index_Copy(const Index *from, Index *to) {
    to->left = from->left;
    to->right = from->right;
    to->middle = from->middle;
    to->height = from->height;
    to->length = from->length;
}

void Index_Mul(void *z, const void *x, const void *y) {
    Index *left = (Index *) x;
    Index *right = (Index *) y;
    Index *res = (Index *) z;

    char buf1[30], buf2[30];
    Index_ToStr(left, buf1);
    Index_ToStr(right, buf2);

    if (Index_IsIdentity(left) || Index_IsIdentity(right)) {
        Index_InitIdentity(res);
    } else {
        uint32_t height = (left->height < right->height ? right->height : left->height) + 1;
        Index_Init(res, left->left, right->right, left->right,
                   height, left->length + right->length);
    }
}

void Index_Add(void *z, const void *x, const void *y) {
    const Index *left = (const Index *) x;
    const Index *right = (const Index *) y;
    Index *res = (Index *) z;

    if (Index_IsIdentity(left) && Index_IsIdentity(right)) {
        Index_InitIdentity(res);
    } else if (Index_IsIdentity(left)) {
        Index_Copy(right, res);
    } else if (Index_IsIdentity(right)) {
        Index_Copy(left, res);
    } else {
        const Index *min_height_index = (left->height < right->height) ? left : right;
        Index_Copy(min_height_index, res);
    }
}


void Index_ToStr(const Index *index, char *buf) {
    if (Index_IsIdentity(index)) {
        sprintf(buf, "(     Identity      )");
    } else {
        sprintf(buf, "(i:%lu,j:%lu,k:%lu,h=%d,l=%d)",
                index->left, index->right, index->middle, index->height, index->length);
    }
}


void IndexMatrix_Init(GrB_Matrix *m, const GrB_Matrix *bool_m) {
    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, *bool_m);

    GrB_Index *I = malloc(nvals * sizeof(GrB_Index));
    GrB_Index *J = malloc(nvals * sizeof(GrB_Index));
    bool *X = malloc(nvals * sizeof(bool));

    GrB_Matrix_extractTuples_BOOL(I, J, X, &nvals, *bool_m);
    for (int k = 0; k < nvals; ++k) {
        Index index;
        Index_Init(&index, I[k], J[k], 0, 1, 1);
        GrB_Matrix_setElement(*m, (void *) &index, I[k], J[k]);
    }

    free(I);
    free(J);
    free(X);
}

void Index_Show(Index *index) {
    char buf[100];
    Index_ToStr(index, buf);
    printf("%s", buf);
}


void IndexMatrix_Show(const GrB_Matrix *matrix) {
    GrB_Index n, m;
    GrB_Matrix_nrows(&n, *matrix);
    GrB_Matrix_ncols(&m, *matrix);


    for (GrB_Index i = 0; i < n; i++) {
        for (GrB_Index j = 0; j < m; j++) {
            char buf[50];
            Index index;
            Index_InitIdentity(&index);

            GrB_Matrix_extractElement((void *) &index, *matrix, i, j);
            Index_ToStr(&index, buf);
            printf("i: %lu, j: %lu, index: %s\n", i, j, buf);
        }
        printf("\n");
    }
}

void _IndexMatrices_GetPath(GrB_Index *arr, const GrB_Matrix *matrices, const Grammar *grammar, GrB_Index left, GrB_Index right, MapperIndex nonterm) {
    Index index;
    Index_InitIdentity(&index);
    GrB_Matrix_extractElement((void *) &index, matrices[nonterm], left, right);

    assert(!Index_IsIdentity(&index) && "Index isn`t correct\n");



    if (index.height == 1) {
//        printf("(%lu-[:%s]->%lu)", index.left, grammar->nontermMapper.items[nonterm], index.right);
        array_append(arr, index.left);
        return;
    }

    for (GrB_Index i = 0; i < grammar->complex_rules_count; ++i) {
        MapperIndex nonterm_l = grammar->complex_rules[i].l;
        MapperIndex nonterm_r1 = grammar->complex_rules[i].r1;
        MapperIndex nonterm_r2 = grammar->complex_rules[i].r2;

        if (nonterm == nonterm_l) {
            Index index_r1, index_r2;
            Index_InitIdentity(&index_r1);
            Index_InitIdentity(&index_r2);
            GrB_Matrix_extractElement((void *) &index_r1, matrices[nonterm_r1], left, index.middle);
            GrB_Matrix_extractElement((void *) &index_r2, matrices[nonterm_r2], index.middle, right);

            if (!Index_IsIdentity(&index_r1) && !Index_IsIdentity(&index_r2)) {
                uint32_t max_height = index_r1.height < index_r2.height ? index_r2.height : index_r1.height;
                if (index.height == max_height + 1) {
                    _IndexMatrices_GetPath(arr, matrices, grammar, left, index.middle, nonterm_r1);
                    _IndexMatrices_GetPath(arr, matrices, grammar, index.middle, right, nonterm_r2);
                    break;
                }
            }
        }
    }
}

GrB_Index * IndexMatrices_GetPath(const GrB_Matrix *matrices, const Grammar *grammar, GrB_Index left, GrB_Index right, MapperIndex nonterm) {

    Index index;
    Index_InitIdentity(&index);
    GrB_Matrix_extractElement((void *) &index, matrices[nonterm], left, right);


    if (Index_IsIdentity(&index)) {
        printf("Path doesnt`t exist\n");
        return NULL;
    }

    GrB_Index *arr = array_new(GrB_Index, index.length + 1);
    _IndexMatrices_GetPath(arr, matrices, grammar, left, right, nonterm);
    array_append(arr, index.right);
    return arr;
}
