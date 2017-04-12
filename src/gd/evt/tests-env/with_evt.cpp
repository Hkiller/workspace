#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "gd/evt/tests-env/with_evt.hpp"

namespace gd { namespace evt { namespace testenv {

with_evt::with_evt() {
}

void with_evt::SetUp() {
    gd_evt_mgr_t evt_mgr = 
        gd_evt_mgr_create(
            envOf<gd::app::testenv::with_app>().t_app(),
            NULL,
            t_allocrator(),
            NULL);
    EXPECT_TRUE(evt_mgr) << "crate default gd_evt_mgr fail!";
}

void with_evt::TearDown() {
}

void with_evt::t_evt_mgr_set_metalib(const char * metalib) {
    envOf<gd::dr_store::testenv::with_dr_store>().t_dr_store_install("test-lib", metalib);
    LPDRMETALIB m = envOf<gd::dr_store::testenv::with_dr_store>().t_metalib("test-lib");
    if (m) {
        EXPECT_EQ(0, gd_evt_mgr_register_evt_in_metalib(t_evt_mgr(), m));
    }
}

gd_evt_mgr_t
with_evt::t_evt_mgr(const char * name) {
    gd_evt_mgr_t r = gd_evt_mgr_find_nc(
        envOf<gd::app::testenv::with_app>().t_app(),
        name);
    EXPECT_TRUE(r) << "gd_evt_mgr " << (name ? name : "default") 
                   << " not exist!";
    return r;
}

}}}
