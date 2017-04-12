#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "cpe/utils/tests-env/with_em.hpp"

namespace gd { namespace dr_store { namespace testenv {

with_dr_store::with_dr_store()
    : m_dr_store_mgr(NULL)
{
}

void with_dr_store::SetUp() {
    Base::SetUp();

    m_dr_store_mgr = dr_store_manage_create(
        envOf<gd::app::testenv::with_app>().t_app(),
        NULL,
        t_allocrator(),
        NULL);
}

void with_dr_store::TearDown() {
    dr_store_manage_free(m_dr_store_mgr);
    m_dr_store_mgr = NULL;

    Base::TearDown();
}

void with_dr_store::t_dr_store_install(const char * name, const char * def, uint8_t dft_align) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, t_tmp_allocrator());

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&buffer, def, strlen(def), dft_align, envOf<utils::testenv::with_em>().t_em()));

    dr_store_t dr_store = dr_store_create(m_dr_store_mgr, name);
    ASSERT_TRUE(dr_store) << "crate dr_store " << name << " fail!";

    EXPECT_EQ(
        0, 
        dr_store_set_lib(dr_store, (LPDRMETALIB)mem_buffer_make_continuous(&buffer, 0), NULL, NULL));
}

void with_dr_store::t_dr_store_install(const char * libname, LPDRMETALIB metalib) {
    dr_store_t dr_store = dr_store_create(m_dr_store_mgr, libname);
    ASSERT_TRUE(dr_store) << "crate dr_store " << libname << " fail!";

    EXPECT_EQ(
        0, 
        dr_store_set_lib(dr_store, metalib, NULL, NULL));
}

void with_dr_store::t_dr_store_reset(const char * name, const char * def, uint8_t dft_align) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, t_tmp_allocrator());

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&buffer, def, strlen(def), dft_align, envOf<utils::testenv::with_em>().t_em()));

    dr_store_t dr_store = dr_store_find_or_create(m_dr_store_mgr, name);
    ASSERT_TRUE(dr_store) << "find or crate dr_store " << name << " fail!";

    dr_store_reset_lib(dr_store, (LPDRMETALIB)mem_buffer_make_continuous(&buffer, 0), NULL, NULL);
}

void with_dr_store::t_dr_store_reset(const char * libname, LPDRMETALIB metalib) {
    dr_store_t dr_store = dr_store_find_or_create(m_dr_store_mgr, libname);
    ASSERT_TRUE(dr_store) << "find or crate dr_store " << libname << " fail!";

    dr_store_reset_lib(dr_store, metalib, NULL, NULL);
}

LPDRMETALIB with_dr_store::t_metalib(const char * libname) {
    dr_store_t store = dr_store_find(m_dr_store_mgr, libname);
    EXPECT_TRUE(store) << "dr_store " << libname << " not exist!";
    if (store == NULL) return NULL;

    LPDRMETALIB lib = dr_store_lib(store);
    EXPECT_TRUE(lib) << "dr_store " << libname << " have no lib!";
    return lib;
}

LPDRMETA with_dr_store::t_meta(const char * libname, const char * metaname) {
    LPDRMETALIB lib = t_metalib(libname);
    if (lib == NULL) return NULL;

    LPDRMETA meta = dr_lib_find_meta_by_name(lib, metaname);
    EXPECT_TRUE(meta) << "dr_store " << libname << " have no meta " << metaname;
    return meta;
}

}}}
