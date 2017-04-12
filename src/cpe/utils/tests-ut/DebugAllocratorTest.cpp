#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "DebugAllocratorTest.hpp"

DebugAllocratorTest::DebugAllocratorTest() : m_allocrator(NULL) {
}

void DebugAllocratorTest::SetUp() {
    Base::SetUp();

    m_allocrator = mem_allocrator_debug_create(t_allocrator(), t_tmp_allocrator(), 10, t_em());
    ASSERT_TRUE(m_allocrator);
}

void DebugAllocratorTest::TearDown() {
    if (m_allocrator) {
        mem_allocrator_debug_free(m_allocrator);
        m_allocrator = NULL;
    }

    Base::TearDown();
}

const char * DebugAllocratorTest::dump(void) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, NULL);

    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);
    mem_allocrator_debug_dump((write_stream_t)&stream, 4, m_allocrator);
    stream_putc((write_stream_t)&stream, 0);

    char * r = t_tmp_strdup((char *)mem_buffer_make_continuous(&buffer, 0));
    mem_buffer_clear(&buffer);
    return r;
}
