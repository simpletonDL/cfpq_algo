//------------------------------------------------------------------------------
// GB_sel:  hard-coded functions for selection operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If this file is in the Generated/ folder, do not edit it (auto-generated).

#include "GB_select.h"
#include "GB_ek_slice.h"
#include "GB_sel__include.h"

// The selection is defined by the following types and operators:

// phase1: GB_sel_phase1__gt_zero_int8
// phase2: GB_sel_phase2__gt_zero_int8

// A type:   int8_t
// selectop: (Ax [p] > 0)

// kind
#define GB_ENTRY_SELECTOR

#define GB_ATYPE \
    int8_t

// test Ax [p]
#define GB_SELECT(p)                                    \
    (Ax [p] > 0)

// get the vector index (user select operators only)
#define GB_GET_J                                        \
    ;

// workspace is a parameter to the function, not defined internally
#define GB_REDUCTION_WORKSPACE(W, ntasks) ;

// W [k] = s, no typecast
#define GB_COPY_SCALAR_TO_ARRAY(W,k,s)                  \
    W [k] = s

// W [k] = S [i], no typecast
#define GB_COPY_ARRAY_TO_ARRAY(W,k,S,i)                 \
    W [k] = S [i]

// W [k] += S [i], no typecast
#define GB_ADD_ARRAY_TO_ARRAY(W,k,S,i)                  \
    W [k] += S [i]

// no terminal value
#define GB_BREAK_IF_TERMINAL(t) ;

// ztype s = (ztype) Ax [p], with typecast
#define GB_CAST_ARRAY_TO_SCALAR(s,Ax,p)                 \
    s = GB_SELECT (p)

// s += (ztype) Ax [p], with typecast
#define GB_ADD_CAST_ARRAY_TO_SCALAR(s,Ax,p)             \
    s += GB_SELECT (p)

// Cx [pC] = Ax [pA], no typecast
#define GB_SELECT_ENTRY(Cx,pC,Ax,pA)                    \
    Cx [pC] = Ax [pA]

// declare scalar for GB_reduce_each_vector
#define GB_SCALAR(s)                                    \
    int64_t s

//------------------------------------------------------------------------------
// GB_sel_phase1__gt_zero_int8
//------------------------------------------------------------------------------



void GB_sel_phase1__gt_zero_int8
(
    int64_t *restrict Zp,
    int64_t *restrict Cp,
    int64_t *restrict Wfirst,
    int64_t *restrict Wlast,
    const GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *restrict xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
)
{ 
    int64_t *restrict Tx = Cp ;
    ;
    #include "GB_select_phase1.c"
}



//------------------------------------------------------------------------------
// GB_sel_phase2__gt_zero_int8
//------------------------------------------------------------------------------

void GB_sel_phase2__gt_zero_int8
(
    int64_t *restrict Ci,
    int8_t *restrict Cx,
    const int64_t *restrict Zp,
    const int64_t *restrict Cp,
    const int64_t *restrict C_pstart_slice,
    const GrB_Matrix A,
    const int64_t *restrict kfirst_slice,
    const int64_t *restrict klast_slice,
    const int64_t *restrict pstart_slice,
    const bool flipij,
    const int64_t ithunk,
    const int8_t *restrict xthunk,
    const GxB_select_function user_select,
    const int ntasks,
    const int nthreads
)
{ 
    ;
    #include "GB_select_phase2.c"
}

