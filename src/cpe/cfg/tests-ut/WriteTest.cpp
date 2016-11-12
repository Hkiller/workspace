#include "cpe/utils/stream_buffer.h"
#include "WriteTest.hpp"

WriteTest::WriteTest() : m_errorList(0) {
}

void WriteTest::SetUp() {
    CfgTest::SetUp();
    mem_buffer_init(&m_buffer, NULL);
}

void WriteTest::TearDown() {
    mem_buffer_clear(&m_buffer);
    cpe_error_list_free(m_errorList);

    CfgTest::TearDown();
}

int WriteTest::write(cfg_t cfg) {
    cpe_error_list_free(m_errorList);
    m_errorList = cpe_error_list_create(NULL);

    CPE_DEF_ERROR_MONITOR(em, cpe_error_list_collect, m_errorList);
    CPE_DEF_ERROR_MONITOR_ADD(printer, &em, cpe_error_log_to_consol, NULL);

    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&m_buffer);

    int r = cfg_yaml_write((write_stream_t)&stream, cfg, &em);
    stream_putc((write_stream_t)&stream, 0);
    return r;
}

const char * WriteTest::result(void) {
    return (const char *)mem_buffer_make_exactly(&m_buffer);
}
