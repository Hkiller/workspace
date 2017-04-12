#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_def.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui_sprite_ui_action_play_anim_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_play_anim_t ui_sprite_ui_action_play_anim_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_PLAY_ANIM_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_play_anim_free(ui_sprite_ui_action_play_anim_t action_play_anim) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_play_anim);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_ui_action_play_anim_set_control(ui_sprite_ui_action_play_anim_t action_play_anim, const char * control) {
    assert(control);

    if (action_play_anim->m_cfg_control) {
        mem_free(action_play_anim->m_module->m_alloc, action_play_anim->m_cfg_control);
        action_play_anim->m_cfg_control = NULL;
    }

    action_play_anim->m_cfg_control = cpe_str_mem_dup_trim(action_play_anim->m_module->m_alloc, control);
    
    return 0;
}

int ui_sprite_ui_action_play_anim_set_back_res(ui_sprite_ui_action_play_anim_t action_play_anim, const char * back_res) {
    assert(back_res);

    if (action_play_anim->m_cfg_back_res) {
        mem_free(action_play_anim->m_module->m_alloc, action_play_anim->m_cfg_back_res);
        action_play_anim->m_cfg_back_res = NULL;
    }

    action_play_anim->m_cfg_back_res = cpe_str_mem_dup_trim(action_play_anim->m_module->m_alloc, back_res);
    
    return 0;
}

int ui_sprite_ui_action_play_anim_set_down_res(ui_sprite_ui_action_play_anim_t action_play_anim, const char * down_res) {
    assert(down_res);

    if (action_play_anim->m_cfg_down_res) {
        mem_free(action_play_anim->m_module->m_alloc, action_play_anim->m_cfg_down_res);
        action_play_anim->m_cfg_down_res = NULL;
    }

    action_play_anim->m_cfg_down_res = cpe_str_mem_dup_trim(action_play_anim->m_module->m_alloc, down_res);
    
    return 0;
}

int ui_sprite_ui_action_play_anim_set_disable_res(ui_sprite_ui_action_play_anim_t action_play_anim, const char * disable_res) {
    assert(disable_res);

    if (action_play_anim->m_cfg_disable_res) {
        mem_free(action_play_anim->m_module->m_alloc, action_play_anim->m_cfg_disable_res);
        action_play_anim->m_cfg_disable_res = NULL;
    }

    action_play_anim->m_cfg_disable_res = cpe_str_mem_dup_trim(action_play_anim->m_module->m_alloc, disable_res);
    
    return 0;
}

static void ui_sprite_ui_action_play_anim_on_event(void * ctx, ui_runtime_render_obj_t obj, const char * evt) {
    ui_sprite_fsm_action_t fsm_action = ctx;
    ui_sprite_ui_action_play_anim_t action_play_anim = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    struct ui_sprite_fsm_addition_source_ctx addition_ctx;
    dr_data_source_t data_source = NULL;
    plugin_ui_page_t page;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &addition_ctx);
    
    page = ui_sprite_ui_find_page_from_action(action_play_anim->m_module, fsm_action, action_play_anim->m_page_name);
    if (page == NULL) {
        CPE_ERROR(
            action_play_anim->m_module->m_em, "entity %d(%s): play anim: on event: page %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_play_anim->m_page_name);
    }
    else {
        plugin_ui_page_build_and_send_event(page, evt, data_source);
    }

    if(action_play_anim->m_module->m_env && action_play_anim->m_module->m_env->m_entity != entity) {
        ui_sprite_entity_check_build_and_send_event(entity, evt, data_source);
    }
}

static int ui_sprite_ui_action_play_anim_add_frame(
    ui_sprite_ui_module_t module, ui_sprite_fsm_action_t fsm_action, plugin_ui_control_t control, plugin_ui_aspect_t aspect,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, const char * input_res, mem_buffer_t buff)
{
    const char * res;
    plugin_ui_control_frame_t frame;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (input_res[0] == '@') {
        ui_sprite_render_sch_t render_sch;
        ui_sprite_render_def_t render_def;

        render_sch = ui_sprite_render_sch_find(entity);
        if (render_sch == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): play anim: get res from animation: animation component not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        render_def = ui_sprite_render_def_find(render_sch, input_res + 1);
        if (render_def == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): play anim: get res from animation: anim %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), input_res + 1);
            return -1;
        }

        res = ui_sprite_render_def_anim_res(render_def);
    }
    else {
        dr_data_source_t page_data = NULL;
        struct dr_data_source page_data_buf;
        plugin_ui_page_t page = control ? plugin_ui_control_page(control) : NULL;

        if (page && plugin_ui_page_data_meta(page)) {
            page_data_buf.m_next = NULL;
            page_data_buf.m_data.m_meta = plugin_ui_page_data_meta(page);
            page_data_buf.m_data.m_data = plugin_ui_page_data(page);
            page_data_buf.m_data.m_size = plugin_ui_page_data_size(page);
            page_data = &page_data_buf;
        }
        
        res = ui_sprite_fsm_action_check_calc_str(buff, input_res, fsm_action, page_data, module->m_em);
        if (res == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): play anim: calc res from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), input_res);
            return -1;
        }
    }

    frame = plugin_ui_control_frame_create_by_res(control, layer, usage, res);
    if (frame == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): play anim: create fail fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    plugin_ui_control_frame_set_auto_remove(frame, 0);
    
    if (plugin_ui_aspect_control_frame_add(aspect, frame, 1) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): play anim: add render to aspect fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        plugin_ui_control_frame_free(frame);
        return -1;
    }

    ui_runtime_render_obj_ref_set_evt_processor(
        plugin_ui_control_frame_render_obj_ref(frame),
        ui_sprite_ui_action_play_anim_on_event, fsm_action);
    
    return 0;
}
    
