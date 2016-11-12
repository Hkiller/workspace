#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "bpg_pkg_internal_ops.h"

static void bpg_pkg_manage_clear(nm_node_t node);

static cpe_hash_string_buf s_bpg_pkg_manage_default_name = CPE_HS_BUF_MAKE("bpg_pkg_manage");

struct nm_node_type s_nm_node_type_bpg_pkg_manage = {
    "usf_bpg_pkg_manage",
    bpg_pkg_manage_clear
};

bpg_pkg_manage_t
bpg_pkg_manage_create(
    gd_app_context_t app,
    const char * name,
    error_monitor_t em)
{
    bpg_pkg_manage_t mgr;
    nm_node_t mgr_node;

    assert(app);

    if (name == 0) name = cpe_hs_data((cpe_hash_string_t)&s_bpg_pkg_manage_default_name);
    if (em == 0) em = gd_app_em(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_pkg_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (bpg_pkg_manage_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = gd_app_alloc(app);
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_base_cvt = NULL;
    mgr->m_data_cvt = NULL;
    mgr->m_metalib_ref = NULL;
    mgr->m_pkg_debug_default_level = bpg_pkg_debug_none;
    mgr->m_op_buff = NULL;
    mgr->m_op_buff_capacity = 2048;
    mgr->m_zip_size_threshold = (uint32_t)(-1);

    mgr->m_metalib_basepkg_ref =
        dr_ref_create(
            dr_store_manage_default(mgr->m_app),
            BPG_BASEPKG_LIB_NAME);
    if (mgr->m_metalib_basepkg_ref == NULL) {
        CPE_ERROR(em, "%s: create: create basepkg_ref fail!", name);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_cmd_info_by_cmd,
            mgr->m_alloc,
            (cpe_hash_fun_t) bpg_pkg_cmd_info_cmd_hash,
            (cpe_hash_eq_t) bpg_pkg_cmd_info_cmd_eq,
            CPE_HASH_OBJ2ENTRY(bpg_pkg_cmd_info, m_hh_for_cmd),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init bpg_cmd_info_by_cmd hash table fail!", name);
        dr_ref_free(mgr->m_metalib_basepkg_ref);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_cmd_info_by_name,
            mgr->m_alloc,
            (cpe_hash_fun_t) bpg_pkg_cmd_info_name_hash,
            (cpe_hash_eq_t) bpg_pkg_cmd_info_name_eq,
            CPE_HASH_OBJ2ENTRY(bpg_pkg_cmd_info, m_hh_for_name),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init bpg_cmd_info_by_name hash table fail!", name);
        cpe_hash_table_fini(&mgr->m_cmd_info_by_cmd);
        dr_ref_free(mgr->m_metalib_basepkg_ref);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_pkg_debug_infos,
            mgr->m_alloc,
            (cpe_hash_fun_t) bpg_pkg_debug_info_hash,
            (cpe_hash_eq_t) bpg_pkg_debug_info_cmp,
            CPE_HASH_OBJ2ENTRY(bpg_pkg_debug_info, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init bpg_pkg_debug_info hash table fail!", name);
        cpe_hash_table_fini(&mgr->m_cmd_info_by_name);
        cpe_hash_table_fini(&mgr->m_cmd_info_by_cmd);
        dr_ref_free(mgr->m_metalib_basepkg_ref);
        nm_node_free(mgr_node);
        return NULL;
    }

    mem_buffer_init(&mgr->m_dump_buff, mgr->m_alloc);
    mem_buffer_init(&mgr->m_zip_buff, mgr->m_alloc);

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_pkg_manage);

    return mgr;
}

static void bpg_pkg_manage_clear(nm_node_t node) {
    bpg_pkg_manage_t mgr;
    mgr = (bpg_pkg_manage_t)nm_node_data(node);

    if (mgr->m_data_cvt) {
        dr_cvt_free(mgr->m_data_cvt);
        mgr->m_data_cvt = NULL;
    }

    if (mgr->m_base_cvt) {
        dr_cvt_free(mgr->m_base_cvt);
        mgr->m_base_cvt = NULL;
    }

    if (mgr->m_metalib_basepkg_ref) {
        dr_ref_free(mgr->m_metalib_basepkg_ref);
        mgr->m_metalib_basepkg_ref = NULL;
    }

    if (mgr->m_metalib_ref) {
        dr_ref_free(mgr->m_metalib_ref);
        mgr->m_metalib_ref = NULL;
    }

    bpg_pkg_cmd_info_free_all(mgr);
    cpe_hash_table_fini(&mgr->m_cmd_info_by_name);
    cpe_hash_table_fini(&mgr->m_cmd_info_by_cmd);

    bpg_pkg_debug_info_free_all(mgr);
    cpe_hash_table_fini(&mgr->m_pkg_debug_infos);

    mem_buffer_clear(&mgr->m_dump_buff);
    mem_buffer_clear(&mgr->m_zip_buff);

    if (mgr->m_op_buff) {
        mem_free(gd_app_alloc(mgr->m_app), mgr->m_op_buff);
        mgr->m_op_buff = NULL;
    }
}

void bpg_pkg_manage_free(bpg_pkg_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_pkg_manage) return;
    nm_node_free(mgr_node);
}

bpg_pkg_manage_t
bpg_pkg_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) name = (cpe_hash_string_t)&s_bpg_pkg_manage_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_pkg_manage) return NULL;
    return (bpg_pkg_manage_t)nm_node_data(node);
}

