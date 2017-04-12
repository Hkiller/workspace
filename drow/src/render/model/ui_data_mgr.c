#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/cache/ui_cache_manager.h"
#include "ui_data_mgr_i.h"
#include "ui_data_evt_collector_i.h"
#include "ui_data_language_i.h"
#include "ui_data_src_i.h"
#include "ui_data_src_user_i.h"
#include "ui_data_src_res_i.h"
#include "ui_data_src_src_i.h"
#include "ui_data_module_i.h"
#include "ui_data_sprite_i.h"
#include "ui_data_layout_i.h"
#include "ui_data_action_i.h"

extern char g_metalib_render_model[];
extern void ui_data_mgr_product_init(ui_data_mgr_t mgr);

#define UI_DATA_MGR_LOAD_META(__arg, __name) \
    mgr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)(void*)g_metalib_render_model, __name); \
    assert(mgr-> __arg)

static void ui_data_mgr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_data_mgr = {
    "ui_data_mgr",
    ui_data_mgr_clear
};

ui_data_mgr_t
ui_data_mgr_create(gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em, ui_cache_manager_t cache_mgr) {
    ui_data_mgr_t mgr;
    nm_node_t mgr_node;

    assert(app);

    if (name == NULL) name = "ui_data_mgr";

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_data_mgr));
    if (mgr_node == NULL) return NULL;

    mgr = (ui_data_mgr_t)nm_node_data(mgr_node);

    mgr->m_app = app; 
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    ui_data_mgr_product_init(mgr);
    mem_buffer_init(&mgr->m_dump_buffer, alloc);
    mgr->m_active_language = NULL;
    mgr->m_cache_mgr = cache_mgr;
    
    UI_DATA_MGR_LOAD_META(m_meta_collision, "ui_collision");
    UI_DATA_MGR_LOAD_META(m_meta_img_block, "ui_img_block");
    UI_DATA_MGR_LOAD_META(m_meta_frame, "ui_frame");
    UI_DATA_MGR_LOAD_META(m_meta_frame_img, "ui_img_ref");
    UI_DATA_MGR_LOAD_META(m_meta_actor, "ui_actor");
    UI_DATA_MGR_LOAD_META(m_meta_actor_layer, "ui_actor_layer");
    UI_DATA_MGR_LOAD_META(m_meta_actor_frame, "ui_actor_frame");
    UI_DATA_MGR_LOAD_META(m_meta_control, "ui_control");
    UI_DATA_MGR_LOAD_META(m_meta_control_anim, "ui_control_anim");
    UI_DATA_MGR_LOAD_META(m_meta_control_anim_frame, "ui_control_anim_frame");
    UI_DATA_MGR_LOAD_META(m_meta_control_addition, "ui_control_addition");
    UI_DATA_MGR_LOAD_META(m_meta_object_url, "ui_object_url");
    UI_DATA_MGR_LOAD_META(m_meta_control_object_url, "ui_control_object_url");

    mgr->m_computer = xcomputer_create(alloc, em);
    if (mgr->m_computer == NULL) {
        nm_node_free(mgr_node);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &mgr->m_srcs_by_name,
            alloc,
            (cpe_hash_fun_t) ui_data_src_hash,
            (cpe_hash_eq_t) ui_data_src_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_src, m_hh_for_name),
            -1) != 0)
    {
        mem_buffer_clear(&mgr->m_dump_buffer);
        xcomputer_free(mgr->m_computer);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_srcs_by_id,
            alloc,
            (cpe_hash_fun_t) ui_data_src_id_hash,
            (cpe_hash_eq_t) ui_data_src_id_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_src, m_hh_for_id),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_srcs_by_name);
        mem_buffer_clear(&mgr->m_dump_buffer);
        xcomputer_free(mgr->m_computer);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_img_blocks,
            alloc,
            (cpe_hash_fun_t) ui_data_img_block_hash,
            (cpe_hash_eq_t) ui_data_img_block_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_img_block, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_srcs_by_name);
        cpe_hash_table_fini(&mgr->m_srcs_by_id);
        mem_buffer_clear(&mgr->m_dump_buffer);
        xcomputer_free(mgr->m_computer);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_frames,
            alloc,
            (cpe_hash_fun_t) ui_data_frame_hash,
            (cpe_hash_eq_t) ui_data_frame_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_frame, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_img_blocks);
        cpe_hash_table_fini(&mgr->m_srcs_by_name);
        cpe_hash_table_fini(&mgr->m_srcs_by_id);
        mem_buffer_clear(&mgr->m_dump_buffer);
        xcomputer_free(mgr->m_computer);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_actors,
            alloc,
            (cpe_hash_fun_t) ui_data_actor_hash,
            (cpe_hash_eq_t) ui_data_actor_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_actor, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_frames);
        cpe_hash_table_fini(&mgr->m_img_blocks);
        cpe_hash_table_fini(&mgr->m_srcs_by_name);
        cpe_hash_table_fini(&mgr->m_srcs_by_id);
        mem_buffer_clear(&mgr->m_dump_buffer);
        xcomputer_free(mgr->m_computer);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_controls_by_id,
            alloc,
            (cpe_hash_fun_t) ui_data_control_hash,
            (cpe_hash_eq_t) ui_data_control_eq,
            CPE_HASH_OBJ2ENTRY(ui_data_control, m_hh_for_mgr),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_actors);
        cpe_hash_table_fini(&mgr->m_frames);
        cpe_hash_table_fini(&mgr->m_img_blocks);
        cpe_hash_table_fini(&mgr->m_srcs_by_name);
        cpe_hash_table_fini(&mgr->m_srcs_by_id);
        mem_buffer_clear(&mgr->m_dump_buffer);
        xcomputer_free(mgr->m_computer);
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_src_root = ui_data_src_create_i(mgr, NULL, ui_data_src_type_dir, "");
    if (mgr->m_src_root == NULL) {
        CPE_ERROR(em, "ui_data_mgr_create: alloc root fail!");
        cpe_hash_table_fini(&mgr->m_controls_by_id);
        cpe_hash_table_fini(&mgr->m_actors);
        cpe_hash_table_fini(&mgr->m_frames);
        cpe_hash_table_fini(&mgr->m_img_blocks);
        cpe_hash_table_fini(&mgr->m_srcs_by_name);
        cpe_hash_table_fini(&mgr->m_srcs_by_id);
        mem_buffer_clear(&mgr->m_dump_buffer);
        xcomputer_free(mgr->m_computer);
        nm_node_free(mgr_node);
        return NULL;
    }
    
    TAILQ_INIT(&mgr->m_languages);
    TAILQ_INIT(&mgr->m_evt_collectors);
    TAILQ_INIT(&mgr->m_free_src_users);
    TAILQ_INIT(&mgr->m_free_src_ress);
    TAILQ_INIT(&mgr->m_free_src_srcs);
        
    nm_node_set_type(mgr_node, &s_nm_node_type_ui_data_mgr);

    return mgr;
}