static int ui_sprite_ui_action_play_anim_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_play_anim_t action_play_anim = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    const char * control_name;
    plugin_ui_control_t control;
    struct mem_buffer buff;

    assert(action_play_anim->m_aspect == NULL);
    assert(action_play_anim->m_page_name == NULL);
    
    mem_buffer_init(&buff, module->m_alloc);

    /*找到目标控件 */
    if (action_play_anim->m_cfg_control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): play anim: control not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_ERROR;
    }
    
    control_name = ui_sprite_fsm_action_check_calc_str(&buff, action_play_anim->m_cfg_control, fsm_action, NULL, NULL);
    if (control_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): play anim: calc control path from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_play_anim->m_cfg_control);
        goto ENTER_ERROR;
    }
    
    control = ui_sprite_ui_find_control_from_action(module, fsm_action, control_name);
    if (control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): play anim: control %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_name);
        goto ENTER_ERROR;
    }

    /*创建aspect */
    action_play_anim->m_aspect = plugin_ui_aspect_create(module->m_env->m_env, NULL);
    if (action_play_anim->m_aspect == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): play anim: create aspect fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_ERROR;
    }

    if (action_play_anim->m_cfg_back_res) {
        if (ui_sprite_ui_action_play_anim_add_frame(
                module, fsm_action, control, action_play_anim->m_aspect,
                plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, action_play_anim->m_cfg_back_res, &buff)
            != 0)
        {
            goto ENTER_ERROR;
        }
    }

    if (action_play_anim->m_cfg_down_res) {
        if (ui_sprite_ui_action_play_anim_add_frame(
                module, fsm_action, control, action_play_anim->m_aspect,
                plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, action_play_anim->m_cfg_down_res, &buff)
            != 0)
        {
            goto ENTER_ERROR;
        }
    }

    action_play_anim->m_page_name = cpe_str_mem_dup(module->m_alloc, plugin_ui_page_name(plugin_ui_control_page(control)));
    if (action_play_anim->m_page_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): play anim: save control name alloc fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_ERROR;
    }
    
    ui_sprite_fsm_action_start_update(fsm_action);

    mem_buffer_clear(&buff);

    return 0;

ENTER_ERROR:
    mem_buffer_clear(&buff);

    if (action_play_anim->m_page_name) {
        mem_free(module->m_alloc, action_play_anim->m_page_name);
        action_play_anim->m_page_name = NULL;
    }
    
    if (action_play_anim->m_aspect) {
        plugin_ui_aspect_free(action_play_anim->m_aspect);
        action_play_anim->m_aspect = NULL;
    }

    return -1;
}

static void ui_sprite_ui_action_play_anim_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_ui_action_play_anim_t action_play_anim = ui_sprite_fsm_action_data(fsm_action);
    struct plugin_ui_control_frame_it frame_it;
    plugin_ui_control_frame_t frame;
    uint8_t have_runing = 0;
    
    assert(action_play_anim->m_aspect);

    plugin_ui_aspect_control_frames(&frame_it, action_play_anim->m_aspect);

    while((frame = plugin_ui_control_frame_it_next(&frame_it))) {
        ui_runtime_render_obj_t render_obj =
            ui_runtime_render_obj_ref_obj(plugin_ui_control_frame_render_obj_ref(frame));
        assert(render_obj);
        if (ui_runtime_render_obj_is_playing(render_obj)) {
            have_runing = 1;
            break;
        }
    }

    if (!have_runing) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_ui_action_play_anim_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_play_anim_t action_play_anim = ui_sprite_fsm_action_data(fsm_action);

    assert(action_play_anim->m_aspect);
    assert(action_play_anim->m_page_name);

    mem_free(action_play_anim->m_module->m_alloc, action_play_anim->m_page_name);
    action_play_anim->m_page_name = NULL;
    
    plugin_ui_aspect_free(action_play_anim->m_aspect);
    action_play_anim->m_aspect = NULL;
}

static int ui_sprite_ui_action_play_anim_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_play_anim_t action_play_anim = ui_sprite_fsm_action_data(fsm_action);
    action_play_anim->m_module = ctx;
    action_play_anim->m_cfg_control = NULL;
    action_play_anim->m_cfg_back_res = NULL;
    action_play_anim->m_cfg_down_res = NULL;
    action_play_anim->m_cfg_disable_res = NULL;
    action_play_anim->m_page_name = NULL;
    action_play_anim->m_aspect = NULL;
    return 0;
}

