#include <assert.h>
#include "graph/graph.h"
#include "cfpq_algorithms/algorithms.h"
#include "cfpq_algorithms/response.h"
#include "utils/simple_timer.h"

const char *GRAPH_INPUT_FILE = "input/graph/pizza.txt";
const char *GRAMMAR_INPUT_FILE = "input/grammar/GPPerf1_cnf.txt";


int main() {
    double timer[2];

    GrB_init(GrB_NONBLOCKING);

    FILE* f = fopen(GRAPH_INPUT_FILE, "r");
    assert(f != NULL);

    GraphRepr g;

    GraphRepr_Load(&g, f);

    fclose(f);

    f = fopen(GRAMMAR_INPUT_FILE, "r");
    assert(f != NULL);

    Grammar grammar;
    Grammar_Load(&grammar, f);
    fclose(f);

    CfpqResponse response;

    simple_tic(timer);
    cfpq_cpu_2(&grammar, &response, g.relations, g.edges.items, g.edges.count, MAX_GRAPH_SIZE);
    double time_query = simple_toc(timer);

    printf("time query: %f\n", time_query);
    printf("iteration count: %d\n", response.iteration_count);
    printf("Control sums:\n");
    for (int i = 0; i < response.count; ++i) {
        printf("%s: %lu\n", response.nonterms[i], response.control_sums[i]);
    }
}