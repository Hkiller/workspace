#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/utils/tests-env/with_id_generator.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "gd/dr_dm/dr_dm_manage.h"
#include "gd/dr_dm/tests-env/with_dr_dm.hpp"
#include "cpe/utils/tests-env/with_em.hpp"

namespace gd { namespace dr_dm { namespace testenv {

with_dr_dm::with_dr_dm() {
}

void with_dr_dm::SetUp() {
    Base::SetUp();
}

void with_dr_dm::TearDown() {
    Base::TearDown();
}

dr_dm_manage_t with_dr_dm::t_dr_dm_manage(const char * name) {
    dr_dm_manage_t r = dr_dm_manage_find_nc(
        envOf<gd::app::testenv::with_app>().t_app(),
        name);
    EXPECT_TRUE(r) << "dr_dm_manage " << name << " not exist!";
    return r;
}

dr_dm_manage_t
with_dr_dm::t_dr_dm_manage_create(
    const char * name,
    const char * metalibname,
    const char * metaname,
    const char * id_attr_name,
    const char * id_generator)
{
    return t_dr_dm_manage_create(
        name,
        envOf<gd::dr_store::testenv::with_dr_store>().t_meta(metalibname, metaname),
        id_attr_name,
        id_generator);
}

dr_dm_manage_t
with_dr_dm::t_dr_dm_manage_create(
    const char * name,
    LPDRMETA meta,
    const char * id_attr_name,
    const char * id_generator)
{
    dr_dm_manage_t r =
        dr_dm_manage_create(
            envOf<gd::app::testenv::with_app>().t_app(), name, NULL, NULL);
    EXPECT_TRUE(r) << "dr_dm_manage " << name << " create fail!";
    if (r == NULL) return NULL;

    dr_dm_manage_set_meta(r, meta, NULL);

    if (id_attr_name) {
        EXPECT_EQ(
            0,
            dr_dm_manage_set_id_attr(r, id_attr_name));
    }

    if (id_generator) {
        dr_dm_manage_set_id_generate(
            r,
            envOf<gd::utils::testenv::with_id_generator>()
            .t_id_generator(id_generator));
    }

    return r;
}

}}}
