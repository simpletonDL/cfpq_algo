#pragma once

#include "../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../grammar/grammar.h"

typedef struct {
    GrB_Index left;
    GrB_Index right;
    GrB_Index middle;

    uint32_t height;
    uint32_t length;
} Index;

extern Index Index_Identity;

void Index_Init(Index *index, GrB_Index left, GrB_Index right, GrB_Index middle, uint32_t height, uint32_t length);
void Index_InitIdentity(Index *index);
void Index_Copy(const Index *from, Index *to);

bool Index_IsIdentity(const Index *index);

void Index_Mul(void *z, const void *x, const void *y);
void Index_Add(void *z, const void *x, const void *y);

void Index_ToStr(const Index *index, char *buf);
void Index_Show(Index *index);


void IndexMatrix_Init(GrB_Matrix *m, const GrB_Matrix *bool_m);
void IndexMatrix_Show(const GrB_Matrix *matrix);
GrB_Index * IndexMatrices_GetPath(const GrB_Matrix *matrices, const Grammar *grammar, GrB_Index left, GrB_Index right, MapperIndex nonterm);
