#pragma once

#include "conf.h"
#include <stdio.h>

#define GRAMMAR_LOAD_ERROR 0
#define GRAMMAR_LOAD_SUCCESS 1

typedef struct {
    const char **arr;
} NontermMapper;

typedef struct {
	const char **arr;
} TokenMapper;

typedef struct {
    MapperIndex l;
    MapperIndex r1;
    MapperIndex r2;
} ComplexRule;

typedef struct {
    MapperIndex l;
    MapperIndex r;
} SimpleRule;

typedef struct {
	// S -> A B
	ComplexRule complex_rules[MAX_GRAMMAR_SIZE]; // [{0, 1, 2}, {3, 4, 5}]
    int complex_rules_count;

    // A -> a
    SimpleRule simple_rules[MAX_GRAMMAR_SIZE]; // [{0, 1}, {2, 3}]
    int simple_rules_count;

    //
    NontermMapper nontermMapper;
    TokenMapper tokenMapper;
} Grammar;

int Grammar_Load(Grammar *gr, FILE *f);
void Grammar_Init(Grammar *gr);

void Grammar_AddSimpleRule(Grammar *gr, MapperIndex l, MapperIndex r);
void Grammar_AddComplexRule(Grammar *gr, MapperIndex l, MapperIndex r1, MapperIndex r2);
