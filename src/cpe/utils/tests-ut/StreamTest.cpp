#include "StreamTest.hpp"

void StreamTest::SetUp() {
    BufferTest::SetUp();

    write_stream_buffer_init(&m_stream, &m_buffer);
}

void StreamTest::TearDown() {
    BufferTest::TearDown();
}
