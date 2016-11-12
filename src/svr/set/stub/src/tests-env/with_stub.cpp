#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "svr/set/stub/tests-env/with_stub.hpp"
#include "../set_svr_stub_internal_ops.h"

extern "C" {
    extern int set_svr_stub_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
    extern void set_svr_stub_app_fini(gd_app_context_t app, gd_app_module_t module);
}

namespace svr { namespace set { namespace stub { namespace testenv {

void with_stub::SetUp() {
    Base::SetUp();

    m_stub = NULL;

    envOf<gd::app::testenv::with_app>()
        .t_app_init_module_type("set_svr_stub", set_svr_stub_app_init, set_svr_stub_app_fini);
}

void with_stub::TearDown() {
    if (m_stub) {
        set_svr_stub_free(m_stub);
        m_stub = NULL;
    }

    envOf<gd::app::testenv::with_app>()
        .t_app_uninstall_modules_by_type("set_svr_stub");

    envOf<gd::app::testenv::with_app>()
        .t_app_fini_module_type("set_svr_stub");

    Base::TearDown();
}

void with_stub::t_install_set_svr_types(const char * str_svr_types) {
    gd_app_context_t app =
        envOf<gd::app::testenv::with_app>().t_app();
    ASSERT_TRUE(app != NULL);

    cfg_t svr_types = cfg_find_cfg(gd_app_cfg(app), "svr_types");
    if (svr_types == NULL) {
        svr_types = cfg_struct_add_struct(gd_app_cfg(app), "svr_types", cfg_merge_use_new);
    }

    ASSERT_TRUE(svr_types != NULL);

    envOf<cpe::cfg::testenv::with_cfg>().t_cfg_read(svr_types, str_svr_types);
}

uint16_t with_stub::t_set_svr_type_id_of(const char * svr_type) {
    gd_app_context_t app =
        envOf<gd::app::testenv::with_app>().t_app();
    EXPECT_TRUE(app != NULL);
    if (app == NULL) return 0;

    cfg_t svr_types = cfg_find_cfg(gd_app_cfg(app), "svr_types");
    EXPECT_TRUE(svr_types != NULL) << "svr_types not configured!";
    if (svr_types == NULL) return 0;

    cfg_t svr_type_cfg = cfg_find_cfg(svr_types, svr_type);
    EXPECT_TRUE(svr_types != NULL) << "svr_type " << svr_type << " not install!";
    if (svr_type_cfg == NULL) return 0;

    uint16_t result = 0;
    EXPECT_EQ(0, cfg_try_get_uint16(svr_type_cfg, "id", &result));

    return result;
}

void with_stub::t_set_stub_create(const char * svr_type, const char * name) {
    if (m_stub) {
        set_svr_stub_free(m_stub);
        m_stub = NULL;
    }

    error_monitor_t em = NULL;;
    if (utils::testenv::with_em * with_em = tryEnvOf<utils::testenv::with_em>()) {
        em = with_em->t_em();
    }

    m_stub = set_svr_stub_create(
        envOf<gd::app::testenv::with_app>().t_app(),
        name,
        t_set_svr_type_id_of(svr_type),
        t_allocrator(),
        em);
    ASSERT_TRUE(m_stub != NULL);
}

}}}}
