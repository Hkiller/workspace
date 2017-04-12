#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/xcalc/xcalc_token.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/ui/plugin_ui_template_render.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_ui_action_show_template_i.h"

ui_sprite_ui_action_show_template_t
ui_sprite_ui_action_show_template_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_show_template_free(ui_sprite_ui_action_show_template_t show_ui) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_ui));
}

uint8_t ui_sprite_ui_action_show_template_is_free(ui_sprite_ui_action_show_template_t show_template) {
    return show_template->m_cfg_is_free;
}

void ui_sprite_ui_action_show_template_set_is_free(ui_sprite_ui_action_show_template_t show_template, uint8_t is_free) {
    show_template->m_cfg_is_free = is_free;
}

const char * ui_sprite_ui_action_show_template_group(ui_sprite_ui_action_show_template_t show_template) {
    return show_template->m_cfg_group;
}

void ui_sprite_ui_action_show_template_set_group(ui_sprite_ui_action_show_template_t show_template, const char * group) {
    if (show_template->m_cfg_group) {
        mem_free(show_template->m_module->m_alloc, show_template->m_cfg_group);
    }

    if (group) {
        show_template->m_cfg_group = cpe_str_mem_dup(show_template->m_module->m_alloc, group);
    }
    else {
        show_template->m_cfg_group = NULL;
    }
}

const char * ui_sprite_ui_action_show_template_res(ui_sprite_ui_action_show_template_t show_template) {
    return show_template->m_cfg_res;
}

void ui_sprite_ui_action_show_template_set_res(ui_sprite_ui_action_show_template_t show_template, const char * res) {
    if (show_template->m_cfg_res) {
        mem_free(show_template->m_module->m_alloc, show_template->m_cfg_res);
    }

    if (res) {
        show_template->m_cfg_res = cpe_str_mem_dup(show_template->m_module->m_alloc, res);
    }
    else {
        show_template->m_cfg_res = NULL;
    }
}

void ui_sprite_ui_action_show_template_exit(ui_sprite_fsm_action_t fsm_action, void * ctx);

struct ui_sprite_ui_action_show_collect_binding_ctx {
    ui_sprite_ui_action_show_template_t m_show_template;
    ui_sprite_ui_action_show_template_def_t m_def;
    ui_sprite_entity_t m_entity;
};

static void ui_sprite_ui_action_show_collect_binding(void * input_ctx, xtoken_t arg) {
    struct ui_sprite_ui_action_show_collect_binding_ctx * ctx = input_ctx;
    ui_sprite_ui_module_t module = ctx->m_show_template->m_module;
    ui_sprite_ui_action_show_template_binding_t show_binding;
    const char * arg_name = xtoken_try_to_str(arg);

    if (arg_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: collect binding, get arg name fail",
            ui_sprite_entity_id(ctx->m_entity), ui_sprite_entity_name(ctx->m_entity));
        return;;
    }

    if (arg_name[0] == '[') return;

    TAILQ_FOREACH(show_binding, &ctx->m_show_template->m_bindings, m_next) {
        if (show_binding->m_def == ctx->m_def && strcmp(show_binding->m_attr_name, arg_name) == 0) return;
    }

    if (ui_sprite_ui_action_show_template_binding_create(ctx->m_show_template, ctx->m_def, arg_name) == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: collect binding, crate binding %s fail",
            ui_sprite_entity_id(ctx->m_entity), ui_sprite_entity_name(ctx->m_entity), arg_name);
        return ;
    }
}

