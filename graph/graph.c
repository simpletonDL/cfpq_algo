#include <assert.h>
#include "graph.h"

#include "../utils/helpers.h"
#include "../utils/item_mapper.h"
#include "../utils/arr.h"

void GraphRepr_Init(GraphRepr* g) {
	g->max_node_id = 0;
    ItemMapper_Init((ItemMapper*) &g->edges);

    for (int i = 0; i < MAX_GRAPH_RELATION_TYPES; ++i) {
        GrB_Matrix_new(&g->relations[i], GrB_BOOL, MAX_GRAPH_SIZE, MAX_GRAPH_SIZE);
    }
}

void GraphRepr_InsertEdge(GraphRepr* g, int v_id, const char *edge, int to_id) {
    MapperIndex edge_id = ItemMapper_Insert((ItemMapper*) &g->edges, edge);

    assert(v_id != MAX_GRAPH_SIZE && to_id != MAX_GRAPH_SIZE && edge_id != MAX_GRAPH_RELATION_TYPES);

    GrB_Matrix_setElement_BOOL(g->relations[edge_id], true, v_id, to_id);
    g->max_node_id = g->max_node_id < v_id ? v_id : g->max_node_id;
    g->max_node_id = g->max_node_id < to_id ? to_id : g->max_node_id;
}

void GraphRepr_Load(GraphRepr *g, FILE *f) {
    GraphRepr_Init(g);

	char v[MAX_ITEM_NAME_LEN], edge[MAX_ITEM_NAME_LEN], to[MAX_ITEM_NAME_LEN];
	char *line_buf;
	size_t buf_size = 0;

    int i = 0;
    while (getline(&line_buf, &buf_size, f) != -1) {
        i++;
        if (i % 10000 == 0) {
            printf("Load %d\n", i);
            fflush(stdout);
        }
        str_strip(line_buf);

        int nitems = sscanf(line_buf, "%s %s %s", v, edge, to);

        char *end;
        int v_id = strtol(v, &end, 10);
        int to_id = strtol(to, &end, 10);
        assert(nitems == 3);

        GraphRepr_InsertEdge(g, v_id, edge, to_id);
    }
}
