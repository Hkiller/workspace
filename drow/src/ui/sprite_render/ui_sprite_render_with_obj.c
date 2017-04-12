#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_object_ref.h"
#include "render/model/ui_data_src.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_render_with_obj_i.h"

ui_sprite_render_with_obj_t ui_sprite_render_with_obj_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_RENDER_WITH_OBJ_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_with_obj_free(ui_sprite_render_with_obj_t action_with_obj) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_with_obj);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_render_with_obj_set_obj_name(ui_sprite_render_with_obj_t action_with_obj, const char * obj_name) {
    assert(obj_name);

    if (action_with_obj->m_cfg_obj_name) {
        mem_free(action_with_obj->m_module->m_alloc, action_with_obj->m_cfg_obj_name);
        action_with_obj->m_cfg_obj_name = NULL;
    }

    action_with_obj->m_cfg_obj_name = cpe_str_mem_dup_trim(action_with_obj->m_module->m_alloc, obj_name);
    
    return 0;
}

int ui_sprite_render_with_obj_set_obj_res(ui_sprite_render_with_obj_t action_with_obj, const char * obj_res) {
    assert(obj_res);

    if (action_with_obj->m_cfg_obj_res) {
        mem_free(action_with_obj->m_module->m_alloc, action_with_obj->m_cfg_obj_res);
        action_with_obj->m_cfg_obj_res = NULL;
    }

    action_with_obj->m_cfg_obj_res = cpe_str_mem_dup_trim(action_with_obj->m_module->m_alloc, obj_res);
    
    return 0;
}

static int ui_sprite_render_with_obj_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_with_obj_t action_with_obj = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    struct mem_buffer buffer;
    const char * res;
    
    assert(action_with_obj->m_render_obj == NULL);

    if (action_with_obj->m_cfg_obj_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): with obj: no obj name configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (action_with_obj->m_cfg_obj_res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): with obj: no obj res configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    mem_buffer_init(&buffer, NULL);
    res = ui_sprite_fsm_action_check_calc_str(&buffer, action_with_obj->m_cfg_obj_res, fsm_action, NULL, module->m_em);
    if (res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): with obj: calc obj res %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_with_obj->m_cfg_obj_res);
        mem_buffer_clear(&buffer);
        return -1;
    }
    
    action_with_obj->m_render_obj = ui_runtime_render_obj_create_by_res(module->m_runtime, res, action_with_obj->m_cfg_obj_name);
    if (action_with_obj->m_render_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): with obj: create obj from res %s!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
        mem_buffer_clear(&buffer);
        return -1;
    }
    mem_buffer_clear(&buffer);
    
    ui_runtime_render_obj_set_auto_release(action_with_obj->m_render_obj, 0);

    return 0;
}

static void ui_sprite_render_with_obj_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_with_obj_t action_with_obj = ui_sprite_fsm_action_data(fsm_action);

    assert(action_with_obj->m_render_obj);

    assert(ui_runtime_render_obj_ref_count(action_with_obj->m_render_obj) == 0);
    
    ui_runtime_render_obj_free(action_with_obj->m_render_obj);

    action_with_obj->m_render_obj = NULL;
}

static int ui_sprite_render_with_obj_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_with_obj_t action_with_obj = ui_sprite_fsm_action_data(fsm_action);
    action_with_obj->m_module = ctx;
    action_with_obj->m_cfg_obj_name = NULL;
    action_with_obj->m_cfg_obj_res = NULL;
    action_with_obj->m_render_obj = NULL;
    return 0;
}

static void ui_sprite_render_with_obj_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t modue = ctx;
    ui_sprite_render_with_obj_t action_with_obj = ui_sprite_fsm_action_data(fsm_action);

    assert(action_with_obj->m_render_obj == NULL);
    
    if (action_with_obj->m_cfg_obj_name) {
        mem_free(modue->m_alloc, action_with_obj->m_cfg_obj_name);
        action_with_obj->m_cfg_obj_name = NULL;
    }

    if (action_with_obj->m_cfg_obj_res) {
        mem_free(modue->m_alloc, action_with_obj->m_cfg_obj_res);
        action_with_obj->m_cfg_obj_res = NULL;
    }
}

static int ui_sprite_render_with_obj_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t modue = ctx;    
	ui_sprite_render_with_obj_t to_action_with_obj = ui_sprite_fsm_action_data(to);
	ui_sprite_render_with_obj_t from_action_with_obj = ui_sprite_fsm_action_data(from);

	if (ui_sprite_render_with_obj_init(to, ctx)) return -1;

    if (from_action_with_obj->m_cfg_obj_name) {
        to_action_with_obj->m_cfg_obj_name = cpe_str_mem_dup(modue->m_alloc, from_action_with_obj->m_cfg_obj_name);
    }
    
    if (from_action_with_obj->m_cfg_obj_res) {
        to_action_with_obj->m_cfg_obj_res = cpe_str_mem_dup(modue->m_alloc, from_action_with_obj->m_cfg_obj_res);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_render_with_obj_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_with_obj_t action_with_obj = ui_sprite_render_with_obj_create(fsm_state, name);
    const char * str_value;
    
    if (action_with_obj == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_with_obj action: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "obj-name", NULL))) {
        if (ui_sprite_render_with_obj_set_obj_name(action_with_obj, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_with_obj action: set obj name %s fail!",
                ui_sprite_render_module_name(module), str_value);
            ui_sprite_render_with_obj_free(action_with_obj);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create action_with_obj action: obj-name not configured!", ui_sprite_render_module_name(module));
        ui_sprite_render_with_obj_free(action_with_obj);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "obj-res", NULL))) {
        if (ui_sprite_render_with_obj_set_obj_res(action_with_obj, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_with_obj action: set obj res %s fail!",
                ui_sprite_render_module_name(module), str_value);
            ui_sprite_render_with_obj_free(action_with_obj);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create action_with_obj action: obj-res not configured!", ui_sprite_render_module_name(module));
        ui_sprite_render_with_obj_free(action_with_obj);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(action_with_obj);
}

int ui_sprite_render_with_obj_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_RENDER_WITH_OBJ_NAME, sizeof(struct ui_sprite_render_with_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_render_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_with_obj_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_with_obj_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_with_obj_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_with_obj_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_with_obj_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_RENDER_WITH_OBJ_NAME, ui_sprite_render_with_obj_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_render_with_obj_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_RENDER_WITH_OBJ_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_render_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_RENDER_WITH_OBJ_NAME);
}

const char * UI_SPRITE_RENDER_WITH_OBJ_NAME = "render-with-obj";
