#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/cache/ui_cache_manager.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_layout_module_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"
#include "plugin_layout_render_group_i.h"
#include "plugin_layout_render_group_node_i.h"
#include "plugin_layout_layout_i.h"
#include "plugin_layout_layout_meta_i.h"
#include "plugin_layout_layout_basic_i.h"
#include "plugin_layout_layout_rich_i.h"
#include "plugin_layout_layout_rich_block_i.h"
#include "plugin_layout_font_cache_i.h"
#include "plugin_layout_font_meta_i.h"
#include "plugin_layout_font_meta_font_i.h"
#include "plugin_layout_font_meta_pic_i.h"
#include "plugin_layout_font_face_i.h"
#include "plugin_layout_font_element_i.h"
#include "plugin_layout_animation_i.h"
#include "plugin_layout_animation_meta_i.h"
#include "plugin_layout_animation_caret_i.h"
#include "plugin_layout_animation_selection_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void plugin_layout_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_layout_module = {
    "plugin_layout_module",
    plugin_layout_module_clear
};

static struct {
    const char * name;
    int (*init)(plugin_layout_module_t module);
    void (*fini)(plugin_layout_module_t module);
} s_auto_reg_products[] = {
    { "font-cache", plugin_layout_font_cache_register, plugin_layout_font_cache_unregister }
    , { "font-meta-font", plugin_layout_font_meta_font_register, plugin_layout_font_meta_font_unregister }
    , { "font-meta-pic", plugin_layout_font_meta_pic_register, plugin_layout_font_meta_pic_unregister }
    , { "layout-render", plugin_layout_render_register, plugin_layout_render_unregister }
    , { "layout-basic", plugin_layout_layout_basic_register, plugin_layout_layout_basic_unregister }
    , { "layout-rich", plugin_layout_layout_rich_register, plugin_layout_layout_rich_unregister }
    , { "animation-caret", plugin_layout_animation_caret_regist, plugin_layout_animation_caret_unregist }
    , { "animation-selection", plugin_layout_animation_selection_regist, plugin_layout_animation_selection_unregist }
};

