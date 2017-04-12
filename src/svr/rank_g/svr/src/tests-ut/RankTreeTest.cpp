#include "cpepp/utils/MemBuffer.hpp"
#include "RankTreeTest.hpp"

void RankTreeTest::SetUp() {
    Base::SetUp();

    t_rank_g_svr_create();

    m_rank_tree = NULL;
}

void RankTreeTest::TearDown() {
    if (m_rank_tree) {
        rt_free(m_rank_tree);
        m_rank_tree = NULL;
    }

    Base::TearDown();
}

void RankTreeTest::t_rank_tree_create(uint32_t node_count) {
    if (m_rank_tree) {
        rt_free(m_rank_tree);
        m_rank_tree = NULL;
    }

    size_t capacity = rt_buff_calc_capacity(node_count);
    void * buff = t_tmp_alloc(capacity);
    ASSERT_TRUE(buff != NULL);
    
    ASSERT_EQ(0, rt_buff_init(t_em(), node_count, buff, capacity));

    m_rank_tree = rt_create(rank_g_svr(), buff, capacity);
    ASSERT_TRUE(m_rank_tree != NULL);
}

void RankTreeTest::t_rank_tree_install(uint32_t const * values, uint32_t value_count) {
    ASSERT_TRUE(m_rank_tree);

    uint32_t max_record_id = rt_size(m_rank_tree);
    for(uint32_t i = 0; i < value_count; ++i) {
        rt_insert(m_rank_tree, values[i], ++max_record_id);
    }
}

void RankTreeTest::t_rank_tree_erase(rt_node_t node) {
    rt_erase(m_rank_tree, node);
}

const char * RankTreeTest::range(uint32_t value) {
    rt_node_t node_min = rt_find_by_value_min(m_rank_tree, value);
    rt_node_t node_max = rt_find_by_value_max(m_rank_tree, value);

    uint8_t i = 0;

    size_t capacity = 1024;
    char * r = (char*)t_tmp_alloc(capacity);
    size_t n = 0;
    r[0] = 0;
    
    while(node_min && node_min != node_max) {
        if (i > 0) {
            n += snprintf(r + n, capacity - n, " ");
        }
        n += snprintf(r + n, capacity - n, "%d", node_min->m_record_id);
        node_min = rt_next(m_rank_tree, node_min);
        i++;
    }

    return r;
}

const char * RankTreeTest::range() {
    rt_node_t node = rt_first(m_rank_tree);

    uint8_t i = 0;
    size_t capacity = 1024;
    char * r = (char*)t_tmp_alloc(capacity);
    size_t n = 0;
    r[0] = 0;
    
    while(node) {
        if (i > 0) {
            n += snprintf(r + n, capacity - n, " ");
        }
        n += snprintf(r + n, capacity - n, "%d", node->m_record_id);
        node = rt_next(m_rank_tree, node);
        i++;
    }

    return r;
}

const char * RankTreeTest::values() {
    rt_node_t node = rt_first(m_rank_tree);

    uint8_t i = 0;
    size_t capacity = 1024;
    char * r = (char*)t_tmp_alloc(capacity);
    size_t n = 0;
    r[0] = 0;
    
    while(node) {
        if (i > 0) {
            n += snprintf(r + n, capacity - n, " ");
        }
        n += snprintf(r + n, capacity - n, "%d", node->m_value);
        node = rt_next(m_rank_tree, node);
        i++;
    }

    return r;
}

const char * RankTreeTest::t_rank_tree_dump(void) {
    Cpe::Utils::MemBuffer buf(t_tmp_allocrator());
    return rt_dump(m_rank_tree, buf);
}

rt_node_t RankTreeTest::find_min(uint32_t value) {
    return rt_find_by_value_min(m_rank_tree, value);
}

rt_node_t RankTreeTest::find_max(uint32_t value) {
    return rt_find_by_value_max(m_rank_tree, value);
}

rt_node_t RankTreeTest::get_at(uint32_t pos) {
    rt_node_t node = rt_first(m_rank_tree);

    while(node && pos > 0) {
        node = rt_next(m_rank_tree, node);
        pos--;
    }

    return node;
}

int RankTreeTest::t_rank_tree_error_pos(void) {
    rt_node_t node, next;
    int pos =0;
    
    for(node = rt_first(m_rank_tree); node; node = next, pos++) {
        next = rt_next(m_rank_tree, node);
        if (next == NULL) break;

        if (node->m_value < next->m_value) return pos;
    }

    return -1;
}
