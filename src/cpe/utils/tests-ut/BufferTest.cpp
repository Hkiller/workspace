#include "BufferTest.hpp"

void BufferTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_buffer, t_allocrator());
}

void BufferTest::TearDown() {
    mem_buffer_clear(&m_buffer);
    Base::TearDown();
}

size_t BufferTest::append_string(const char * data) {
    return mem_buffer_append(&m_buffer, data, strlen(data));
}

size_t BufferTest::append_zero() {
    char buf = 0;
    return mem_buffer_append(&m_buffer, &buf, 1);
}

struct mem_buffer_trunk *
BufferTest::append_trunk(const char * data) {
    if (data) {
        size_t capacity = strlen(data);
        struct mem_buffer_trunk * trunk =
            mem_buffer_append_trunk(&m_buffer, capacity);
        mem_trunk_append(&m_buffer, trunk, data, capacity);
        return trunk;
    }
    else {
        return mem_buffer_append_trunk(&m_buffer, m_buffer.m_auto_inc_size);
    }
}

size_t BufferTest::trunk_count(void) {
    return mem_buffer_trunk_count(&m_buffer);
}

struct mem_buffer_trunk *
BufferTest::trunk_at(size_t pos) {
    return mem_buffer_trunk_at(&m_buffer, pos);
}

char *
BufferTest::as_string(void) {
    return (char *)mem_buffer_make_continuous(&m_buffer, 0);
}

size_t BufferTest::buffer_size(void) {
    return mem_buffer_size(&m_buffer);
}
