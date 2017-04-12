#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "ReadTest.hpp"

ReadTest::ReadTest() {
}

void ReadTest::SetUp() {
    Base::SetUp();
}

void ReadTest::TearDown() {
    Base::TearDown();
}

int ReadTest::read(const char * input, cfg_policy_t policy) {
    return read(m_root, input, policy);
}

int ReadTest::read(cfg_t cfg, const char * input, cfg_policy_t policy) {
    struct read_stream_mem stream = CPE_READ_STREAM_MEM_INITIALIZER(input, strlen(input));
    return cfg_yaml_read(cfg, (read_stream_t)&stream, policy, t_em());
}

