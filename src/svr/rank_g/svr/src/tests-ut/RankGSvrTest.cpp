#include <stdexcept>
#include "usf/logic/logic_executor_type.h"
#include "RankGSvrTest.hpp"

extern "C" {
    extern int rank_g_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
    extern void rank_g_svr_app_fini(gd_app_context_t app, gd_app_module_t module);
    extern char g_metalib_svr_rank_g_pro[];
}

void RankGSvrTest::SetUp() {
    Base::SetUp();

    t_em_set_print();

    t_dr_store_install("svr_rank_g_lib", (LPDRMETALIB)g_metalib_svr_rank_g_pro);

    t_install_set_svr_types(
        "rank_g_svr:\n"
        "    id: 12\n"
        "    pkg-meta: svr_rank_g_lib.svr_rank_g_pkg\n"
        "    pkg-meta-data: data\n"
        "    pkg-meta-error: svr_rank_g_res_error.error\n"
        "    connect-to: [ ]\n");

    t_set_stub_create("rank_g_svr");

    t_app_init_module_type("rank_g_svr", rank_g_svr_app_init, rank_g_svr_app_fini);

    m_rank_g_svr = NULL;
}

void RankGSvrTest::TearDown() {
    if (m_rank_g_svr) {
        rank_g_svr_free(m_rank_g_svr);
        m_rank_g_svr = NULL;
    }

    t_app_uninstall_modules_by_type("rank_g_svr");
    t_app_fini_module_type("rank_g_svr");

    Base::TearDown();
}

void RankGSvrTest::t_rank_g_svr_create(void) {
    if (m_rank_g_svr) {
        rank_g_svr_free(m_rank_g_svr);
        m_rank_g_svr = NULL;
    }

    m_rank_g_svr = rank_g_svr_create(t_app(), "rank_g_svr", t_set_stub(), NULL, NULL, NULL, t_allocrator(), t_em());
    ASSERT_TRUE(m_rank_g_svr != NULL);
}