bpg_pkg_manage_t
bpg_pkg_manage_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) return bpg_pkg_manage_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_pkg_manage) return NULL;
    return (bpg_pkg_manage_t)nm_node_data(node);
}

bpg_pkg_manage_t
bpg_pkg_manage_default(gd_app_context_t app) {
    return bpg_pkg_manage_find(app, (cpe_hash_string_t)&s_bpg_pkg_manage_default_name);
}

gd_app_context_t bpg_pkg_manage_app(bpg_pkg_manage_t mgr) {
    return mgr->m_app;
}

const char * bpg_pkg_manage_name(bpg_pkg_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
bpg_pkg_manage_name_hs(bpg_pkg_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}


int bpg_pkg_manage_set_base_cvt(bpg_pkg_manage_t mgr, const char * cvt) {
    dr_cvt_t new_cvt;

    if (mgr->m_base_cvt && strcmp(dr_cvt_name(mgr->m_base_cvt), cvt) == 0) return 0;

    new_cvt = dr_cvt_create(mgr->m_app, cvt);
    if (new_cvt == NULL) return -1;

    if (mgr->m_base_cvt) dr_cvt_free(mgr->m_base_cvt);

    mgr->m_base_cvt = new_cvt;
    return 0;
}

dr_cvt_t bpg_pkg_manage_base_cvt(bpg_pkg_manage_t mgr) {
    return mgr->m_base_cvt;
}

const char * bpg_pkg_manage_base_cvt_name(bpg_pkg_manage_t mgr) {
    return mgr->m_base_cvt ? dr_cvt_name(mgr->m_base_cvt) : "";
}

int bpg_pkg_manage_set_data_cvt(bpg_pkg_manage_t mgr, const char * cvt) {
    dr_cvt_t new_cvt;

    if (mgr->m_data_cvt && strcmp(dr_cvt_name(mgr->m_data_cvt), cvt) == 0) return 0;

    new_cvt = dr_cvt_create(mgr->m_app, cvt);
    if (new_cvt == NULL) return -1;

    if (mgr->m_data_cvt) dr_cvt_free(mgr->m_data_cvt);

    mgr->m_data_cvt = new_cvt;
    return 0;
}

dr_cvt_t bpg_pkg_manage_data_cvt(bpg_pkg_manage_t mgr) {
    return mgr->m_data_cvt;
}

const char * bpg_pkg_manage_data_cvt_name(bpg_pkg_manage_t mgr) {
    return mgr->m_data_cvt ? dr_cvt_name(mgr->m_data_cvt) : "";
}

int bpg_pkg_manage_set_data_metalib(bpg_pkg_manage_t mgr, const char * metalib_name) {
    assert(mgr);
    assert(metalib_name);

    if (mgr->m_metalib_ref) dr_ref_free(mgr->m_metalib_ref);

    mgr->m_metalib_ref =
        dr_ref_create(
            dr_store_manage_default(mgr->m_app),
            metalib_name);
    if (mgr->m_metalib_ref == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: set metalib %s, create dr_ref fail!", 
            bpg_pkg_manage_name(mgr), metalib_name);
        return -1;
    }

    return 0;
}

int bpg_pkg_manage_add_cmd(bpg_pkg_manage_t mgr, uint32_t cmd, const char * name) {
    LPDRMETA meta;

    if (mgr->m_metalib_ref == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: add cmd %d => %s, data metalib not exist!", 
            bpg_pkg_manage_name(mgr), cmd, name);
        return -1;
    }

    if (dr_ref_lib(mgr->m_metalib_ref) == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: add cmd %d => %s, data metalib no lib!", 
            bpg_pkg_manage_name(mgr), cmd, name);
        return -1;
    }

    meta = dr_lib_find_meta_by_name(dr_ref_lib(mgr->m_metalib_ref), name);
    if (meta == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: add cmd %d => %s, meta not exist in lib!", 
            bpg_pkg_manage_name(mgr), cmd, name);
        return -1;
    }

    if (bpg_pkg_cmd_info_create(mgr, cmd, meta) == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: add cmd %d => %s, create fail!",
            bpg_pkg_manage_name(mgr), cmd, name);
        return -1;
    }

    return 0;
}

