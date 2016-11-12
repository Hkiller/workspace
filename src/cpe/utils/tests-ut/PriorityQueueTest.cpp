#include "cpe/utils/stream_buffer.h"
#include "PriorityQueueTest.hpp"

void PriorityQueueTest::SetUp() {
    Base::SetUp();
    t_em_set_print();
    m_queue = cpe_priority_queue_create(t_allocrator(), t_em(), sizeof(int32_t), ele_cmp, 0);
    ASSERT_TRUE(m_queue);
}

void PriorityQueueTest::TearDown() {
    cpe_priority_queue_free(m_queue);
    m_queue = NULL;
    Base::TearDown();
}

void PriorityQueueTest::insert(int32_t v) {
    int r = cpe_priority_queue_insert(m_queue, &v);
    ASSERT_EQ(0, r);
}

int32_t PriorityQueueTest::top(void) {
    int32_t * r = (int32_t*)cpe_priority_queue_top(m_queue);
    if (r == NULL) return -1;
    return *r;
}

uint16_t PriorityQueueTest::count(void) {
    return cpe_priority_queue_count(m_queue);
}

const char * PriorityQueueTest::dump(void) {
    struct mem_buffer dump_buffer;

    mem_buffer_init(&dump_buffer, t_tmp_allocrator());

    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&dump_buffer);

    cpe_priority_queue_dump((write_stream_t)&ws, m_queue, ele_dump, " ");

    mem_buffer_append_char(&dump_buffer, 0);

    return (char *)mem_buffer_make_continuous(&dump_buffer, 0);
}

const char * PriorityQueueTest::dump_dequeue(void) {
    struct mem_buffer dump_buffer;
    
    mem_buffer_init(&dump_buffer, t_tmp_allocrator());

    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&dump_buffer);

    int i = 0;
    while(void * p = cpe_priority_queue_top(m_queue)) {
        if (i > 0) stream_printf((write_stream_t)&ws, "%s", " ");
        ele_dump((write_stream_t)&ws, p);
        i++;
        cpe_priority_queue_pop(m_queue);
    }

    mem_buffer_append_char(&dump_buffer, 0);

    return (char *)mem_buffer_make_continuous(&dump_buffer, 0);
}

int PriorityQueueTest::ele_cmp(void const * l, void const * r) {
    return (*(int32_t*)l) - (*(int32_t*)r);
}

void PriorityQueueTest::ele_dump(write_stream_t s, void const * e) {
    stream_printf(s, "%d", *(int32_t*)e);
}
