#include <assert.h>
#include "graph.h"

#include "../utils/helpers.h"
#include "../utils/item_mapper.h"

void GraphRepr_Init(GraphRepr* g) {
    ItemMapper_Init((ItemMapper*) &g->nodes);
    ItemMapper_Init((ItemMapper*) &g->edges);

    for (int i = 0; i < MAX_GRAPH_RELATION_TYPES; ++i) {
        GrB_Matrix_new(&g->relations[i], GrB_BOOL, MAX_GRAPH_SIZE, MAX_GRAPH_SIZE);
    }
}

void GraphRepr_InsertEdge(GraphRepr* g, const char *v, const char *edge, const char *to) {
    MapperIndex v_id = ItemMapper_Insert((ItemMapper*) &g->nodes, v);
    MapperIndex to_id = ItemMapper_Insert((ItemMapper*) &g->nodes, to);
    MapperIndex edge_id = ItemMapper_Insert((ItemMapper*) &g->edges, edge);

    assert(v_id != MAX_GRAPH_SIZE && to_id != MAX_GRAPH_SIZE && edge_id != MAX_GRAPH_RELATION_TYPES);

    GrB_Matrix_setElement_BOOL(g->relations[edge_id], true, v_id, to_id);
}

void GraphRepr_Load(GraphRepr *g, FILE *f) {
    GraphRepr_Init(g);

    char *line_buf;
    size_t buf_size = 0;

    ItemMapper nodes, edges;
    ItemMapper_Init(&nodes);
    ItemMapper_Init(&edges);

    while (getline(&line_buf, &buf_size, f) != -1) {
        str_strip(line_buf);

        char v[MAX_ITEM_NAME_LEN], edge[MAX_ITEM_NAME_LEN], to[MAX_ITEM_NAME_LEN];
        int nitems = sscanf(line_buf, "%s %s %s", v, edge, to);
        assert(nitems == 3);

        GraphRepr_InsertEdge(g, v, edge, to);
    }
}
