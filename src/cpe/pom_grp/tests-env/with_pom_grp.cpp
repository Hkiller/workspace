#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_meta_build.h"
#include "cpe/pom_grp/pom_grp_cfg.h"
#include "cpe/pom_grp/tests-env/with_pom_grp.hpp"

namespace cpe { namespace pom_grp { namespace testenv {

with_pom_grp::with_pom_grp() {
}

const char *
with_pom_grp::t_pom_grp_meta_dump(pom_grp_meta_t meta) {
    mem_buffer buffer;
    mem_buffer_init(&buffer, 0);

    write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);
    pom_grp_meta_dump((write_stream_t)&stream, meta, 0);
    stream_putc((write_stream_t)&stream, 0);

    const char * r = t_tmp_strdup((char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);

    return r;
}

pom_grp_meta_t
with_pom_grp::t_pom_grp_meta_create_by_cfg(const char * om_meta, const char * str_metalib, uint16_t page_size) {
    LPDRMETALIB metalib = 
        envOf<cpe::dr::testenv::with_dr>().t_create_metalib(
            str_metalib);
    EXPECT_TRUE(metalib) << "create metalib fail!";
    if (metalib == NULL) return NULL;

    return t_pom_grp_meta_create_by_cfg(om_meta, metalib, page_size);
}

pom_grp_meta_t
with_pom_grp::t_pom_grp_meta_create_by_cfg(const char * str_om_meta, LPDRMETALIB metalib, uint16_t page_size) {
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    cfg_t om_meta = envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(str_om_meta);
    EXPECT_TRUE(om_meta) << "parse om_meta fail!";
    if (om_meta == NULL) return NULL;

    om_meta = cfg_child_only(om_meta);
    EXPECT_TRUE(om_meta) << "om_meta format error, get child only fail!";
    if (om_meta == NULL) return NULL;

    pom_grp_meta_t meta =
        pom_grp_meta_build_from_cfg(t_tmp_allocrator(), page_size, om_meta, metalib, em);
    EXPECT_TRUE(meta) << "create pom_grp_meta fail!"; 
    if (meta == NULL) return NULL;

    return meta;
}

pom_grp_obj_mgr_t
with_pom_grp::t_pom_grp_obj_mgr_create_by_cfg(const char * om_meta, const char * str_metalib, size_t capacity, uint16_t page_size) {
    LPDRMETALIB metalib = 
        envOf<cpe::dr::testenv::with_dr>().t_create_metalib(
            str_metalib);
    EXPECT_TRUE(metalib) << "create metalib fail!";
    if (metalib == NULL) return NULL;

    return t_pom_grp_obj_mgr_create_by_cfg(om_meta, metalib, capacity, page_size);
}

pom_grp_obj_mgr_t
with_pom_grp::t_pom_grp_obj_mgr_create_by_cfg(
    const char * om_meta,
    LPDRMETALIB metalib, size_t capacity, uint16_t page_size)
{
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    void * buf = mem_alloc(t_tmp_allocrator(), capacity);
    EXPECT_TRUE(buf) << "malloc buf fail!";
    if (buf == NULL) return NULL;

    pom_grp_meta_t meta = t_pom_grp_meta_create_by_cfg(om_meta, metalib, page_size);
    if (meta == NULL) return NULL;

    int rv = pom_grp_obj_mgr_buf_init(
            metalib,
            meta,
            buf, capacity,
            em);

    EXPECT_TRUE(rv == 0) << "pom_grp_obj_mgr_buf_init fail!";
    if (rv != 0) return NULL;

    pom_grp_obj_mgr_t mgr = pom_grp_obj_mgr_create(t_allocrator(), buf, capacity, em);
    EXPECT_TRUE(mgr) << "om_mgr_obj_mgr create fail!";

    pom_grp_obj_mgr_set_auto_validate(mgr, 2);
    pom_mgr_set_auto_validate(pom_grp_obj_mgr_pom(mgr), 1);

    return mgr;
}

pom_grp_meta_t
with_pom_grp::t_pom_grp_meta_create_by_meta(const char * str_metalib, const char * metaname, uint16_t page_size) {
    LPDRMETALIB metalib = envOf<cpe::dr::testenv::with_dr>().t_create_metalib(str_metalib);
    EXPECT_TRUE(metalib) << "create metalib fail!";
    if (metalib == NULL) return NULL;

    LPDRMETA dr_meta = dr_lib_find_meta_by_name(metalib, metaname);
    EXPECT_TRUE(dr_meta) << "meta " << metaname << " not exist in metalib!";
    if (dr_meta == NULL) return NULL;

    return t_pom_grp_meta_create_by_meta(dr_meta, page_size);
}

pom_grp_meta_t
with_pom_grp::t_pom_grp_meta_create_by_meta(LPDRMETA dr_meta, uint16_t page_size) {
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    pom_grp_meta_t meta =
        pom_grp_meta_build_from_meta(t_tmp_allocrator(), page_size, dr_meta, em);
    EXPECT_TRUE(meta) << "create pom_grp_meta fail!"; 
    if (meta == NULL) return NULL;

    return meta;
}

pom_grp_obj_mgr_t
with_pom_grp::t_pom_grp_obj_mgr_create_by_meta(const char * str_metalib, const char * metaname, size_t capacity, uint16_t page_size) {
    LPDRMETALIB metalib = envOf<cpe::dr::testenv::with_dr>().t_create_metalib(str_metalib);
    EXPECT_TRUE(metalib) << "create metalib fail!";
    if (metalib == NULL) return NULL;

    LPDRMETA dr_meta = dr_lib_find_meta_by_name(metalib, metaname);
    EXPECT_TRUE(dr_meta) << "meta " << metaname << " not exist in metalib!";
    if (dr_meta == NULL) return NULL;

    return t_pom_grp_obj_mgr_create_by_meta(dr_meta, capacity, page_size);
}

pom_grp_obj_mgr_t
with_pom_grp::t_pom_grp_obj_mgr_create_by_meta(LPDRMETA dr_meta, size_t capacity, uint16_t page_size) {
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    void * buf = mem_alloc(t_tmp_allocrator(), capacity);
    EXPECT_TRUE(buf) << "malloc buf fail!";
    if (buf == NULL) return NULL;

    pom_grp_meta_t meta = t_pom_grp_meta_create_by_meta(dr_meta, page_size);
    if (meta == NULL) return NULL;

    int rv = pom_grp_obj_mgr_buf_init(dr_meta_owner_lib(dr_meta), meta, buf, capacity, em);
    EXPECT_TRUE(rv == 0) << "pom_grp_obj_mgr_buf_init fail!";
    if (rv != 0) return NULL;

    pom_grp_obj_mgr_t mgr = pom_grp_obj_mgr_create(t_allocrator(), buf, capacity, em);
    EXPECT_TRUE(mgr) << "om_mgr_obj_mgr create fail!";

    pom_grp_obj_mgr_set_auto_validate(mgr, 2);
    pom_mgr_set_auto_validate(pom_grp_obj_mgr_pom(mgr), 1);

    return mgr;
}

pom_grp_obj_t
with_pom_grp::t_pom_grp_obj_create(
    pom_grp_obj_mgr_t obj_mgr,
    const char * data)
{
    cfg_t cfg_data = envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(data);
    EXPECT_TRUE(cfg_data) << "parse cfg fail!";
    if (cfg_data == NULL) return NULL;

    return t_pom_grp_obj_create(obj_mgr, cfg_data);
}

pom_grp_obj_t
with_pom_grp::t_pom_grp_obj_create(
    pom_grp_obj_mgr_t obj_mgr,
    cfg_t data)
{
    pom_grp_obj_t obj = pom_grp_obj_alloc(obj_mgr);
    EXPECT_TRUE(obj) << "alloc obj fail!";
    if (obj == NULL) return NULL;

    if (t_pom_grp_obj_load(obj_mgr, obj, data) != 0) {
        pom_grp_obj_free(obj_mgr, obj);
        return NULL;
    }

    return obj;
}

int with_pom_grp::t_pom_grp_obj_load(
    pom_grp_obj_mgr_t obj_mgr,
    pom_grp_obj_t obj,
    const char * data)
{
    cfg_t cfg_data = envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(data);
    EXPECT_TRUE(cfg_data) << "parse cfg fail!";
    if (cfg_data == NULL) return -1;

    return t_pom_grp_obj_load(obj_mgr, obj, cfg_data);
}

int with_pom_grp::t_pom_grp_obj_load(
    pom_grp_obj_mgr_t obj_mgr,
    pom_grp_obj_t obj,
    cfg_t data)
{
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    return pom_grp_obj_cfg_load(obj, obj_mgr, data, em);
}

const char * with_pom_grp::t_pom_grp_obj_dump(
    pom_grp_obj_mgr_t obj_mgr,
    pom_grp_obj_t obj)
{
    mem_buffer buffer;
    mem_buffer_init(&buffer, 0);

    write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);
    pom_grp_obj_cfg_dump_to_stream((write_stream_t)&stream, obj_mgr, obj, 0);
    stream_putc((write_stream_t)&stream, 0);

    const char * r = t_tmp_strdup((char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);

    return r;
}

}}}
