#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "GraphTest.hpp"

void GraphTest::SetUp() {
    Base::SetUp();
    t_em_set_print();
    m_graph = cpe_graph_create(t_allocrator(), t_em(), sizeof(node), sizeof(edge));
    ASSERT_TRUE(m_graph);
    cpe_graph_set_print_fun(m_graph, node_dump, edge_dump);
    
    m_weight_path = NULL;
}

void GraphTest::TearDown() {
    cpe_graph_free(m_graph);
    m_graph = NULL;

    if (m_weight_path) {
        cpe_graph_weight_path_free(m_weight_path);
        m_weight_path = NULL;
    }
    
    Base::TearDown();
}

uint16_t GraphTest::node_count(void) {
    return cpe_graph_node_count(m_graph);
}

uint16_t GraphTest::edge_count(void) {
    return cpe_graph_edge_count(m_graph);
}

int GraphTest::shortest_path_dijkstra(const char * start, const char * finish) {
    node * start_node = find_node(start);
    EXPECT_TRUE(start_node) << "shortest_path_dijkstra: start node " << start << " not exist!";
    if (start_node == NULL) return -1;

    node * finish_node = find_node(finish);
    EXPECT_TRUE(finish_node) << "shortest_path_dijkstra: finish node " << finish << " not exist!";
    if (finish_node == NULL) return -1;

    if (m_weight_path == NULL) {
        m_weight_path = cpe_graph_weight_path_create(t_allocrator());
        EXPECT_TRUE(m_weight_path) << "shortest_path_dijkstra: create weight path fail";
        if (m_weight_path == NULL) return -1;
    }
    else {
        cpe_graph_weight_path_clear(m_weight_path);
    }
    
    int rv = cpe_graph_shortest_path_dijkstra(
        m_weight_path,
        m_graph, cpe_graph_node_from_data(start_node), edge_weight,
        cpe_graph_node_from_data(finish_node), NULL);
    EXPECT_EQ(0, rv) << "shortest_path_dijkstra fail!";
    return rv;
}

const char * GraphTest::dump(void) {
    struct mem_buffer dump_buffer;

    mem_buffer_init(&dump_buffer, t_tmp_allocrator());

    // struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&dump_buffer);

    //cpe_priority_graph_dump((write_stream_t)&ws, m_graph, ele_dump, " ");

    mem_buffer_append_char(&dump_buffer, 0);

    return (char *)mem_buffer_make_continuous(&dump_buffer, 0);
}

const char * GraphTest::dump_weight_path(void) {
    struct mem_buffer dump_buffer;

    mem_buffer_init(&dump_buffer, t_tmp_allocrator());

    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&dump_buffer);

    assert(m_weight_path);
    cpe_graph_weight_path_print((write_stream_t)&ws, m_weight_path);

    mem_buffer_append_char(&dump_buffer, 0);

    return (char *)mem_buffer_make_continuous(&dump_buffer, 0);
}

GraphTest::node *
GraphTest::find_node(const char * name) {
    cpe_graph_node_it node_it;
    cpe_graph_nodes(&node_it, m_graph);

    while(cpe_graph_node_t g_node = cpe_graph_node_it_next(&node_it)) {
        node * node_data = (node*)cpe_graph_node_data(g_node);
        if (strcmp(node_data->m_name, name) == 0) return node_data;
    }

    return NULL;
}

GraphTest::node * GraphTest::add_node(const char * name) {
    cpe_graph_node_t n = cpe_graph_node_create(m_graph);
    EXPECT_TRUE(n)  << "create node fail!";
    if (n == NULL) return NULL;
    
    node * node_data = (node*)cpe_graph_node_data(n);

    cpe_str_dup(node_data->m_name, sizeof(node_data->m_name), name);
    
    return node_data;
}

void GraphTest::add_nodes(const char * def) {
    def = cpe_str_trim_head((char *)def);
    
    while(const char * p = strchr(def, ' ')) {
        char name_buf[64];
        size_t len = p - def;
        memcpy(name_buf, def, len);
        name_buf[len] = 0;

        add_node(name_buf);
        
        def = cpe_str_trim_head((char *)(p + 1));
    }

    if (def[0]) {
        add_node(def);
    }
}

GraphTest::edge * GraphTest::add_edge(const char * a, const char * b) {
    node * a_node = find_node(a);
    node * b_node = find_node(b);

    EXPECT_TRUE(a_node) << "add_edge: node " << a << " not exist!";
    if (a_node == NULL) return NULL;

    EXPECT_TRUE(b_node) << "add_edge: node " << b << " not exist!";
    if (b_node == NULL) return NULL;
    
    cpe_graph_edge_t n = cpe_graph_edge_create(cpe_graph_node_from_data(a_node), cpe_graph_node_from_data(b_node));
    EXPECT_TRUE(n) << "create edge fail!";
    if (n == NULL) return NULL;
    
    edge * edge_data = (edge*)cpe_graph_edge_data(n);

    bzero(edge_data, sizeof(*edge_data));
    
    return edge_data;
}

GraphTest::edge * GraphTest::add_edge(const char * a, const char * b, int16_t weight) {
    edge * e = add_edge(a, b);
    if (e) e->m_weight = weight;
    return e;
}

void GraphTest::node_dump(write_stream_t s, cpe_graph_node_t n) {
    stream_printf(s, "%s", ((node*)cpe_graph_node_data(n))->m_name);
}

void GraphTest::edge_dump(write_stream_t s, cpe_graph_edge_t e) {
    stream_printf(s, "%d", ((edge*)cpe_graph_edge_data(e))->m_weight);
}

float GraphTest::edge_weight(cpe_graph_edge_t e) {
    return ((edge*)cpe_graph_edge_data(e))->m_weight;
}
