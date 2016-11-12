#ifndef SVR_GIFT_SVR_TEST_H
#define SVR_GIFT_SVR_TEST_H
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "svr/set/stub/tests-env/with_stub.hpp"
#include "usf/logic/tests-env/with_logic.hpp"
#include "../gift_svr.h"

typedef LOKI_TYPELIST_6(
    utils::testenv::with_em
    , cpe::dr::testenv::with_dr
    , cpe::cfg::testenv::with_cfg
    , gd::app::testenv::with_app
    , gd::dr_store::testenv::with_dr_store
    , svr::set::stub::testenv::with_stub
    ) GiftSvrTestBase;

class GiftSvrTest : public testenv::fixture<GiftSvrTestBase> {
public:
    void SetUp();
    void TearDown();

    void t_gift_svr_create(void);

    gift_svr_t gift_svr(void) { return m_gift_svr; }
    void t_generate_mgr_init(const char * meta, uint32_t record_count = 100, float ratio = 1.5f);

    const char * dump(gift_svr_generator_block_t block);
    
private:
    gift_svr_t m_gift_svr;
};

#define ASSERT_PREFIX_MATCH(__prefix, __generator)                      \
    do {                                                                \
        const char * __p = (__prefix);                                  \
        gift_svr_generator_t __g = (__generator);                       \
        ASSERT_TRUE(gift_svr_generator_prefix_match(__g, __p))          \
            << "prefix " << __p \
            << " scope " << dump(&__g->m_prefix) \
            << " mismatch";                      \
    } while(0)
              
#endif
