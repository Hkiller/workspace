#ifndef CPE_PLIST_TEST_CFG_H
#define CPE_PLIST_TEST_CFG_H
#include <string.h>
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/cfg/cfg.h"
#include "cpe/plist/plist_cfg.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em
    , cpe::cfg::testenv::with_cfg) CfgTestBase;

class CfgTest : public testenv::fixture<CfgTestBase> {
public:
    CfgTest();
    virtual void SetUp();
    virtual void TearDown();

    cfg_t m_root;
    int m_test_attr_id;

    cfg_t build(int typeId, const char * value);
    cfg_t build(const char * value);

    cfg_t build_dft(int typeId);

    struct mem_buffer m_result_buffer;
    const char * result(void);
    const char * result(cfg_t cfg);

    int read(const char * input, cfg_policy_t policy = cfg_replace);
    int read(cfg_t cfg, const char * input, cfg_policy_t policy = cfg_replace);

};

#endif
