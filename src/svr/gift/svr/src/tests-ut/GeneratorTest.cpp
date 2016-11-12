#include "cpe/cfg/cfg_manage.h"
#include "cpepp/utils/MemBuffer.hpp"
#include "GeneratorTest.hpp"

void GeneratorTest::SetUp() {
    Base::SetUp();

    t_gift_svr_create();

    m_generator = NULL;
}

void GeneratorTest::TearDown() {
    if (m_generator) {
        gift_svr_generator_free(m_generator);
        m_generator = NULL;
    }

    Base::TearDown();
}

void GeneratorTest::t_generator_create(const char * name, const char * def) {
    if (m_generator) {
        gift_svr_generator_free(m_generator);
        m_generator = NULL;
    }

    cfg_t cfg = t_cfg_parse(def);
    ASSERT_TRUE(cfg);

    m_generator = gift_svr_generator_create(gift_svr(), name, cfg);
    ASSERT_TRUE(m_generator != NULL);
}

const char * GeneratorTest::select_prefix(void) {
    char buf[32] = {0};
    EXPECT_TRUE(gift_svr_generator_select_prefix(m_generator, buf) == 0);
    return t_tmp_strdup(buf);
}
