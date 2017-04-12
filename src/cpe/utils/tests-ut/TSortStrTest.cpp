#include "cpe/utils/buffer.h"
#include "TSortStrTest.hpp"

void TSortStrTest::SetUp() {
    Base::SetUp();
    m_sorter = tsorter_str_create(t_allocrator(), 1);
    ASSERT_TRUE(m_sorter);
}

void TSortStrTest::TearDown() {
    tsorter_str_free(m_sorter);
    Base::TearDown();
}

const char * TSortStrTest::sort(void) {
    const char * r = NULL;
    return sort(r) == 0 ? r : NULL;
}

int TSortStrTest::sort(const char * & str) {
    struct mem_buffer buffer;
    struct tsorter_str_it it;

    int r = tsorter_str_sort(&it, m_sorter);

    mem_buffer_init(&buffer, NULL);

    while(const char * d = tsorter_str_next(&it)) {
        mem_buffer_strcat(&buffer, d);
        mem_buffer_strcat(&buffer, ":");
    }

    mem_buffer_strcat(&buffer, "");

    str = t_tmp_strdup((const char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);

    return r;
}

void TSortStrTest::addDepend(const char * dep_from, const char * dep_to) {
    EXPECT_TRUE(tsorter_str_add_dep(m_sorter, dep_from, dep_to) == 0);
}