int bpg_pkg_manage_add_cmd_by_meta(bpg_pkg_manage_t mgr, const char * name) {
    LPDRMETA cmd_meta;
    int i, count;

    if (mgr->m_metalib_ref == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: add cmd by meta %s, data metalib not exist!", 
            bpg_pkg_manage_name(mgr), name);
        return -1;
    }

    if (dr_ref_lib(mgr->m_metalib_ref) == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: add cmd by meta %s, data metalib no lib!", 
            bpg_pkg_manage_name(mgr), name);
        return -1;
    }

    cmd_meta = dr_lib_find_meta_by_name(dr_ref_lib(mgr->m_metalib_ref), name);
    if (cmd_meta == NULL) {
        CPE_ERROR(
            mgr->m_em, "bpg_pkg_manage %s: add cmd by meta %s, meta not exist in lib!", 
            bpg_pkg_manage_name(mgr), name);
        return -1;
    }

    count = dr_meta_entry_num(cmd_meta);
    for(i = 0; i < count; ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(cmd_meta, i);
        LPDRMETA cmd_data_meta = dr_entry_ref_meta(entry);
        if (cmd_data_meta == NULL) {
            CPE_ERROR(
                mgr->m_em, "bpg_pkg_manage %s: add cmd by meta %s, entry %s have no ref meta!", 
                bpg_pkg_manage_name(mgr), name, dr_entry_name(entry));
            return -1;
        }

        if (dr_entry_id(entry) == -1) {
            CPE_ERROR(
                mgr->m_em, "bpg_pkg_manage %s: add cmd by meta %s, entry %s have no id!", 
                bpg_pkg_manage_name(mgr), name, dr_entry_name(entry));
            return -1;
        }

        if (bpg_pkg_cmd_info_create(mgr, dr_entry_id(entry), cmd_data_meta) == NULL) {
            CPE_ERROR(
                mgr->m_em, "bpg_pkg_manage %s: add cmd by meta %s, create cmd %d ==> %s fail!",
                bpg_pkg_manage_name(mgr), name, dr_entry_id(entry), dr_meta_name(cmd_data_meta));
            return -1;
        }
    }

    return 0;
}

LPDRMETALIB bpg_pkg_manage_data_metalib(bpg_pkg_manage_t mgr) {
    return mgr->m_metalib_ref ? dr_ref_lib(mgr->m_metalib_ref) : NULL;
}

const char * bpg_pkg_manage_data_metalib_name(bpg_pkg_manage_t mgr) {
    return mgr->m_metalib_ref 
        ? dr_lib_name(dr_ref_lib(mgr->m_metalib_ref))
        : "???";
}

LPDRMETALIB bpg_pkg_manage_basepkg_metalib(bpg_pkg_manage_t mgr) {
    return mgr->m_metalib_basepkg_ref ? dr_ref_lib(mgr->m_metalib_basepkg_ref) : NULL;
}

LPDRMETA bpg_pkg_manage_basepkg_head_meta(bpg_pkg_manage_t mgr) {
    LPDRMETALIB metalib;

    metalib = bpg_pkg_manage_basepkg_metalib(mgr);
    return metalib ? dr_lib_find_meta_by_name(metalib, "basepkg_head") : NULL;
}

LPDRMETA bpg_pkg_manage_basepkg_meta(bpg_pkg_manage_t mgr) {
    LPDRMETALIB metalib;

    metalib = bpg_pkg_manage_basepkg_metalib(mgr);
    return metalib ? dr_lib_find_meta_by_name(metalib, "basepkg") : NULL;
}

LPDRMETA bpg_pkg_manage_find_meta_by_cmd(bpg_pkg_manage_t mgr, uint32_t cmd) {
    struct bpg_pkg_cmd_info * r;
    struct bpg_pkg_cmd_info key;
    key.m_cmd = cmd;

    r = (struct bpg_pkg_cmd_info *)cpe_hash_table_find(&mgr->m_cmd_info_by_cmd, &key);
    return r ? r->m_cmd_meta : NULL;
}

int bpg_pkg_find_cmd_from_meta_name(uint32_t * cmd, bpg_pkg_manage_t mgr, const char * meta_name) {
    struct bpg_pkg_cmd_info * r;
    struct bpg_pkg_cmd_info key;
    key.m_name = meta_name;

    r = (struct bpg_pkg_cmd_info *)cpe_hash_table_find(&mgr->m_cmd_info_by_name, &key);
    if (r) {
        *cmd = r->m_cmd;
        return 0;
    }
    else {
        return -1;
    }
}

int bpg_pkg_manage_set_op_buff_capacity(bpg_pkg_manage_t mgr, size_t buf_size) {
    mgr->m_op_buff_capacity = buf_size;
    if (mgr->m_op_buff) {
        mem_free(gd_app_alloc(mgr->m_app), mgr->m_op_buff);
        mgr->m_op_buff = NULL;
    }

    return 0;
}

void * bpg_pkg_op_buff(bpg_pkg_manage_t mgr) {
    if (mgr->m_op_buff == NULL) {
        mgr->m_op_buff = mem_alloc(gd_app_alloc(mgr->m_app), mgr->m_op_buff_capacity);
    }

    return mgr->m_op_buff;
}

uint32_t bpg_pkg_zip_size_threshold(bpg_pkg_manage_t mgr) {
    return mgr->m_zip_size_threshold;
}

void bpg_pkg_set_zip_size_threshold(bpg_pkg_manage_t mgr, uint32_t threaded) {
    mgr->m_zip_size_threshold = threaded;
}
