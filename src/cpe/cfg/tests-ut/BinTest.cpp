#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "BinTest.hpp"

BinTest::BinTest() {
}

void BinTest::SetUp() {
    Base::SetUp();
}

void BinTest::TearDown() {
    Base::TearDown();
}

cfg_t BinTest::build_by_bin(cfg_t cfg) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, t_allocrator());

    int write_r = cfg_bin_write_to_buffer(&buffer, cfg, t_em());
    EXPECT_LT(0, write_r);
    EXPECT_EQ(write_r, (int)mem_buffer_size(&buffer));

    char name[128];
    snprintf(name, sizeof(name), "test_%d", ++m_test_attr_id);

    EXPECT_EQ(0, cfg_bin_read_with_name(m_root, name, mem_buffer_make_continuous(&buffer, 0), mem_buffer_size(&buffer), t_em()));

    mem_buffer_clear(&buffer);

    return cfg_find_cfg(m_root, name);
}

cfg_t BinTest::build_by_bin_file(const char * path) {
    char name[128];
    snprintf(name, sizeof(name), "test_%d", ++m_test_attr_id);

    EXPECT_EQ(0, cfg_bin_read_file(m_root, t_vfs(), path, t_em()));

    return m_root;
}

cfg_t BinTest::build_by_bin(int typeId, const char * value) {
    cfg_t input = build(typeId, value);
    EXPECT_TRUE(input != NULL) << "build input of (type=" << typeId << ", value=" << value << " fail!";
    if (input == NULL) return NULL;

    return build_by_bin(input);
}

cfg_t BinTest::build_by_bin(const char * value) {
    cfg_t input = build(value);
    EXPECT_TRUE(input != NULL) << "build input of (value=" << value << " fail!";
    if (input == NULL) return NULL;

    return build_by_bin(input);
}
