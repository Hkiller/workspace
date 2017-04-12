#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pal/pal_strings.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_render_change_second_color_i.h"
#include "ui_sprite_render_utils_i.h"

ui_sprite_render_change_second_color_t ui_sprite_render_change_second_color_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_RENDER_CHANGE_SECOND_COLOR_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_change_second_color_free(ui_sprite_render_change_second_color_t change_second_color) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(change_second_color);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_render_changle_second_color_set_anim_name(ui_sprite_render_change_second_color_t change_second_color, const char * anim_name) {
    assert(anim_name);

    if (change_second_color->m_anim_name) {
        mem_free(change_second_color->m_module->m_alloc, change_second_color->m_anim_name);
        change_second_color->m_anim_name = NULL;
    }

    change_second_color->m_anim_name = cpe_str_mem_dup(change_second_color->m_module->m_alloc, anim_name);
    
    return 0;
}

ui_runtime_render_second_color_t ui_sprite_render_change_second_color_second_color(ui_sprite_render_change_second_color_t change_second_color) {
    return &change_second_color->m_second_color;
}

void ui_sprite_render_change_second_color_set_second_color(ui_sprite_render_change_second_color_t change_second_color, ui_runtime_render_second_color_t second_color) {
    change_second_color->m_second_color = *second_color;
}

ui_color_t ui_sprite_render_change_second_color_change_to_color(ui_sprite_render_change_second_color_t change_second_color) {
    return &change_second_color->m_change_to_color;
}

float ui_sprite_render_change_second_color_change_take_time(ui_sprite_render_change_second_color_t change_second_color) {
    return change_second_color->m_change_take_time;
}

void ui_sprite_render_change_second_color_set_change_to_color(ui_sprite_render_change_second_color_t change_second_color, ui_color_t des_color, float take_time) {
    change_second_color->m_change_to_color = *des_color;
    change_second_color->m_change_take_time = take_time;
}

int ui_sprite_render_change_second_color_set_decorator(ui_sprite_render_change_second_color_t change_second_color, const char * decorator) {
    return ui_percent_decorator_setup(&change_second_color->m_decorator, decorator, change_second_color->m_module->m_em);
}

static int ui_sprite_render_change_second_color_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_change_second_color_t change_second_color = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_runtime_render_obj_ref_t render_obj_ref;

    if (change_second_color->m_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render change second_color: anim name not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    render_obj_ref = ui_sprite_render_find_entity_render_obj(module, entity, change_second_color->m_anim_name);
    if (render_obj_ref == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render change second_color: find anim %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), change_second_color->m_anim_name);
        return -1;
    }

    change_second_color->m_saved = * ui_runtime_render_obj_ref_second_color(render_obj_ref);

    ui_runtime_render_obj_ref_set_second_color(render_obj_ref, &change_second_color->m_second_color);

    change_second_color->m_runing_time = 0.0f;
	change_second_color->m_runing_loop_count = 0;
    change_second_color->m_render_obj_ref = render_obj_ref;
    ui_sprite_fsm_action_sync_update(fsm_action, change_second_color->m_change_take_time > 0);
 
    return 0;
}

static void ui_sprite_render_change_second_color_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_change_second_color_t change_second_color = ui_sprite_fsm_action_data(fsm_action);

    if (change_second_color->m_render_obj_ref) {
        ui_runtime_render_obj_ref_set_second_color(change_second_color->m_render_obj_ref, &change_second_color->m_saved);
        change_second_color->m_render_obj_ref = NULL;
    }
}

void ui_sprite_render_change_second_color_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_render_change_second_color_t change_second_color = ui_sprite_fsm_action_data(fsm_action);
    struct ui_runtime_render_second_color target_second_color;
    float percent;

    assert(change_second_color->m_render_obj_ref);

    change_second_color->m_runing_time += delta;
   
    percent = change_second_color->m_runing_time >= change_second_color->m_change_take_time ? 1.0f : change_second_color->m_runing_time / change_second_color->m_change_take_time;
    percent = ui_percent_decorator_decorate(&change_second_color->m_decorator, percent);

    target_second_color = change_second_color->m_second_color;
    target_second_color.m_color.a += (change_second_color->m_change_to_color.a - target_second_color.m_color.a) * percent;
    target_second_color.m_color.r += (change_second_color->m_change_to_color.r - target_second_color.m_color.r) * percent;
    target_second_color.m_color.g += (change_second_color->m_change_to_color.g - target_second_color.m_color.g) * percent;
    target_second_color.m_color.b += (change_second_color->m_change_to_color.b - target_second_color.m_color.b) * percent;

    ui_runtime_render_obj_ref_set_second_color(change_second_color->m_render_obj_ref, &target_second_color);

    if(change_second_color->m_runing_time >= change_second_color->m_change_take_time) {
		change_second_color->m_runing_loop_count++;
		if(change_second_color->m_runing_loop_count >= change_second_color->m_loop_count) {
			ui_sprite_fsm_action_sync_update(fsm_action, 0);
		}
		else {
			change_second_color->m_runing_time = 0.0f;
			target_second_color.m_color = change_second_color->m_change_to_color;
		}
    }
}