static void ui_data_mgr_clear(nm_node_t node) {
    ui_data_mgr_t mgr = nm_node_data(node);

    while(!TAILQ_EMPTY(&mgr->m_languages)) {
        ui_data_language_free(TAILQ_FIRST(&mgr->m_languages));
    }

    while(!TAILQ_EMPTY(&mgr->m_evt_collectors)) {
        ui_data_evt_collector_free(TAILQ_FIRST(&mgr->m_evt_collectors));
    }

    ui_data_src_free(mgr->m_src_root);
    mgr->m_src_root = NULL;

	assert(cpe_hash_table_count(&mgr->m_controls_by_id) == 0);
	cpe_hash_table_fini(&mgr->m_controls_by_id);

	assert(cpe_hash_table_count(&mgr->m_img_blocks) == 0);
	cpe_hash_table_fini(&mgr->m_img_blocks);

    assert(cpe_hash_table_count(&mgr->m_frames) == 0);
    cpe_hash_table_fini(&mgr->m_frames);

    assert(cpe_hash_table_count(&mgr->m_actors) == 0);
    cpe_hash_table_fini(&mgr->m_actors);

    assert(cpe_hash_table_count(&mgr->m_srcs_by_name) == 0);
    cpe_hash_table_fini(&mgr->m_srcs_by_name);

    assert(cpe_hash_table_count(&mgr->m_srcs_by_id) == 0);
    cpe_hash_table_fini(&mgr->m_srcs_by_id);

    xcomputer_free(mgr->m_computer);
    mgr->m_computer = NULL;
    
    while(!TAILQ_EMPTY(&mgr->m_free_src_users)) {
        ui_data_src_user_real_free(TAILQ_FIRST(&mgr->m_free_src_users));
    }

    while(!TAILQ_EMPTY(&mgr->m_free_src_ress)) {
        ui_data_src_res_real_free(TAILQ_FIRST(&mgr->m_free_src_ress));
    }

    while(!TAILQ_EMPTY(&mgr->m_free_src_srcs)) {
        ui_data_src_src_real_free(TAILQ_FIRST(&mgr->m_free_src_srcs));
    }
    
    mem_buffer_clear(&mgr->m_dump_buffer);
}