plugin_layout_module_t
plugin_layout_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    const char * name, error_monitor_t em)
{
    struct plugin_layout_module * module;
    nm_node_t module_node;
    int8_t component_pos;

    assert(app);

    if (name == NULL) name = "plugin_layout_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_layout_module));
    if (module_node == NULL) return NULL;

    module = (plugin_layout_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_data_mgr = data_mgr;
    module->m_cache_mgr = cache_mgr;
    module->m_runtime = runtime;
    module->m_layout_data_capacity = 0;
    module->m_font_cache = NULL;
    module->m_max_element_capacity = 0;
    module->m_element_count = 0;
    module->m_free_element_count = 0;
    module->m_node_count = 0;
    module->m_free_node_count = 0;
    module->m_group_count = 0;
    module->m_free_group_count = 0;
    module->m_group_node_count = 0;
    module->m_free_group_node_count = 0;
    module->m_animation_max_capacity = 0;
    module->m_max_animation_id = 0;
    
    TAILQ_INIT(&module->m_layouts);
    TAILQ_INIT(&module->m_font_metas);
    TAILQ_INIT(&module->m_free_layouts);
    TAILQ_INIT(&module->m_free_render_nodes);
    TAILQ_INIT(&module->m_free_render_groups);
    TAILQ_INIT(&module->m_free_render_group_nodes);
    TAILQ_INIT(&module->m_free_font_elements);
    TAILQ_INIT(&module->m_free_layout_rich_blocks);
    TAILQ_INIT(&module->m_free_animations);

    module->m_default_font_id.category = plugin_layout_font_category_font; /*see plugin_layout_font_category*/
	module->m_default_font_id.face = 0;
	module->m_default_font_id.size = 12;
	module->m_default_font_id.stroke_width = 0;

    module->m_animation_meta_caret = NULL;
    module->m_animation_meta_selection = NULL;
    
    if (cpe_hash_table_init(
            &module->m_layout_metas,
            alloc,
            (cpe_hash_fun_t) plugin_layout_layout_meta_hash,
            (cpe_hash_eq_t) plugin_layout_layout_meta_eq,
            CPE_HASH_OBJ2ENTRY(plugin_layout_layout_meta, m_hh_for_module),
            -1) != 0)
    {
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_font_faces,
            alloc,
            (cpe_hash_fun_t) plugin_layout_font_face_hash,
            (cpe_hash_eq_t) plugin_layout_font_face_eq,
            CPE_HASH_OBJ2ENTRY(plugin_layout_font_face, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&module->m_layout_metas);
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_animation_metas,
            alloc,
            (cpe_hash_fun_t) plugin_layout_animation_meta_hash,
            (cpe_hash_eq_t) plugin_layout_animation_meta_eq,
            CPE_HASH_OBJ2ENTRY(plugin_layout_animation_meta, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&module->m_font_faces);
        cpe_hash_table_fini(&module->m_layout_metas);
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_animations,
            alloc,
            (cpe_hash_fun_t) plugin_layout_animation_hash,
            (cpe_hash_eq_t) plugin_layout_animation_eq,
            CPE_HASH_OBJ2ENTRY(plugin_layout_animation, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&module->m_animation_metas);
        cpe_hash_table_fini(&module->m_font_faces);
        cpe_hash_table_fini(&module->m_layout_metas);
        nm_node_free(module_node);
        return NULL;
    }

    if (gd_app_tick_add(module->m_app, plugin_layout_module_tick, module, 0) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_module_create: add tick func fail");
        cpe_hash_table_fini(&module->m_animations);
        cpe_hash_table_fini(&module->m_animation_metas);
        cpe_hash_table_fini(&module->m_font_faces);
        cpe_hash_table_fini(&module->m_layout_metas);
        nm_node_free(module_node);
        return NULL;
    }
    
    mem_buffer_init(&module->m_dump_buffer, alloc);
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }
            mem_buffer_clear(&module->m_dump_buffer);
            gd_app_tick_remove(module->m_app, plugin_layout_module_tick, module);
            cpe_hash_table_fini(&module->m_animations);
            cpe_hash_table_fini(&module->m_animation_metas);
            cpe_hash_table_fini(&module->m_font_faces);
            cpe_hash_table_fini(&module->m_layout_metas);
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_plugin_layout_module);

    return module;
}