int ui_sprite_ui_action_show_template_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    xcomputer_t computer = ui_sprite_world_computer(ui_sprite_entity_world(entity));
    ui_sprite_render_sch_t render_sch = ui_sprite_render_sch_find(entity);
    ui_sprite_render_anim_t anim;
    ui_sprite_ui_action_show_template_def_t def;

    if (render_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: no render sch, can`t start anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res);
        return -1;
    }

    assert(show_template->m_anim_id == 0);

    anim = ui_sprite_render_sch_start_anim(render_sch, show_template->m_cfg_group, show_template->m_cfg_res, NULL);
    if (anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_action_show_template: start template %s: start anim fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res);
        return -1;
    }
    show_template->m_anim_id = ui_sprite_render_anim_id(anim);
    
    plugin_ui_template_render_set_is_free(
        ui_runtime_render_obj_data(ui_runtime_render_obj_ref_obj(ui_sprite_render_anim_obj(anim))),
        show_template->m_cfg_is_free);
        
    TAILQ_FOREACH(def, &show_template->m_cfg_defs, m_next) {
        struct ui_sprite_ui_action_show_collect_binding_ctx collect_ctx = { show_template, def, entity };

        if (ui_sprite_ui_action_show_template_def_set_value(def, show_template, anim, entity, fsm_action) != 0) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): ui_action_show_template: start template %s: set attr %s.%s fail",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res,
                    def->m_control, def->m_attr_name);
            }
        }

        if (xcomputer_visit_args(computer, def->m_attr_value, &collect_ctx, ui_sprite_ui_action_show_collect_binding) != 0) {
            ui_sprite_ui_action_show_template_exit(fsm_action, ctx);
            return -1;
        }
    }

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;
}

void ui_sprite_ui_action_show_template_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    while(!TAILQ_EMPTY(&show_template->m_bindings)) {
        ui_sprite_ui_action_show_template_binding_free(TAILQ_FIRST(&show_template->m_bindings));
    }

    if (show_template->m_anim_id) {
        ui_sprite_render_sch_t render_sch;
        
        if ((render_sch = ui_sprite_render_sch_find(entity))) {
            ui_sprite_render_anim_t anim = ui_sprite_render_anim_find_by_id(render_sch, show_template->m_anim_id);
            if (anim) ui_sprite_render_anim_free(anim);
        }
        show_template->m_anim_id = 0;
    }
}

void ui_sprite_ui_action_show_template_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_ui_action_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_render_sch_t render_sch = ui_sprite_render_sch_find(entity);
    ui_sprite_render_anim_t anim;

    if (render_sch == NULL) {
        CPE_ERROR(
            show_template->m_module->m_em, "entity %d(%s): ui_action_show_template: no render_sch, template %s stop update!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    anim = ui_sprite_render_anim_find_by_id(render_sch, show_template->m_anim_id);
    if (anim == NULL || !ui_sprite_render_anim_is_runing(anim)) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                show_template->m_module->m_em, "entity %d(%s): ui_action_show_template: template %s stoped",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_template->m_cfg_res);
        }
        
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}

int ui_sprite_ui_action_show_template_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);

    bzero(show_template, sizeof(*show_template));

    show_template->m_cfg_is_free = 0;
    show_template->m_module = ctx;
    show_template->m_anim_id = 0;
    
    TAILQ_INIT(&show_template->m_cfg_defs);
    TAILQ_INIT(&show_template->m_bindings);

    return 0;
}

int ui_sprite_ui_action_show_template_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_template_t to_show_template = ui_sprite_fsm_action_data(to);
    ui_sprite_ui_action_show_template_t from_show_template = ui_sprite_fsm_action_data(from);
    ui_sprite_ui_action_show_template_def_t from_def;
    
    ui_sprite_ui_action_show_template_init(to, ctx);

    to_show_template->m_cfg_is_free = from_show_template->m_cfg_is_free;
    
    if (from_show_template->m_cfg_res) {
        to_show_template->m_cfg_res = cpe_str_mem_dup(module->m_alloc, from_show_template->m_cfg_res);
    }

    if (from_show_template->m_cfg_group) {
        to_show_template->m_cfg_group = cpe_str_mem_dup(module->m_alloc, from_show_template->m_cfg_group);
    }

    TAILQ_FOREACH(from_def, &from_show_template->m_cfg_defs, m_next) {
        ui_sprite_ui_action_show_template_def_create(to_show_template, from_def->m_control, from_def->m_attr_name, from_def->m_attr_value);
    }
    
    return 0;
}

void ui_sprite_ui_action_show_template_fini(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_template_t show_template = ui_sprite_fsm_action_data(fsm_action);

    assert(TAILQ_EMPTY(&show_template->m_bindings));
    assert(show_template->m_anim_id == 0);

    while(!TAILQ_EMPTY(&show_template->m_cfg_defs)) {
        ui_sprite_ui_action_show_template_def_free(show_template, TAILQ_FIRST(&show_template->m_cfg_defs));
    }

    if (show_template->m_cfg_res) {
        mem_free(module->m_alloc, show_template->m_cfg_res);
        show_template->m_cfg_res = NULL;
    }

    if (show_template->m_cfg_group) {
        mem_free(module->m_alloc, show_template->m_cfg_group);
        show_template->m_cfg_group = NULL;
    }
}

static ui_sprite_fsm_action_t ui_sprite_ui_action_show_template_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_template_t show_template = ui_sprite_ui_action_show_template_create(fsm_state, name);
    const char * template_res;
    struct cfg_it binding_cfg_it;
    cfg_t binding_cfg;

    if (show_template == NULL) {
        CPE_ERROR(module->m_em, "%s: create show_template action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    if ((template_res = cfg_get_string(cfg, "res", NULL)) == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create show_template action: name not configured!",
            ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_show_template_free(show_template);
        return NULL;
    }
    ui_sprite_ui_action_show_template_set_res(show_template, template_res);
    ui_sprite_ui_action_show_template_set_group(show_template, cfg_get_string(cfg, "group", ""));
    ui_sprite_ui_action_show_template_set_is_free(show_template, cfg_get_uint8(cfg, "is-free", show_template->m_cfg_is_free));

    cfg_it_init(&binding_cfg_it, cfg_find_cfg(cfg, "bindings"));
    while((binding_cfg = cfg_it_next(&binding_cfg_it))) {
        struct cfg_it attr_cfg_it;
        cfg_t attr_cfg;
        const char * control_name;

        control_name = cfg_get_string(binding_cfg, "control", NULL);
        if (control_name == NULL) {
            CPE_ERROR(
                module->m_em, "%s: template %s: binding control name not set!",
                ui_sprite_ui_module_name(module), template_res);
            ui_sprite_ui_action_show_template_free(show_template);
            return NULL;
        }

        cfg_it_init(&attr_cfg_it, binding_cfg);
        while((attr_cfg = cfg_it_next(&attr_cfg_it))) {
            const char * attr_name = cfg_name(attr_cfg);
            const char * value;
            char value_buf[64];

            if (strcmp(attr_name, "control") == 0) continue;

            value = cfg_as_string(attr_cfg, NULL);
            if (value == NULL) {
                struct write_stream_mem s = CPE_WRITE_STREAM_MEM_INITIALIZER(value_buf, sizeof(value_buf));
                cfg_print_inline(attr_cfg, (write_stream_t)&s);
                stream_putc((write_stream_t)&s, 0);
                value = value_buf;
            }

            if (ui_sprite_ui_action_show_template_def_create(show_template, control_name, attr_name, value) == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: template %s: binding control %s (%s=%s) fail!",
                    ui_sprite_ui_module_name(module), template_res, control_name, attr_name, value);
                ui_sprite_ui_action_show_template_free(show_template);
                return NULL;
            }
        }
    }
    
    return ui_sprite_fsm_action_from_data(show_template);
}

int ui_sprite_ui_action_show_template_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME, sizeof(struct ui_sprite_ui_action_show_template));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_show_template_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_show_template_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_show_template_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_show_template_exit, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ui_action_show_template_update, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_show_template_fini, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(
                module->m_loader, UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME, ui_sprite_ui_action_show_template_load, module) != 0)
        {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ui_action_show_template_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME);
    }
}

const char * UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME = "show-template";
