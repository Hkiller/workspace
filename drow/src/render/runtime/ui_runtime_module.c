#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "ui_runtime_module_i.h"
#include "ui_runtime_render_obj_i.h"
#include "ui_runtime_render_obj_child_i.h"
#include "ui_runtime_render_program_i.h"
#include "ui_runtime_render_program_state_i.h"
#include "ui_runtime_render_program_state_attr_i.h"
#include "ui_runtime_render_program_state_unif_i.h"
#include "ui_runtime_render_queue_i.h"
#include "ui_runtime_render_cmd_i.h"
#include "ui_runtime_render_state_i.h"
#include "ui_runtime_render_material_i.h"
#include "ui_runtime_render_technique_i.h"
#include "ui_runtime_render_pass_i.h"
#include "ui_runtime_render_obj_meta_i.h"
#include "ui_runtime_render_i.h"
#include "ui_runtime_render_backend_i.h"
#include "ui_runtime_sound_playing_i.h"
#include "ui_runtime_sound_backend_i.h"

static void ui_runtime_module_do_clear(nm_node_t node);
static ptr_int_t ui_runtime_module_do_tick(void * ctx, ptr_int_t arg, float delta_s);

struct nm_node_type s_nm_node_type_ui_runtime_module = {
    "ui_runtime_module",
    ui_runtime_module_do_clear
};

static struct {
    const char * name; 
    int (*init)(ui_runtime_module_t module);
    void (*fini)(ui_runtime_module_t module);
} s_auto_reg_products[] = {
    { "sound-updator", ui_runtime_module_init_sound_updator, ui_runtime_module_fini_sound_updator }
    , { "sound-res-plugin", ui_runtime_module_init_sound_res_plugin, ui_runtime_module_fini_sound_res_plugin }    
};

