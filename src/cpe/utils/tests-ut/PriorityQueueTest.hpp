#ifndef CPE_DR_TEST_PRIORITYQUEUE_H
#define CPE_DR_TEST_PRIORITYQUEUE_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/utils/priority_queue.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) PriorityQueueTestBase;

class PriorityQueueTest : public testenv::fixture<PriorityQueueTestBase> {
public:
    virtual void SetUp();
    virtual void TearDown();

    void insert(int32_t v);
    int32_t top(void);
    uint16_t count(void);

    const char * dump(void);
    const char * dump_dequeue(void);

    cpe_priority_queue_t m_queue;

    static int ele_cmp(void const * l, void const * r);
    static void ele_dump(write_stream_t s, void const * e);
};

#endif
