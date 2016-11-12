#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_sp.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "friend_svr_ops.h"

extern char g_metalib_svr_friend_pro[];
static int friend_svr_load_def(friend_svr_t svr, cfg_t cfg);
static int friend_svr_build_record_meta(friend_svr_t svr);

EXPORT_DIRECTIVE
int friend_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    set_logic_sp_t set_sp;
    friend_svr_t friend_svr;
    set_logic_rsp_manage_t rsp_manage;
    mongo_cli_proxy_t db;
    cfg_t friend_def;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    set_sp = set_logic_sp_find_nc(app, cfg_get_string(cfg, "set-sp", NULL));
    if (set_sp == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-sp %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-sp", "default"));
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

    friend_svr =
        friend_svr_create(
            app, gd_app_module_name(module),
            stub, set_sp, rsp_manage, db,
            gd_app_alloc(app), gd_app_em(app));
    if (friend_svr == NULL) return -1;

    if (set_logic_rsp_build(
            friend_svr->m_rsp_manage,
            cfg_find_cfg(gd_app_cfg(app), "rsps"), (LPDRMETALIB)g_metalib_svr_friend_pro, gd_app_em(app)) != 0)
    {
        CPE_ERROR(gd_app_em(app), "%s: create: load rsps fail!", gd_app_module_name(module));
        friend_svr_free(friend_svr);
        return -1;
    }

    friend_svr->m_debug = cfg_get_int8(cfg, "debug", friend_svr->m_debug);

    friend_def = cfg_find_cfg(gd_app_cfg(app), "friend-svr-def");
    if (friend_def == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: friend-svr-def not configured.", gd_app_module_name(module));
        friend_svr_free(friend_svr);
        return -1;
    }

    if (friend_svr_load_def(friend_svr, friend_def) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load friend-svr-def fail.", gd_app_module_name(module));
        friend_svr_free(friend_svr);
        return -1;
    }

    if (friend_svr_build_record_meta(friend_svr) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: build record meta fail.", gd_app_module_name(module));
        friend_svr_free(friend_svr);
        return -1;
    }


    if (friend_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void friend_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    friend_svr_t friend_svr;

    friend_svr = friend_svr_find_nc(app, gd_app_module_name(module));
    if (friend_svr) {
        friend_svr_free(friend_svr);
    }
}

static LPDRMETA friend_svr_load_meta(friend_svr_t svr, const char * str_meta) {
    dr_store_manage_t store_mgr;
    dr_store_t store;
    char const * sep;
    char lib_name[64];
    LPDRMETA meta;

    store_mgr = dr_store_manage_find_nc(svr->m_app, NULL);
    if (store_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: store_mgr not exist!", friend_svr_name(svr));
        return NULL;
    }

    sep = strchr(str_meta, '.');
    if (sep == NULL || (sep - str_meta) > (sizeof(lib_name) - 1)) {
        CPE_ERROR(svr->m_em, "%s: pkg-meta %s format error or overflow!", friend_svr_name(svr), str_meta);
        return NULL;
    }
    memcpy(lib_name, str_meta, sep - str_meta);
    lib_name[sep - str_meta] = 0;

    store = dr_store_find(store_mgr, lib_name);
    if (store == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: metalib %s not exist in %s!",
            friend_svr_name(svr), lib_name, dr_store_manage_name(store_mgr));
        return NULL;
    }

    meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
    if (meta == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: metalib %s have no meta %s!",
            friend_svr_name(svr), lib_name, sep + 1);
        return NULL;
    }

    return meta;
}

static int friend_svr_load_def(friend_svr_t svr, cfg_t cfg) {
    const char * str_meta;
    const char * str_fuid_entry;
    const char * runing_mode;

    str_meta = cfg_get_string(cfg, "meta", NULL);
    if (str_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: friend-svr-def.meta not configured.", friend_svr_name(svr));
        return -1;
    }

    str_fuid_entry = cfg_get_string(cfg, "user-id", NULL);
    if (str_fuid_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: friend-svr-def.user-id not configured.", friend_svr_name(svr));
        return -1;
    }

    svr->m_data_meta = friend_svr_load_meta(svr, str_meta);
    if (svr->m_data_meta == NULL) return -1;

    svr->m_data_size = dr_meta_size(svr->m_data_meta);

    svr->m_data_fuid_entry = dr_meta_find_entry_by_name(svr->m_data_meta, str_fuid_entry);
    if (svr->m_data_fuid_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: entry %s not exist in meta %s.", friend_svr_name(svr), str_fuid_entry, str_meta);
        return -1;
    }

    svr->m_data_fuid_start_pos = dr_entry_data_start_pos(svr->m_data_fuid_entry, 0);

    runing_mode = cfg_get_string(cfg, "runing-mode", NULL);
    if (runing_mode == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: runing-mode not configured.", friend_svr_name(svr));
        return -1;
    }

    if (strcmp(runing_mode, "one-way") == 0) {
        svr->m_runing_mode = friend_svr_runing_mode_one_way;
    }
    else if (strcmp(runing_mode, "ack") == 0) {
        svr->m_runing_mode = friend_svr_runing_mode_ack;
    }
    else {
        CPE_ERROR(svr->m_em, "%s: create: runing-mode %s is unknown.", friend_svr_name(svr), runing_mode);
        return -1;
    }

    return 0;
}

