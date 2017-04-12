#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "MergeTest.hpp"

MergeTest::MergeTest() {
}

void MergeTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_result_buffer, NULL);
}

void MergeTest::TearDown() {
    mem_buffer_clear(&m_result_buffer);
    Base::TearDown();
}

void MergeTest::install(const char * input) {
    install(m_root, input);
}

void MergeTest::install(cfg_t cfg, const char * input) {
    struct read_stream_mem stream = CPE_READ_STREAM_MEM_INITIALIZER(input, strlen(input));
    ASSERT_EQ(0, cfg_yaml_read(cfg, (read_stream_t)&stream, cfg_replace, t_em()));
}

int MergeTest::merge(const char * input, cfg_policy_t policy) {
    cfg_t inputCfg = cfg_create(t_tmp_allocrator());
    EXPECT_TRUE(inputCfg);

    if (inputCfg) { install(inputCfg, input); }

    return cfg_merge(m_root, inputCfg, policy, t_em());
}

const char * MergeTest::result(void) {
    return result(m_root);
}

const char * MergeTest::result(cfg_t cfg) {
    CPE_DEF_ERROR_MONITOR(em, cpe_error_log_to_consol, NULL);

    mem_buffer_clear(&m_result_buffer);
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&m_result_buffer);

    EXPECT_EQ(0, cfg_yaml_write((write_stream_t)&stream, cfg, &em));
    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_exactly(&m_result_buffer);
}