ui_runtime_module_t
ui_runtime_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr,
    const char * name, error_monitor_t em)
{
    struct ui_runtime_module * module;
    nm_node_t module_node;
    uint8_t component_pos;

    assert(app);

    if (name == NULL) name = "ui_runtime_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_runtime_module));
    if (module_node == NULL) return NULL;

    module = (ui_runtime_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;

    module->m_data_mgr = data_mgr;
    module->m_cache_mgr = cache_mgr;

    module->m_runing_level = ui_runtime_runing;
    
    module->m_obj_count = 0;
    module->m_render_backend = NULL;
    module->m_render = NULL;

    /*声音 */
    module->m_sound_chanel_count = 0;
    module->m_sound_playing_max_id = 0;
    module->m_sound_playing_count = 0;
    module->m_sound_logic_pause = 1;
    module->m_sound_volume = 1.0f;

    TAILQ_INIT(&module->m_objs);
    TAILQ_INIT(&module->m_update_objs);
    TAILQ_INIT(&module->m_free_states);
    TAILQ_INIT(&module->m_free_cmds);
    TAILQ_INIT(&module->m_free_queues);
    TAILQ_INIT(&module->m_free_materials);
    TAILQ_INIT(&module->m_free_techniques);
    TAILQ_INIT(&module->m_free_passes);

    TAILQ_INIT(&module->m_sound_backends);
    TAILQ_INIT(&module->m_sound_groups);
    TAILQ_INIT(&module->m_sound_playings);
    TAILQ_INIT(&module->m_free_playings);
    TAILQ_INIT(&module->m_free_program_states);
    TAILQ_INIT(&module->m_free_program_state_attrs);
    TAILQ_INIT(&module->m_free_program_state_unifs);

    TAILQ_INIT(&module->m_free_obj_childs);
    
    bzero(module->m_obj_metas_by_id, sizeof(module->m_obj_metas_by_id));

    if (gd_app_tick_add(module->m_app, ui_runtime_module_do_tick, module, 0) != 0) {
        CPE_ERROR(module->m_em, "%s: add tick fail!", name);
        nm_node_free(module_node);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &module->m_named_objs,
            alloc,
            (cpe_hash_fun_t) ui_runtime_render_obj_name_hash,
            (cpe_hash_eq_t) ui_runtime_render_obj_name_eq,
            CPE_HASH_OBJ2ENTRY(ui_runtime_render_obj, m_hh_for_module),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init named objs fail", name);
        gd_app_tick_remove(module->m_app, ui_runtime_module_do_tick, module);
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_obj_childs,
            alloc,
            (cpe_hash_fun_t) ui_runtime_render_obj_child_hash,
            (cpe_hash_eq_t) ui_runtime_render_obj_child_eq,
            CPE_HASH_OBJ2ENTRY(ui_runtime_render_obj_child, m_hh_for_module),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init child objs fail", name);
        cpe_hash_table_fini(&module->m_named_objs);
        gd_app_tick_remove(module->m_app, ui_runtime_module_do_tick, module);
        nm_node_free(module_node);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &module->m_obj_metas,
            alloc,
            (cpe_hash_fun_t) ui_runtime_render_obj_meta_hash,
            (cpe_hash_eq_t) ui_runtime_render_obj_meta_eq,
            CPE_HASH_OBJ2ENTRY(ui_runtime_render_obj_meta, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: init obj metas fail", name);
        cpe_hash_table_fini(&module->m_obj_childs);
        cpe_hash_table_fini(&module->m_named_objs);
        gd_app_tick_remove(module->m_app, ui_runtime_module_do_tick, module);
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
            cpe_hash_table_fini(&module->m_obj_metas);
            cpe_hash_table_fini(&module->m_obj_childs);
            cpe_hash_table_fini(&module->m_named_objs);
            gd_app_tick_remove(module->m_app, ui_runtime_module_do_tick, module);
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_ui_runtime_module);

    return module;
}

static void ui_runtime_module_do_clear(nm_node_t node) {
    ui_runtime_module_t module;
    uint8_t i;
    uint8_t component_pos;

    module = nm_node_data(node);

    gd_app_tick_remove(module->m_app, ui_runtime_module_do_tick, module);
    
    while(!TAILQ_EMPTY(&module->m_sound_groups)) {
        ui_runtime_sound_group_free(TAILQ_FIRST(&module->m_sound_groups));
    }
    assert(module->m_sound_chanel_count == 0);
    assert(module->m_sound_playing_count == 0);

    if (module->m_render_backend) {
        ui_runtime_render_backend_free(module->m_render_backend);
        assert(module->m_render_backend == NULL);
    }
    
    while(!TAILQ_EMPTY(&module->m_sound_backends)) {
        ui_runtime_sound_backend_free(TAILQ_FIRST(&module->m_sound_backends));
    }

    while(!TAILQ_EMPTY(&module->m_objs)) {
        ui_runtime_render_obj_free(TAILQ_FIRST(&module->m_objs));
    }
    assert(cpe_hash_table_count(&module->m_named_objs) == 0);
    assert(cpe_hash_table_count(&module->m_obj_childs) == 0);
    assert(module->m_obj_count == 0);
    cpe_hash_table_fini(&module->m_named_objs);
    cpe_hash_table_fini(&module->m_obj_childs);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    ui_runtime_render_obj_meta_free_all(module);
    cpe_hash_table_fini(&module->m_obj_metas);
    for(i = 0; i < CPE_ARRAY_SIZE(module->m_obj_metas_by_id); ++i) {
        assert(module->m_obj_metas_by_id[i] ==  NULL);
    }

    if (module->m_render) {
        ui_runtime_render_free(module->m_render);
    }

    while(!TAILQ_EMPTY(&module->m_free_obj_childs)) {
        ui_runtime_render_obj_child_real_free(TAILQ_FIRST(&module->m_free_obj_childs));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_program_states)) {
        ui_runtime_render_program_state_real_free(TAILQ_FIRST(&module->m_free_program_states));
    }

    while(!TAILQ_EMPTY(&module->m_free_program_state_attrs)) {
        ui_runtime_render_program_state_attr_real_free(TAILQ_FIRST(&module->m_free_program_state_attrs));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_program_state_unifs)) {
        ui_runtime_render_program_state_unif_real_free(TAILQ_FIRST(&module->m_free_program_state_unifs));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_states)) {
        ui_runtime_render_state_real_free(TAILQ_FIRST(&module->m_free_states));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_cmds)) {
        ui_runtime_render_cmd_real_free(TAILQ_FIRST(&module->m_free_cmds));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_queues)) {
        ui_runtime_render_queue_real_free(TAILQ_FIRST(&module->m_free_queues));
    }

    while(!TAILQ_EMPTY(&module->m_free_materials)) {
        ui_runtime_render_material_real_free(TAILQ_FIRST(&module->m_free_materials));
    }

    while(!TAILQ_EMPTY(&module->m_free_techniques)) {
        ui_runtime_render_technique_real_free(TAILQ_FIRST(&module->m_free_techniques));
    }

    while(!TAILQ_EMPTY(&module->m_free_passes)) {
        ui_runtime_render_pass_real_free(TAILQ_FIRST(&module->m_free_passes));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_playings)) {
        ui_runtime_sound_playing_real_free(TAILQ_FIRST(&module->m_free_playings));
    }
    
    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t ui_runtime_module_app(ui_runtime_module_t module) {
    return module->m_app;
}