static void ui_sprite_ui_action_play_anim_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;
    ui_sprite_ui_action_play_anim_t action_play_anim = ui_sprite_fsm_action_data(fsm_action);

    assert(action_play_anim->m_aspect == NULL);
    
    if (action_play_anim->m_cfg_control) {
        mem_free(modue->m_alloc, action_play_anim->m_cfg_control);
        action_play_anim->m_cfg_control = NULL;
    }

    if (action_play_anim->m_cfg_back_res) {
        mem_free(modue->m_alloc, action_play_anim->m_cfg_back_res);
        action_play_anim->m_cfg_back_res = NULL;
    }
    
    if (action_play_anim->m_cfg_down_res) {
        mem_free(modue->m_alloc, action_play_anim->m_cfg_down_res);
        action_play_anim->m_cfg_down_res = NULL;
    }

    if (action_play_anim->m_cfg_disable_res) {
        mem_free(modue->m_alloc, action_play_anim->m_cfg_disable_res);
        action_play_anim->m_cfg_disable_res = NULL;
    }
}

static int ui_sprite_ui_action_play_anim_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;    
	ui_sprite_ui_action_play_anim_t to_action_play_anim = ui_sprite_fsm_action_data(to);
	ui_sprite_ui_action_play_anim_t from_action_play_anim = ui_sprite_fsm_action_data(from);

	if (ui_sprite_ui_action_play_anim_init(to, ctx)) return -1;

    if (from_action_play_anim->m_cfg_control) {
        to_action_play_anim->m_cfg_control = cpe_str_mem_dup(modue->m_alloc, from_action_play_anim->m_cfg_control);
    }
    
    if (from_action_play_anim->m_cfg_back_res) {
        to_action_play_anim->m_cfg_back_res = cpe_str_mem_dup(modue->m_alloc, from_action_play_anim->m_cfg_back_res);
    }
    
    if (from_action_play_anim->m_cfg_down_res) {
        to_action_play_anim->m_cfg_down_res = cpe_str_mem_dup(modue->m_alloc, from_action_play_anim->m_cfg_down_res);
    }
    
    if (from_action_play_anim->m_cfg_disable_res) {
        to_action_play_anim->m_cfg_disable_res = cpe_str_mem_dup(modue->m_alloc, from_action_play_anim->m_cfg_disable_res);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_ui_action_play_anim_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_play_anim_t action_play_anim = ui_sprite_ui_action_play_anim_create(fsm_state, name);
    const char * str_value;
    
    if (action_play_anim == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_play_anim action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "control", NULL))) {
        if (ui_sprite_ui_action_play_anim_set_control(action_play_anim, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_play_anim action: set control %s fail!",
                ui_sprite_ui_module_name(module), str_value);
            ui_sprite_ui_action_play_anim_free(action_play_anim);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create action_play_anim action: control not configured!",
            ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_play_anim_free(action_play_anim);
        return NULL;
    }

    str_value = cfg_get_string(cfg, "back_res", NULL);
    if (str_value == NULL) str_value = cfg_get_string(cfg, "back-res", NULL);
    if (str_value) {
        if (ui_sprite_ui_action_play_anim_set_back_res(action_play_anim, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_play_anim action: set back_res %s fail!",
                ui_sprite_ui_module_name(module), str_value);
            ui_sprite_ui_action_play_anim_free(action_play_anim);
            return NULL;
        }
    }

    str_value = cfg_get_string(cfg, "down_res", NULL);
    if (str_value == NULL) str_value = cfg_get_string(cfg, "down-res", NULL);
    if (str_value) {
        if (ui_sprite_ui_action_play_anim_set_down_res(action_play_anim, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_play_anim action: set down_res %s fail!",
                ui_sprite_ui_module_name(module), str_value);
            ui_sprite_ui_action_play_anim_free(action_play_anim);
            return NULL;
        }
    }

    str_value = cfg_get_string(cfg, "disable_res", NULL);
    if (str_value == NULL) str_value = cfg_get_string(cfg, "disable-res", NULL);
    if (str_value) {
        if (ui_sprite_ui_action_play_anim_set_disable_res(action_play_anim, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_play_anim action: set disable_res %s fail!",
                ui_sprite_ui_module_name(module), str_value);
            ui_sprite_ui_action_play_anim_free(action_play_anim);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(action_play_anim);
}

int ui_sprite_ui_action_play_anim_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_PLAY_ANIM_NAME, sizeof(struct ui_sprite_ui_action_play_anim));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_play_anim_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_play_anim_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_play_anim_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_play_anim_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_play_anim_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ui_action_play_anim_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_UI_ACTION_PLAY_ANIM_NAME, ui_sprite_ui_action_play_anim_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_ui_action_play_anim_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_PLAY_ANIM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_PLAY_ANIM_NAME);
}

const char * UI_SPRITE_UI_ACTION_PLAY_ANIM_NAME = "ui-play-anim";