static void plugin_layout_module_clear(nm_node_t node) {
    plugin_layout_module_t module;
    int component_pos;

    module = (plugin_layout_module_t)nm_node_data(node);

    gd_app_tick_remove(module->m_app, plugin_layout_module_tick, module);
    
    /*附加信息 */
    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }
    assert(module->m_font_cache == NULL);
    assert(module->m_node_count == 0);
    assert(module->m_group_count == 0);
    assert(module->m_group_node_count == 0);
    
    /*layout*/
    plugin_layout_layout_meta_free_all(module);
    assert(cpe_hash_table_count(&module->m_layout_metas) == 0);
    assert(TAILQ_EMPTY(&module->m_layouts));
    cpe_hash_table_fini(&module->m_layout_metas);

    /*animation*/
    plugin_layout_animation_meta_free_all(module);
    assert(cpe_hash_table_count(&module->m_animation_metas) == 0);
    assert(cpe_hash_table_count(&module->m_animations) == 0);
    cpe_hash_table_fini(&module->m_animations);
    cpe_hash_table_fini(&module->m_animation_metas);
    
    /*font*/
    while(!TAILQ_EMPTY(&module->m_font_metas)) {
        plugin_layout_font_meta_free(TAILQ_FIRST(&module->m_font_metas));
    }
    assert(cpe_hash_table_count(&module->m_font_faces) == 0);
    cpe_hash_table_fini(&module->m_font_faces);
    assert(module->m_element_count == 0);

    /*free */
    while(!TAILQ_EMPTY(&module->m_free_layouts)) {
        plugin_layout_layout_real_free(TAILQ_FIRST(&module->m_free_layouts));
    }

    while(!TAILQ_EMPTY(&module->m_free_render_nodes)) {
        plugin_layout_render_node_real_free(TAILQ_FIRST(&module->m_free_render_nodes));
    }
    assert(module->m_free_node_count == 0);

    while(!TAILQ_EMPTY(&module->m_free_render_groups)) {
        plugin_layout_render_group_real_free(TAILQ_FIRST(&module->m_free_render_groups));
    }
    assert(module->m_free_group_count == 0);

    while(!TAILQ_EMPTY(&module->m_free_render_group_nodes)) {
        plugin_layout_render_group_node_real_free(TAILQ_FIRST(&module->m_free_render_group_nodes));
    }
    assert(module->m_free_group_node_count == 0);
    
    while(!TAILQ_EMPTY(&module->m_free_font_elements)) {
        plugin_layout_font_element_real_free(TAILQ_FIRST(&module->m_free_font_elements));
    }
    assert(module->m_free_element_count == 0);

    while(!TAILQ_EMPTY(&module->m_free_layout_rich_blocks)) {
        plugin_layout_layout_rich_block_real_free(TAILQ_FIRST(&module->m_free_layout_rich_blocks));
    }

    while(!TAILQ_EMPTY(&module->m_free_animations)) {
        plugin_layout_animation_real_free(TAILQ_FIRST(&module->m_free_animations));
    }
    
    mem_buffer_clear(&module->m_dump_buffer);
}

void plugin_layout_module_free(plugin_layout_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_layout_module) return;
    nm_node_free(module_node);
}

plugin_layout_module_t
plugin_layout_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_layout_module) return NULL;
    return (plugin_layout_module_t)nm_node_data(node);
}

plugin_layout_module_t
plugin_layout_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_layout_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_layout_module) return NULL;
    return (plugin_layout_module_t)nm_node_data(node);
}

const char * plugin_layout_module_name(plugin_layout_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

plugin_layout_font_id_t
plugin_layout_module_default_font_id(plugin_layout_module_t module) {
    return &module->m_default_font_id;
}
    
void plugin_layout_module_set_default_font_id(plugin_layout_module_t module, plugin_layout_font_id_t default_font_id) {
    module->m_default_font_id = *default_font_id;
}

uint32_t plugin_layout_module_node_count(plugin_layout_module_t module) {
    return module->m_node_count;
}

uint32_t plugin_layout_module_free_node_count(plugin_layout_module_t module) {
    return module->m_free_node_count;
}

EXPORT_DIRECTIVE
int plugin_layout_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_layout_module_t plugin_layout_module;
    
    plugin_layout_module =
        plugin_layout_module_create(
            app, gd_app_alloc(app),
            ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL)),
            ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL)),
            ui_runtime_module_find_nc(app, cfg_get_string(cfg, "runtime", NULL)),
            gd_app_module_name(module), gd_app_em(app));
    if (plugin_layout_module == NULL) return -1;

    if (plugin_layout_font_cache_set_size(
            plugin_layout_module->m_font_cache,
            cfg_get_uint32(cfg, "font-sys-cache-size.x", 1024u),
            cfg_get_uint32(cfg, "font-sys-cache-size.y", 1024u))
        != 0)
    {
        CPE_ERROR(gd_app_em(app), "%s: create: set size fail", plugin_layout_module_name(plugin_layout_module));
        plugin_layout_module_free(plugin_layout_module);
        return -1;
    }
    
    plugin_layout_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_layout_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_layout_module_name(plugin_layout_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_layout_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_layout_module_t plugin_layout_module;

    plugin_layout_module = plugin_layout_module_find_nc(app, gd_app_module_name(module));
    if (plugin_layout_module) {
        plugin_layout_module_free(plugin_layout_module);
    }
}

#ifdef __cplusplus
}
#endif
