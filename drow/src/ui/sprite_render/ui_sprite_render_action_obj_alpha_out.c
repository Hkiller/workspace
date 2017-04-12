#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pal/pal_strings.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_render_action_obj_alpha_out_i.h"
#include "ui_sprite_render_utils_i.h"

static void ui_sprite_render_action_obj_alpha_out_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta);

ui_sprite_render_action_obj_alpha_out_t ui_sprite_render_action_obj_alpha_out_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_action_obj_alpha_out_free(ui_sprite_render_action_obj_alpha_out_t obj_alpha_out) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(obj_alpha_out);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_render_changle_action_obj_alpha_out_set_anim_name(ui_sprite_render_action_obj_alpha_out_t alpha_out, const char * anim_name) {
    assert(anim_name);

    if (alpha_out->m_cfg_anim_name) {
        mem_free(alpha_out->m_module->m_alloc, alpha_out->m_cfg_anim_name);
        alpha_out->m_cfg_anim_name = NULL;
    }

    alpha_out->m_cfg_anim_name = cpe_str_mem_dup(alpha_out->m_module->m_alloc, anim_name);
    
    return 0;
}

float ui_sprite_render_action_obj_alpha_out_take_time(ui_sprite_render_action_obj_alpha_out_t alpha_out) {
    return alpha_out->m_cfg_take_time;
}

void ui_sprite_render_action_obj_alpha_out_set_take_time(ui_sprite_render_action_obj_alpha_out_t alpha_out, float take_time) {
    alpha_out->m_cfg_take_time = take_time;
}

int ui_sprite_render_action_obj_alpha_out_set_decorator(ui_sprite_render_action_obj_alpha_out_t alpha_out, const char * decorator) {
    return ui_percent_decorator_setup(&alpha_out->m_cfg_decorator, decorator, alpha_out->m_module->m_em);
}

static int ui_sprite_render_action_obj_alpha_out_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_obj_alpha_out_t alpha_out = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_runtime_render_obj_ref_t render_obj_ref;
    struct ui_runtime_render_second_color second_color;

    if (alpha_out->m_cfg_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render action_obj_alpha_out: anim name not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    render_obj_ref = ui_sprite_render_find_entity_render_obj(module, entity, alpha_out->m_cfg_anim_name);
    if (render_obj_ref == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render action_obj_alpha_out: find anim %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), alpha_out->m_cfg_anim_name);
        return -1;
    }

    second_color = *ui_runtime_render_obj_ref_second_color(render_obj_ref);

    if (second_color.m_mix == ui_runtime_render_second_color_none) {
        second_color.m_mix = ui_runtime_render_second_color_add;
        ui_runtime_render_obj_ref_set_second_color(render_obj_ref, &second_color);
    }
    else {
        alpha_out->m_saved_mix = second_color.m_mix;
    }
    alpha_out->m_saved_alpha = second_color.m_color.a;

    alpha_out->m_runing_time = 0.0f;
    alpha_out->m_render_obj_ref = render_obj_ref;
    ui_sprite_render_action_obj_alpha_out_update(fsm_action, ctx, 0.0f);
    
    return 0;
}

static void ui_sprite_render_action_obj_alpha_out_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_action_obj_alpha_out_t alpha_out = ui_sprite_fsm_action_data(fsm_action);

    if (alpha_out->m_render_obj_ref) {
        struct ui_runtime_render_second_color second_color;
        second_color = *ui_runtime_render_obj_ref_second_color(alpha_out->m_render_obj_ref);
        second_color.m_color.a = alpha_out->m_saved_alpha;
        second_color.m_mix = alpha_out->m_saved_mix;
        ui_runtime_render_obj_ref_set_second_color(alpha_out->m_render_obj_ref, &second_color);
        ui_runtime_render_obj_ref_set_hide(alpha_out->m_render_obj_ref, 1);

        alpha_out->m_render_obj_ref = NULL;
    }
}

