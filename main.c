#include <assert.h>
#include "graph/graph.h"
#include "cfpq_algorithms/algorithms.h"
#include "cfpq_algorithms/response.h"
#include "utils/simple_timer.h"
//#include "util/arr.h"
//#include "cfpq_algorithms/index.h"

const char *GRAPH_INPUT_FILE = "input/graph/worstcase_4.txt";
const char *GRAMMAR_INPUT_FILE = "input/grammar/Brackets.txt";


int main() {
    // Initialize GraphBLAS
    GrB_init(GrB_NONBLOCKING);

    // Load graph
    FILE* f = fopen(GRAPH_INPUT_FILE, "r");
    assert(f != NULL);

    GraphRepr g;
    GraphRepr_Load(&g, f);
    fclose(f);

    // Load grammar
    f = fopen(GRAMMAR_INPUT_FILE, "r");
    assert(f != NULL);

    Grammar grammar;
    Grammar_Load(&grammar, f);
    fclose(f);

    // Start algorithm
    CfpqResponse response;

    double timer[2];
    simple_tic(timer);
    cfpq_cpu_1(&grammar, &response, g.relations, g.edges.items, g.edges.count, MAX_GRAPH_SIZE);
    double time_query = simple_toc(timer);

    // Output results
    printf("time query: %f\n", time_query);
    printf("iteration count: %d\n", response.iteration_count);
    printf("Control sums:\n");
    for (int i = 0; i < response.count; ++i) {
        printf("%s: %lu\n", response.nonterms[i], response.control_sums[i]);
    }
}