static int friend_svr_build_record_meta(friend_svr_t svr) {
    struct DRInBuildMetaLib * in_build_lib;
    struct DRInBuildMeta * in_build_meta;
    struct DRInBuildMetaEntry *  in_build_entry;
    LPDRMETA src_record_meta;
    LPDRMETALIB metalib;
    LPDRMETAENTRY data_first_entry;

    src_record_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_friend_pro, "svr_friend_record");
    if (src_record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: meta svr_friend_record not exist", friend_svr_name(svr));
        return -1;
    }

    in_build_lib = dr_inbuild_create_lib();
    if (in_build_lib == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: create builder fail", friend_svr_name(svr));
        return -1;
    }

    /*数据meta */
    in_build_meta = dr_inbuild_metalib_add_meta(in_build_lib);
    if (in_build_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: create meta record in metalib fail", friend_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_meta_set_type(in_build_meta, CPE_DR_TYPE_STRUCT);
    dr_inbuild_meta_set_name(in_build_meta, "record");
    dr_inbuild_meta_set_align(in_build_meta, dr_meta_align(svr->m_data_meta));

    if (dr_inbuild_meta_copy_entrys(in_build_meta, src_record_meta) != 0
        || dr_inbuild_meta_copy_entrys(in_build_meta, svr->m_data_meta) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: copy entries fail", friend_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    /*记录集合的meta */
    in_build_meta = dr_inbuild_metalib_add_meta(in_build_lib);
    if (in_build_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: create meta record_list in metalib fail", friend_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_meta_set_type(in_build_meta, CPE_DR_TYPE_STRUCT);
    dr_inbuild_meta_set_name(in_build_meta, "record_list");
    dr_inbuild_meta_set_align(in_build_meta, dr_meta_align(svr->m_data_meta));

    in_build_entry = dr_inbuild_meta_add_entry(in_build_meta);
    if (in_build_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: add entry in record_list fail", friend_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_entry_set_type(in_build_entry, "uint32");
    dr_inbuild_entry_set_name(in_build_entry, "record_count");
    dr_inbuild_entry_set_id(in_build_entry, 1);

    in_build_entry = dr_inbuild_meta_add_entry(in_build_meta);
    if (in_build_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: add entry in record_list fail", friend_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_entry_set_type(in_build_entry, "record");
    dr_inbuild_entry_set_name(in_build_entry, "records");
    dr_inbuild_entry_set_array_count(in_build_entry, 0);
    dr_inbuild_entry_set_array_refer(in_build_entry, "record_count");
    dr_inbuild_entry_set_id(in_build_entry, 2);

    /*创建lib */
    if (dr_inbuild_build_lib(&svr->m_record_metalib, in_build_lib, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: build record metalib fail", friend_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    metalib = (LPDRMETALIB)mem_buffer_make_continuous(&svr->m_record_metalib, 0);
    svr->m_record_meta = dr_lib_find_meta_by_name(metalib, "record");
    if (svr->m_record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: find record in metalib fail", friend_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    /*初始化record相关的数据 */
    /*    id*/
    svr->m_record_id_entry = dr_meta_find_entry_by_name(svr->m_record_meta, "_id");
    assert(svr->m_record_id_entry);
    svr->m_record_id_start_pos = dr_entry_data_start_pos(svr->m_record_id_entry, 0);
    svr->m_record_id_capacity = dr_entry_element_size(svr->m_record_id_entry);

    /*    uid*/
    svr->m_record_uid_entry = dr_meta_find_entry_by_name(svr->m_record_meta, "user_id");
    assert(svr->m_record_uid_entry);
    svr->m_record_uid_start_pos = dr_entry_data_start_pos(svr->m_record_uid_entry, 0);

    /*    fuid*/
    svr->m_record_fuid_entry = dr_meta_find_entry_by_name(svr->m_record_meta, dr_entry_name(svr->m_data_fuid_entry));
    assert(svr->m_record_fuid_entry);
    svr->m_record_fuid_start_pos = dr_entry_data_start_pos(svr->m_record_fuid_entry, 0);

    /*    first entry*/
    data_first_entry = dr_meta_find_entry_by_name(svr->m_record_meta, dr_entry_name(dr_meta_entry_at(svr->m_data_meta, 0)));
    assert(data_first_entry);
    svr->m_record_data_start_pos = dr_entry_data_start_pos(data_first_entry, 0);

    if (dr_meta_size(svr->m_record_meta) != dr_meta_size(src_record_meta) + dr_meta_size(svr->m_data_meta)) {
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, svr->m_alloc);

        CPE_ERROR(
            svr->m_em, "%s: create: build record meta: data size mismatch:"
            " svr_friend_record.size=%d, %s.size=%d, record.size=%d\n%s",
            friend_svr_name(svr), (int)dr_meta_size(src_record_meta),
            dr_meta_name(svr->m_data_meta), (int)dr_meta_size(svr->m_data_meta),
            (int)dr_meta_size(svr->m_record_meta),
            dr_lib_dump(&buffer, metalib, 4));

        dr_inbuild_free_lib(in_build_lib);

        mem_buffer_clear(&buffer);

        return -1;
    }

    /*初始化record_list相关的数据 */
    svr->m_record_list_meta = dr_lib_find_meta_by_name(metalib, "record_list");
    assert(svr->m_record_list_meta);

    svr->m_record_size = dr_meta_size(svr->m_record_meta);
    svr->m_record_list_count_entry = dr_meta_find_entry_by_name(svr->m_record_list_meta, "record_count");
    assert(svr->m_record_list_count_entry);
    svr->m_record_list_data_entry = dr_meta_find_entry_by_name(svr->m_record_list_meta, "records");
    assert(svr->m_record_list_data_entry);

    dr_inbuild_free_lib(in_build_lib);

    return 0;
}