gd_app_context_t ui_data_mgr_app(ui_data_mgr_t mgr) {
    return mgr->m_app;
}

void ui_data_mgr_free(ui_data_mgr_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_ui_data_mgr) return;
    nm_node_free(mgr_node);
}

ui_data_mgr_t
ui_data_mgr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_data_mgr) return NULL;
    return (ui_data_mgr_t)nm_node_data(node);
}

ui_data_mgr_t
ui_data_mgr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "ui_data_mgr";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_data_mgr) return NULL;
    return (ui_data_mgr_t)nm_node_data(node);
}

const char * ui_data_mgr_name(ui_data_mgr_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

ui_cache_manager_t ui_data_mgr_cache_mgr(ui_data_mgr_t mgr) {
    return mgr->m_cache_mgr;
}

ui_data_src_t ui_data_mgr_src_root(ui_data_mgr_t mgr) {
    return mgr->m_src_root;
}

int ui_data_mgr_set_root(ui_data_mgr_t mgr, const char * root) {
    return ui_data_src_set_data(mgr->m_src_root, root);
}

LPDRMETA ui_data_mgr_meta_object_url(ui_data_mgr_t mgr) {
    return mgr->m_meta_object_url;
}

LPDRMETA ui_data_mgr_meta_control_object_url(ui_data_mgr_t mgr) {
    return mgr->m_meta_control_object_url;
}

int ui_data_mgr_register_type(
    ui_data_mgr_t mgr, ui_data_src_type_t type,
    ui_data_product_create_fun_t product_create, void * product_create_ctx,
    ui_data_product_free_fun_t product_free, void * product_free_ctx,
    product_using_src_update_using_fun_t update_usings)
{
    ui_product_type_t product_type;

    if ((uint8_t)type >= (uint8_t)UI_DATA_SRC_TYPE_MAX || (uint8_t)type < (uint8_t)UI_DATA_SRC_TYPE_MIN) {
        APP_CTX_ERROR(
            mgr->m_app, "%s: register type %d: type is unknown!", ui_data_mgr_name(mgr), type);
        return -1;
    }

    product_type = ui_product_type_of(mgr, type);
    if (product_type->product_create != NULL) {
        APP_CTX_ERROR(
            mgr->m_app, "%s: register type %d: type is already registered!", ui_data_mgr_name(mgr), type);
        return -1;
    }

    product_type->product_create_ctx = product_create_ctx;
    product_type->product_create = product_create;
    product_type->product_free_ctx = product_free_ctx;
    product_type->product_free = product_free;
    product_type->product_update_usings = update_usings;

    return 0;
}

int ui_data_mgr_unregister_type(ui_data_mgr_t mgr, ui_data_src_type_t type) {
    ui_product_type_t product_type;

    if ((uint8_t)type >= (uint8_t)UI_DATA_SRC_TYPE_MAX || (uint8_t)type < (uint8_t)UI_DATA_SRC_TYPE_MIN) {
        APP_CTX_ERROR(
            mgr->m_app, "%s: unregister type %d: type is unknown!", ui_data_mgr_name(mgr), type);
        return -1;
    }

    product_type = ui_product_type_of(mgr, type);
    if (product_type->product_free == NULL) {
        APP_CTX_ERROR(
            mgr->m_app, "%s: unregister type %d: type is already unregistered!", ui_data_mgr_name(mgr), type);
        return -1;
    }

    while(!TAILQ_EMPTY(&product_type->srcs)) {
        ui_data_src_free(TAILQ_FIRST(&product_type->srcs));
    }

    bzero(product_type, sizeof(*product_type));
    TAILQ_INIT(&product_type->srcs);

    return 0;
}

EXPORT_DIRECTIVE
int ui_data_mgr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_data_mgr_t mgr;

    mgr =
        ui_data_mgr_create(
            app,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app),
            ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL)));
    if (mgr == NULL) return -1;

    mgr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (mgr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done", ui_data_mgr_name(mgr));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_data_mgr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_data_mgr_t mgr;

    mgr = ui_data_mgr_find_nc(app, gd_app_module_name(module));
    if (mgr) {
        ui_data_mgr_free(mgr);
    }
}