static void ui_sprite_render_action_obj_alpha_out_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_render_action_obj_alpha_out_t alpha_out = ui_sprite_fsm_action_data(fsm_action);
    float percent;
    struct ui_runtime_render_second_color second_color;

    assert(alpha_out->m_render_obj_ref);

    alpha_out->m_runing_time += delta;
   
    percent = alpha_out->m_runing_time >= alpha_out->m_cfg_take_time ? 1.0f : alpha_out->m_runing_time / alpha_out->m_cfg_take_time;
    percent = ui_percent_decorator_decorate(&alpha_out->m_cfg_decorator, percent);

    second_color = *ui_runtime_render_obj_ref_second_color(alpha_out->m_render_obj_ref);
    second_color.m_color.a = alpha_out->m_saved_alpha * (1.0f - percent);
    //printf("xxxxx: percent=%f, alpha=%f, mix=%d\n", percent, second_color.m_color.a, second_color.m_mix);
    ui_runtime_render_obj_ref_set_second_color(alpha_out->m_render_obj_ref, &second_color);

    ui_sprite_fsm_action_sync_update(fsm_action, alpha_out->m_runing_time < alpha_out->m_cfg_take_time);
}

static int ui_sprite_render_action_obj_alpha_out_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_action_obj_alpha_out_t alpha_out = ui_sprite_fsm_action_data(fsm_action);

    alpha_out->m_module = ctx;
    alpha_out->m_cfg_anim_name = NULL;
    alpha_out->m_cfg_take_time = 0.0f;
    bzero(&alpha_out->m_cfg_decorator, sizeof(alpha_out->m_cfg_decorator));

    alpha_out->m_runing_time = 0.0f;
    alpha_out->m_saved_mix = ui_runtime_render_second_color_none;
    alpha_out->m_saved_alpha = 0.0f;
    alpha_out->m_render_obj_ref = NULL;
    
    return 0;
}

static void ui_sprite_render_action_obj_alpha_out_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_obj_alpha_out_t alpha_out = ui_sprite_fsm_action_data(fsm_action);
    assert(alpha_out->m_render_obj_ref == NULL);

    if (alpha_out->m_cfg_anim_name) {
        mem_free(module->m_alloc, alpha_out->m_cfg_anim_name);
        alpha_out->m_cfg_anim_name = NULL;
    }
}

static int ui_sprite_render_action_obj_alpha_out_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_obj_alpha_out_t to_alpha_out = ui_sprite_fsm_action_data(to);
    ui_sprite_render_action_obj_alpha_out_t from_alpha_out = ui_sprite_fsm_action_data(from);

    if (ui_sprite_render_action_obj_alpha_out_init(to, ctx)) return -1;

    to_alpha_out->m_cfg_take_time = from_alpha_out->m_cfg_take_time;
    to_alpha_out->m_cfg_decorator = from_alpha_out->m_cfg_decorator;

    if (from_alpha_out->m_cfg_anim_name) {
        to_alpha_out->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_alpha_out->m_cfg_anim_name);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_render_action_obj_alpha_out_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_obj_alpha_out_t alpha_out = ui_sprite_render_action_obj_alpha_out_create(fsm_state, name);
    const char * str_value;
    
    if (alpha_out == NULL) {
        CPE_ERROR(module->m_em, "%s: create render_action_obj_alpha_out action: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_render_changle_action_obj_alpha_out_set_anim_name(alpha_out, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create render_action_obj_alpha_out action: set anim-name fail!", ui_sprite_render_module_name(module));
            ui_sprite_render_action_obj_alpha_out_free(alpha_out);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create render_action_obj_alpha_out action: anim-name not configured!", ui_sprite_render_module_name(module));
        return NULL;
    }

    alpha_out->m_cfg_take_time = cfg_get_float(cfg, "take-time", 0.0f);
    if ((str_value = cfg_get_string(cfg, "decorate", NULL))) {
        ui_sprite_render_action_obj_alpha_out_set_decorator(alpha_out, str_value);
    }

    return ui_sprite_fsm_action_from_data(alpha_out);
}

int ui_sprite_render_action_obj_alpha_out_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT, sizeof(struct ui_sprite_render_action_obj_alpha_out));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_render_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_action_obj_alpha_out_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_action_obj_alpha_out_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_action_obj_alpha_out_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_action_obj_alpha_out_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_action_obj_alpha_out_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_render_action_obj_alpha_out_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT, ui_sprite_render_action_obj_alpha_out_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_render_action_obj_alpha_out_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_render_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT = "render-obj-alpha-out";