static int ui_sprite_render_change_second_color_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_change_second_color_t change_second_color = ui_sprite_fsm_action_data(fsm_action);
	bzero(change_second_color, sizeof(*change_second_color));
    change_second_color->m_module = ctx;

    change_second_color->m_second_color.m_mix = ui_runtime_render_second_color_none;
    change_second_color->m_second_color.m_color.a = 1.0f;
    change_second_color->m_second_color.m_color.r = 0;
    change_second_color->m_second_color.m_color.g = 0;
    change_second_color->m_second_color.m_color.b = 0;

    return 0;
}

static void ui_sprite_render_change_second_color_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_change_second_color_t change_second_color = ui_sprite_fsm_action_data(fsm_action);
    assert(change_second_color->m_render_obj_ref == NULL);

    if (change_second_color->m_anim_name) {
        mem_free(module->m_alloc, change_second_color->m_anim_name);
        change_second_color->m_anim_name = NULL;
    }
}

static int ui_sprite_render_change_second_color_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_change_second_color_t to_change_second_color = ui_sprite_fsm_action_data(to);
    ui_sprite_render_change_second_color_t from_change_second_color = ui_sprite_fsm_action_data(from);

    if (ui_sprite_render_change_second_color_init(to, ctx)) return -1;

    to_change_second_color->m_change_to_color = from_change_second_color->m_change_to_color;
    to_change_second_color->m_change_take_time = from_change_second_color->m_change_take_time;
    to_change_second_color->m_second_color = from_change_second_color->m_second_color;

    if (from_change_second_color->m_anim_name) {
        to_change_second_color->m_anim_name = cpe_str_mem_dup(module->m_alloc, from_change_second_color->m_anim_name);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_render_change_second_color_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_change_second_color_t render_change_second_color = ui_sprite_render_change_second_color_create(fsm_state, name);
	cfg_t change_cfg;
    const char * str_value;
    
    if (render_change_second_color == NULL) {
        CPE_ERROR(module->m_em, "%s: create render_change_second_color action: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_render_changle_second_color_set_anim_name(render_change_second_color, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create render_change_second_color action: set anim-name fail!", ui_sprite_render_module_name(module));
            ui_sprite_render_change_second_color_free(render_change_second_color);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create render_change_second_color action: anim-name not configured!", ui_sprite_render_module_name(module));
        return NULL;
    }

	render_change_second_color->m_second_color.m_color.a = cfg_get_float(cfg, "second-color.color.a", render_change_second_color->m_second_color.m_color.a * 255.0f);
	render_change_second_color->m_second_color.m_color.r = cfg_get_float(cfg, "second-color.color.r", render_change_second_color->m_second_color.m_color.r * 255.0f);
	render_change_second_color->m_second_color.m_color.g = cfg_get_float(cfg, "second-color.color.g", render_change_second_color->m_second_color.m_color.g * 255.0f);
	render_change_second_color->m_second_color.m_color.b = cfg_get_float(cfg, "second-color.color.b", render_change_second_color->m_second_color.m_color.b * 255.0f);
	render_change_second_color->m_second_color.m_mix = ui_runtime_render_second_color_mix_from_str(cfg_get_string(cfg, "second-color.mix", ""), render_change_second_color->m_second_color.m_mix);
	render_change_second_color->m_second_color.m_color.a /= 255.0f;
	render_change_second_color->m_second_color.m_color.r /= 255.0f;
	render_change_second_color->m_second_color.m_color.g /= 255.0f;
	render_change_second_color->m_second_color.m_color.b /= 255.0f;

	if ((change_cfg = cfg_find_cfg(cfg, "change"))) {
		const char * str_decorator;
		render_change_second_color->m_loop_count = cfg_get_float(change_cfg, "loop-count", 1);		
		render_change_second_color->m_change_take_time = cfg_get_float(change_cfg, "take-time", 0.0f);
		render_change_second_color->m_change_to_color.a = cfg_get_float(change_cfg, "color.a", render_change_second_color->m_second_color.m_color.a * 255.0f);
		render_change_second_color->m_change_to_color.r = cfg_get_float(change_cfg, "color.r", render_change_second_color->m_second_color.m_color.r * 255.0f);
		render_change_second_color->m_change_to_color.g = cfg_get_float(change_cfg, "color.g", render_change_second_color->m_second_color.m_color.g * 255.0f);
		render_change_second_color->m_change_to_color.b = cfg_get_float(change_cfg, "color.b", render_change_second_color->m_second_color.m_color.b * 255.0f);
		render_change_second_color->m_change_to_color.a /= 255.0f;
		render_change_second_color->m_change_to_color.r /= 255.0f;
		render_change_second_color->m_change_to_color.g /= 255.0f;
		render_change_second_color->m_change_to_color.b /= 255.0f;
		if ((str_decorator = cfg_get_string(change_cfg, "decorate", NULL))) {
			ui_sprite_render_change_second_color_set_decorator(render_change_second_color, str_decorator);
		}
	}

    return ui_sprite_fsm_action_from_data(render_change_second_color);
}

int ui_sprite_render_change_second_color_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_RENDER_CHANGE_SECOND_COLOR_NAME, sizeof(struct ui_sprite_render_change_second_color));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_render_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_change_second_color_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_change_second_color_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_change_second_color_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_change_second_color_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_change_second_color_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_render_change_second_color_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_RENDER_CHANGE_SECOND_COLOR_NAME, ui_sprite_render_change_second_color_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_render_change_second_color_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_RENDER_CHANGE_SECOND_COLOR_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_render_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_RENDER_CHANGE_SECOND_COLOR_NAME = "render-change-second-color";

