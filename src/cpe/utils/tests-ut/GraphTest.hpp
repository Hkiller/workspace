#ifndef CPE_DR_TEST_GRAPH_H
#define CPE_DR_TEST_GRAPH_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/utils/graph.h"
#include "cpe/utils/graph_weight_path.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) GraphTestBase;

class GraphTest : public testenv::fixture<GraphTestBase> {
public:
    struct node {
        char m_name[64];
    };

    struct edge {
        int16_t m_weight;
    };
    
    virtual void SetUp();
    virtual void TearDown();

    node * find_node(const char * name);
    node * add_node(const char * name);
    edge * add_edge(const char * a, const char * b);
    edge * add_edge(const char * a, const char * b, int16_t weight);    

    void add_nodes(const char * def);
    
    uint16_t edge_count(void);
    uint16_t node_count(void);

    int shortest_path_dijkstra(const char * start, const char * finish);
    
    const char * dump(void);
    const char * dump_weight_path(void);

    cpe_graph_t m_graph;
    cpe_graph_weight_path_t m_weight_path;

    static void node_dump(write_stream_t s, cpe_graph_node_t node);
    static void edge_dump(write_stream_t s, cpe_graph_edge_t edge);
    static float edge_weight(cpe_graph_edge_t edge);
};

#endif
