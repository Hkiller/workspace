#include "cpe/utils/tests-env/with_em.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/dr_cvt/dr_cvt_manage.h"
#include "gd/dr_cvt/tests-env/with_dr_cvt.hpp"

extern "C" {
int dr_pbuf_cvt_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
void dr_pbuf_cvt_app_fini(gd_app_context_t app, gd_app_module_t module);

int dr_pbuf_len_cvt_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg);
void dr_pbuf_len_cvt_app_fini(gd_app_context_t app, gd_app_module_t module);
}

namespace gd { namespace dr_cvt { namespace testenv {

void with_dr_cvt::SetUp() {
    Base::SetUp();

    dr_cvt_manage_t dr_cvt_mgr =
        dr_cvt_manage_create(
            envOf<gd::app::testenv::with_app>().t_app(),
            NULL,
            t_allocrator(),
            envOf<utils::testenv::with_em>().t_em());
    ASSERT_TRUE(dr_cvt_mgr);

    envOf<gd::app::testenv::with_app>()
        .t_app_init_module_type("dr_pbuf_cvt", dr_pbuf_cvt_app_init, dr_pbuf_cvt_app_fini);

    envOf<gd::app::testenv::with_app>()
        .t_app_init_module_type("dr_pbuf_len_cvt", dr_pbuf_len_cvt_app_init, dr_pbuf_len_cvt_app_fini);
}

void with_dr_cvt::TearDown() {
    envOf<gd::app::testenv::with_app>().t_app_uninstall_module("dr_pbuf_cvt");
    envOf<gd::app::testenv::with_app>().t_app_fini_module_type("dr_pbuf_cvt");

    envOf<gd::app::testenv::with_app>().t_app_uninstall_module("dr_pbuf_len_cvt");
    envOf<gd::app::testenv::with_app>().t_app_fini_module_type("dr_pbuf_len_cvt");

    envOf<gd::app::testenv::with_app>()
        .t_app_uninstall_modules_by_type("gd_dr_cvt_manage");
}

void with_dr_cvt::t_dr_cvt_install_pbuf(void) {
    envOf<gd::app::testenv::with_app>().t_app_install_module("dr_pbuf_cvt", NULL, NULL);
}

void with_dr_cvt::t_dr_cvt_install_pbuf_len(void) {
    envOf<gd::app::testenv::with_app>().t_app_install_module("dr_pbuf_len_cvt", NULL, NULL);
}

}}}
