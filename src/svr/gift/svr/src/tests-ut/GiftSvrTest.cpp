#include <stdexcept>
#include "usf/logic/logic_executor_type.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "GiftSvrTest.hpp"
#include "../gift_svr_generate_record.h"
#include "../gift_svr_generator.h"

extern "C" {
    extern int gift_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
    extern void gift_svr_app_fini(gd_app_context_t app, gd_app_module_t module);
    extern char g_metalib_svr_gift_pro[];
}

void GiftSvrTest::SetUp() {
    Base::SetUp();

    t_em_set_print();

    t_dr_store_install("svr_gift_lib", (LPDRMETALIB)g_metalib_svr_gift_pro);

    t_install_set_svr_types(
        "gift_svr:\n"
        "    id: 12\n"
        "    pkg-meta: svr_gift_lib.svr_gift_pkg\n"
        "    pkg-meta-data: data\n"
        "    pkg-meta-error: svr_gift_res_error.error\n"
        "    connect-to: [ ]\n");

    t_set_stub_create("gift_svr");

    t_app_init_module_type("gift_svr", gift_svr_app_init, gift_svr_app_fini);

    m_gift_svr = NULL;
}

void GiftSvrTest::TearDown() {
    if (m_gift_svr) {
        gift_svr_free(m_gift_svr);
        m_gift_svr = NULL;
    }

    t_app_uninstall_modules_by_type("gift_svr");
    t_app_fini_module_type("gift_svr");

    Base::TearDown();
}

void GiftSvrTest::t_gift_svr_create(void) {
    if (m_gift_svr) {
        gift_svr_free(m_gift_svr);
        m_gift_svr = NULL;
    }

    m_gift_svr = gift_svr_create(t_app(), "gift_svr", t_set_stub(), NULL, NULL, t_allocrator(), t_em());
    ASSERT_TRUE(m_gift_svr);
}

void GiftSvrTest::t_generate_mgr_init(const char * meta, uint32_t record_count, float ratio) {
    ASSERT_TRUE(gift_svr());
    
    LPDRMETALIB metalib = t_create_metalib(meta);

    ASSERT_TRUE(dr_lib_meta_num(metalib) > 0);

    LPDRMETA data_meta = dr_lib_meta_at(metalib, dr_lib_meta_num(metalib) - 1);

    ASSERT_TRUE(gift_svr_generate_record_init(gift_svr(), data_meta, record_count, ratio) == 0);
}

const char * GiftSvrTest::dump(struct gift_svr_generator_block * block) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%s*%d", gift_svr_generator_scope_name(block->m_scope), block->m_len);
    return t_tmp_strdup(buf);
}