void ui_runtime_module_free(ui_runtime_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_runtime_module) return;
    nm_node_free(module_node);
}

ui_runtime_module_t
ui_runtime_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_runtime_module) return NULL;
    return (ui_runtime_module_t)nm_node_data(node);
}

ui_runtime_module_t
ui_runtime_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "ui_runtime_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_runtime_module) return NULL;
    return (ui_runtime_module_t)nm_node_data(node);
}

ui_data_src_t ui_runtime_module_find_src(ui_runtime_module_t module, UI_OBJECT_SRC_REF const * src_ref, uint8_t src_type) {
    ui_data_src_t src;
    
    if (src_ref->type == UI_OBJECT_SRC_REF_TYPE_BY_ID) {
        src = ui_data_src_find_by_id(
            ui_runtime_module_data_mgr(module), src_ref->data.by_id.src_id);
        if (src == NULL) {
            CPE_ERROR(module->m_em, "%s: src %d not exist!", ui_runtime_module_name(module), src_ref->data.by_id.src_id);
            return NULL;
        }

        if (ui_data_src_type(src) != src_type) {
            CPE_ERROR(
                module->m_em, "%s: runtime obj init: src %d is not type %d!",
                ui_runtime_module_name(module), src_ref->data.by_id.src_id, src_type);
            return NULL;
        }
    }
    else if (src_ref->type == UI_OBJECT_SRC_REF_TYPE_BY_PATH) {
        src = ui_data_src_find_by_path(
            ui_runtime_module_data_mgr(module), (const char *)src_ref->data.by_path.path, src_type);
        if (src == NULL) {
            CPE_ERROR(
                module->m_em, "%s: src %s not exist!",
                ui_runtime_module_name(module), (const char *)src_ref->data.by_path.path);
            return NULL;
        }
    }
    else {
        assert(0);
        CPE_ERROR(module->m_em, "%s: unknown path type %d!", ui_runtime_module_name(module), src_ref->type);
        return NULL;
    }

    return src;
}

const char * ui_runtime_module_name(ui_runtime_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

ui_cache_manager_t ui_runtime_module_cache_mgr(ui_runtime_module_t module) {
    return module->m_cache_mgr;
}

ui_data_mgr_t ui_runtime_module_data_mgr(ui_runtime_module_t module) {
    return module->m_data_mgr;
}

uint32_t ui_runtime_module_obj_count(ui_runtime_module_t module) {
    return module->m_obj_count;
}

static ptr_int_t ui_runtime_module_do_tick(void * ctx, ptr_int_t arg, float delta) {
    ui_runtime_module_t module = ctx;
    ui_runtime_render_obj_t render_obj;

    TAILQ_FOREACH(render_obj, &module->m_update_objs, m_next_for_update) {
        assert(render_obj->m_meta->m_update_fun);
        assert(render_obj->m_updator == NULL);
        assert(render_obj->m_suspend == 0);
        assert(render_obj->m_in_module_update);
       
        render_obj->m_meta->m_update_fun(render_obj->m_meta->m_ctx, render_obj, delta * render_obj->m_time_scale);
    }

    return 0;
}

ui_runtime_runing_level_t ui_runtime_module_runing_level(ui_runtime_module_t module) {
    return module->m_runing_level;
}

int ui_runtime_module_set_runing_level(ui_runtime_module_t module, ui_runtime_runing_level_t runing_level) {
    if (module->m_runing_level == runing_level) return 0;

    module->m_runing_level = runing_level;
    ui_runtime_module_sound_sync_suspend(module);
    return 0;
}
