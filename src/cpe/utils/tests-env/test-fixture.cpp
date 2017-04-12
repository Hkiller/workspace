#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/memory_debug.h"
#include "cpe/utils/hex_utils.h"
#include "cpe/utils/tests-env/test-fixture.hpp"

namespace testenv {

/*tmp allocrator impl*/
static void * do_tmp_alloc(size_t size, struct mem_allocrator * allocrator) {
    return mem_buffer_alloc((mem_buffer_t)(allocrator + 1), size);
}

void do_tmp_free(void * p, struct mem_allocrator * allocrator) {
}

CPE_DEF_ERROR_MONITOR(g_test_fixture_em, cpe_error_log_to_consol, NULL);

/*Test impl*/
Test::Test() : m_allocrator(NULL) {
    m_tmp_allocrator.m_alloc = do_tmp_alloc;
    m_tmp_allocrator.m_free = do_tmp_free;
    mem_buffer_init(&m_tmp_alloc_buf, NULL);

    m_allocrator = mem_allocrator_debug_create(&m_tmp_allocrator, &m_tmp_allocrator, 10, &g_test_fixture_em);
}

Test::~Test() {
    if (m_allocrator) mem_allocrator_debug_free(m_allocrator);
    mem_buffer_clear(&m_tmp_alloc_buf);
}

void Test::SetUp() {
}

void Test::TearDown() {
    CHECK_NO_MEMLEAK();
}

int Test::t_alloc_count(void) const {
    return mem_allocrator_debug_alloc_count(m_allocrator);
}

int Test::t_free_count(void) const {
    return mem_allocrator_debug_free_count(m_allocrator);
}

const char * Test::t_allocrator_alloc_info(void) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, NULL);

    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);
    mem_allocrator_debug_dump((write_stream_t)&stream, 4, m_allocrator);
    stream_putc((write_stream_t)&stream, 0);

    char * r = t_tmp_strdup((char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);

    return r;
}

mem_allocrator_t
Test::t_allocrator() {
    return m_allocrator;
}

void *
Test::t_alloc(size_t size) {
    return mem_alloc(t_allocrator(), size);
}

char *
Test::t_strdup(const char * str) {
    size_t len = strlen(str) + 1;
    char * p = (char*)t_alloc(len);
    memcpy(p, str, len);
    return p;
}

mem_allocrator_t
Test::t_tmp_allocrator() {
    return &m_tmp_allocrator;
}

void *
Test::t_tmp_alloc(size_t size) {
    return mem_buffer_alloc(&m_tmp_alloc_buf, size);
}

char *
Test::t_tmp_strdup(const char * str) {
    return mem_buffer_strdup(&m_tmp_alloc_buf, str);
}

void * Test::t_tmp_memdup(void const * buf, size_t size) {
    void * r = t_tmp_alloc(size);
    memcpy(r, buf, size);
    return r;
}

char * Test::t_tmp_hexdup(void const * buf, size_t size) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, NULL);

    char * r = t_tmp_strdup(cpe_hex_dup_buf(buf, size, &buffer));

    mem_buffer_clear(&buffer);

    return r;
}

}
