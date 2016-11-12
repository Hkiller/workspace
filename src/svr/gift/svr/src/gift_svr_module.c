#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_stub_buff.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_sp.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "gift_svr.h"
#include "gift_svr_generator.h"
#include "gift_svr_generate_record.h"

extern char g_metalib_svr_gift_pro[];
static LPDRMETA gift_svr_load_meta(gd_app_context_t app, gd_app_module_t module, const char * str_meta);

EXPORT_DIRECTIVE
int gift_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    gift_svr_t gift_svr;
    set_logic_rsp_manage_t rsp_manage;
    mongo_cli_proxy_t db;
    cfg_t gift_def;
    LPDRMETA data_meta;
    uint32_t generate_record_count;
    float bucket_ratio = 1.5;
    const char * str_value;

    str_value = gd_app_arg_find(app, "--generate-record-count");
    if (str_value == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: generate-record-count not conrigured", gd_app_module_name(module));
        return -1;
    }
    else {
        generate_record_count = strtol(str_value, NULL, 10);
    }

    if ((str_value = gd_app_arg_find(app, "--bucket-ratio"))) {
        bucket_ratio = strtof(str_value, NULL);
    }

    str_value = cfg_get_string(gd_app_cfg(app), "gift-svr-def.meta", NULL);
    if (str_value == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: gift-svr-def.meta not configured.", gd_app_module_name(module));
        return -1;
    }

    data_meta = gift_svr_load_meta(app, module, str_value);
    if (data_meta == NULL) return -1;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    rsp_manage = set_logic_rsp_manage_find_nc(app, cfg_get_string(cfg, "rsp-manage", NULL));
    if (rsp_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: rsp-manage %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "rsp-manage", "default"));
        return -1;
    }

    db = mongo_cli_proxy_find_nc(app, cfg_get_string(cfg, "db", NULL));
    if (db == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: db %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "db", "default"));
        return -1;
    }

    gift_svr =
        gift_svr_create(
            app, gd_app_module_name(module),
            stub, rsp_manage, db,
            gd_app_alloc(app), gd_app_em(app));
    if (gift_svr == NULL) return -1;

    if (gift_svr_install_ops(gift_svr) != 0
        || gift_svr_start_tick(gift_svr) != 0
        )
    {
        gift_svr_free(gift_svr);
        return -1;
    }

    if (set_logic_rsp_build(
            gift_svr->m_rsp_manage,
            cfg_find_cfg(gd_app_cfg(app), "rsps"), (LPDRMETALIB)g_metalib_svr_gift_pro, gd_app_em(app)) != 0)
    {
        CPE_ERROR(gd_app_em(app), "%s: create: load rsps fail!", gd_app_module_name(module));
        gift_svr_free(gift_svr);
        return -1;
    }

    gift_svr->m_debug = cfg_get_int8(cfg, "debug", gift_svr->m_debug);

    gift_def = cfg_find_cfg(gd_app_cfg(app), "gift-svr-def");
    if (gift_def == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: gift-svr-def not configured.", gd_app_module_name(module));
        gift_svr_free(gift_svr);
        return -1;
    }

    if (gift_svr_generators_load(gift_svr, cfg_find_cfg(gd_app_cfg(app), "generators")) != 0) {
        gift_svr_free(gift_svr);
        return -1;
    }

    if (gift_svr_generate_record_init(gift_svr, data_meta, generate_record_count, bucket_ratio) != 0) {
        gift_svr_free(gift_svr);
        return -1;
    }

    CPE_INFO(
        gd_app_em(app),
        "%s: create: done. generate_record-size=%d, generate_record-count=%d, generate_record-buf=%.2fm, hash-buf=%.2fm\n"
        "dump meta lib:\n%s",
        gd_app_module_name(module),
        (int)dr_meta_size(aom_obj_mgr_meta(gift_svr->m_generate_record_mgr)),
        aom_obj_mgr_allocked_obj_count(gift_svr->m_generate_record_mgr),
        aom_obj_mgr_data_capacity(gift_svr->m_generate_record_mgr) / 1024.0f / 1024.0f,
        aom_obj_hash_table_buff_capacity(gift_svr->m_generate_record_hash) / 1024.0f / 1024.0f,
        dr_lib_dump(gd_app_tmp_buffer(app), mem_buffer_make_continuous(&gift_svr->m_record_metalib, 0), 4));

    return 0;
}

EXPORT_DIRECTIVE
void gift_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    gift_svr_t gift_svr;

    gift_svr = gift_svr_find_nc(app, gd_app_module_name(module));
    if (gift_svr) {
        gift_svr_free(gift_svr);
    }
}

static LPDRMETA gift_svr_load_meta(gd_app_context_t app, gd_app_module_t module, const char * str_meta) {
    dr_store_manage_t store_mgr;
    dr_store_t store;
    char const * sep;
    char lib_name[64];
    LPDRMETA meta;

    store_mgr = dr_store_manage_find_nc(app, NULL);
    if (store_mgr == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: store_mgr not exist!", gd_app_module_name(module));
        return NULL;
    }

    sep = strchr(str_meta, '.');
    if (sep == NULL || (sep - str_meta) > (sizeof(lib_name) - 1)) {
        CPE_ERROR(gd_app_em(app), "%s: pkg-meta %s format error or overflow!", gd_app_module_name(module), str_meta);
        return NULL;
    }
    memcpy(lib_name, str_meta, sep - str_meta);
    lib_name[sep - str_meta] = 0;

    store = dr_store_find(store_mgr, lib_name);
    if (store == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: metalib %s not exist in %s!",
            gd_app_module_name(module), lib_name, dr_store_manage_name(store_mgr));
        return NULL;
    }

    meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
    if (meta == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: metalib %s have no meta %s!",
            gd_app_module_name(module), lib_name, sep + 1);
        return NULL;
    }

    return meta;
}
