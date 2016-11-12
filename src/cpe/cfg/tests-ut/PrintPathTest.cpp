#include "cpe/utils/stream_mem.h"
#include "PrintPathTest.hpp"

PrintPathTest::PrintPathTest() : m_root(NULL) {
}

void PrintPathTest::TearDown() {
    if (m_root) {
        cfg_free(m_root);
        m_root = 0;
    }

    Base::TearDown();
}

void PrintPathTest::installCfg(const char * def) {
    if (m_root) {
        cfg_free(m_root);
        m_root = 0;
    }

    m_root = cfg_create(t_tmp_allocrator());

    read_stream_mem inputStream = CPE_READ_STREAM_MEM_INITIALIZER(def, strlen(def));
    EXPECT_EQ(
        0,
        cfg_yaml_read(m_root, (read_stream_t)&inputStream, cfg_merge_use_new, 0))
        << "parse cfg fail!\ninput:\n" << def;
}

const char *
PrintPathTest::path(const char * p, const char * r) {
    EXPECT_TRUE(m_root) << "root not set!";

    cfg_t input = cfg_find_cfg(m_root, p);
    EXPECT_TRUE(input) << "input path " << p << " not exist";

    cfg_t to = 0;
    if (r) {
        to = cfg_find_cfg(m_root, r);
        EXPECT_TRUE(input) << "root path " << r << " not exist";
    }

    struct mem_buffer buffer;
    mem_buffer_init(&buffer, t_tmp_allocrator());

    return cfg_path(&buffer, input, to);
}

