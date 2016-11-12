#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_manage.h"
#include "NodeTest.hpp"

NodeTest::NodeTest() : _cfg(NULL) {
}

void NodeTest::SetUp() {
    Base::SetUp();
    _cfg = cfg_create(t_tmp_allocrator());
}

void NodeTest::TearDown() {
    if (_cfg) {
        cfg_free(_cfg);
        _cfg = NULL;
    }
    Base::TearDown();
}

void NodeTest::install(const char * input) {
    struct read_stream_mem stream = CPE_READ_STREAM_MEM_INITIALIZER(input, strlen(input));

    CPE_DEF_ERROR_MONITOR(em, cpe_error_log_to_consol, NULL);

    ASSERT_EQ(
        0,
        cfg_read(_cfg, (read_stream_t)&stream, cfg_replace, &em));
}

