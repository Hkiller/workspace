#include "cpe/nm/nm_manage.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "usf/bpg_pkg/tests-env/with_bpg_pkg.hpp"

extern "C" {
    int bpg_metalib_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
    void bpg_metalib_app_fini(gd_app_context_t app, gd_app_module_t module);
}

namespace usf { namespace bpg { namespace testenv {

with_bpg_pkg::with_bpg_pkg() {
}

void with_bpg_pkg::SetUp() {
    envOf<utils::testenv::with_em>().t_em_set_print();
    
    envOf<gd::app::testenv::with_app>()
        .t_app_init_module_type("bpg_metalib", bpg_metalib_app_init, bpg_metalib_app_fini);

    ASSERT_TRUE(
        envOf<gd::app::testenv::with_app>()
        .t_app_install_module("bpg_metalib", NULL, NULL));
}

void with_bpg_pkg::TearDown() {
    envOf<gd::app::testenv::with_app>()
        .t_app_uninstall_modules_by_type("usf_bpg_pkg_manage");

    envOf<gd::app::testenv::with_app>().t_app_uninstall_module("bpg_metalib");
    envOf<gd::app::testenv::with_app>().t_app_fini_module_type("bpg_metalib");
}

bpg_pkg_manage_t
with_bpg_pkg::t_bpg_pkg_manage(const char * name) {
    if (name == NULL) name = "TestPkgManager";

    bpg_pkg_manage_t mgr = bpg_pkg_manage_find_nc(envOf<gd::app::testenv::with_app>().t_app(), name);
    if (mgr == NULL) {
        mgr =
            bpg_pkg_manage_create(
                envOf<gd::app::testenv::with_app>().t_app(),
                name,
                envOf<utils::testenv::with_em>().t_em());
        EXPECT_TRUE(mgr) << "bpg_pkg_manager create fail!";
    }

    return mgr;
}

void with_bpg_pkg::t_bpg_pkg_manage_set_cvt(const char * data_cvt, const char * base_cvt, const char * mgr_name) {
    bpg_pkg_manage_t mgr = t_bpg_pkg_manage(mgr_name);
    if (mgr == NULL) return;

    bpg_pkg_manage_set_data_cvt(mgr, data_cvt);
    bpg_pkg_manage_set_base_cvt(mgr, base_cvt);
}

void with_bpg_pkg::t_bpg_pkg_manage_set_model(const char * model, uint8_t dft_align, const char * mgr_name) {
    bpg_pkg_manage_t mgr = t_bpg_pkg_manage(mgr_name);
    if (mgr == NULL) return;

    char metalib_name[256];
    snprintf(metalib_name, sizeof(metalib_name), "metalib.%s", bpg_pkg_manage_name(mgr));
    envOf<gd::dr_store::testenv::with_dr_store>().t_dr_store_reset(metalib_name, model, dft_align);

    ASSERT_TRUE(bpg_pkg_manage_set_data_metalib(mgr, metalib_name) == 0)
        << "set metalib to bpg-pkg-manager fail";
}

void with_bpg_pkg::t_bpg_pkg_manage_add_cmd(uint32_t cmd, const char * meta_name, const char * mgr_name) {
    bpg_pkg_manage_t mgr = t_bpg_pkg_manage(mgr_name);
    ASSERT_TRUE(mgr) << "get bpg_pkg_manage " << mgr_name << " fail!";
    bpg_pkg_manage_add_cmd(mgr, cmd, meta_name);
}

void with_bpg_pkg::t_bpg_pkg_manage_add_cmd_by_meta(const char * cmd_meta_name, const char * mgr_name) {
    bpg_pkg_manage_t mgr = t_bpg_pkg_manage(mgr_name);
    ASSERT_TRUE(mgr) << "get bpg_pkg_manage " << mgr_name << " fail!";
    bpg_pkg_manage_add_cmd_by_meta(mgr, cmd_meta_name);
}

dp_req_t with_bpg_pkg::t_bpg_pkg_create(size_t capacity, const char * mgr_name) {
    return bpg_pkg_create_with_body(t_bpg_pkg_manage(mgr_name), capacity);
}

const char * with_bpg_pkg::t_bpg_pkg_dump(dp_req_t body) {
    mem_buffer buffer;
    mem_buffer_init(&buffer, NULL);

    const char * r = t_tmp_strdup(bpg_pkg_dump(body, &buffer));

    mem_buffer_clear(&buffer);

    return r;
}

dp_req_t with_bpg_pkg::t_bpg_pkg_build(const char * cfg, const char * mgr_name) {
    dp_req_t body = t_bpg_pkg_create(1024, mgr_name);
    if (body == NULL) return NULL;
    t_bpg_pkg_init(body, cfg);
    return body; 
}

dp_req_t with_bpg_pkg::t_bpg_pkg_build(cfg_t cfg, const char * mgr_name) {
    dp_req_t body = t_bpg_pkg_create(1024, mgr_name);
    if (body == NULL) return NULL;
    t_bpg_pkg_init(body, cfg);
    return body; 
}

void with_bpg_pkg::t_bpg_pkg_init(dp_req_t body, const char * cfg) {
    t_bpg_pkg_init(body, envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(cfg));
}

void with_bpg_pkg::t_bpg_pkg_init(dp_req_t body, cfg_t cfg) {
    ASSERT_EQ(
        0,
        bpg_pkg_build_from_cfg(body, cfg, envOf<utils::testenv::with_em>().t_em()))
        << "build pkg from cfg fail!";
}

}}}
