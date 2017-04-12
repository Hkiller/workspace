#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_render_resume_i.h"
#include "ui_sprite_render_utils_i.h"

ui_sprite_render_resume_t ui_sprite_render_resume_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_RENDER_RESUME_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_resume_free(ui_sprite_render_resume_t resume) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(resume);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_render_resume_set_anim_name(ui_sprite_render_resume_t resume, const char * anim_name) {
    assert(anim_name);

    if (resume->m_cfg_anim_name) {
        mem_free(resume->m_module->m_alloc, resume->m_cfg_anim_name);
    }

    resume->m_cfg_anim_name = cpe_str_mem_dup(resume->m_module->m_alloc, anim_name);
    
    return 0;
}

static int ui_sprite_render_resume_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_resume_t resume = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_runtime_render_obj_t render_obj;

    if (resume->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_resume: render not configured!");
        return -1;
    }

    render_obj = ui_sprite_render_find_render_obj(module, entity, resume->m_cfg_anim_name);
    if (render_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_render_resume: render obj %s not exist!",
            resume->m_cfg_anim_name);
        return -1;
    }

    ui_runtime_render_obj_set_suspend(render_obj, 0);

    return 0;
}

static void ui_sprite_render_resume_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_resume_t resume = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_runtime_render_obj_t render_obj;

    if (resume->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_resume: render not configured!");
        return;
    }

    render_obj = ui_sprite_render_find_render_obj(module, entity, resume->m_cfg_anim_name);
    if (render_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_render_resume: render obj %s not exist!",
            resume->m_cfg_anim_name);
        return;
    }

    ui_runtime_render_obj_set_suspend(render_obj, 1);
}

static int ui_sprite_render_resume_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_resume_t resume = ui_sprite_fsm_action_data(fsm_action);
	bzero(resume, sizeof(*resume));
    resume->m_module = ctx;
    resume->m_cfg_anim_name = NULL;
    return 0;
}

static void ui_sprite_render_resume_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_resume_t resume = ui_sprite_fsm_action_data(fsm_action);

    if (resume->m_cfg_anim_name) {
        mem_free(module->m_alloc, resume->m_cfg_anim_name);
        resume->m_cfg_anim_name = NULL;
    }
}

static int ui_sprite_render_resume_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_resume_t to_resume = ui_sprite_fsm_action_data(to);
    ui_sprite_render_resume_t from_resume = ui_sprite_fsm_action_data(from);

    if (ui_sprite_render_resume_init(to, ctx)) return -1;

    if (from_resume->m_cfg_anim_name) {
        to_resume->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_resume->m_cfg_anim_name);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_render_resume_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_resume_t render_resume = ui_sprite_render_resume_create(fsm_state, name);
    const char * str_value;
    
    if (render_resume == NULL) {
        CPE_ERROR(module->m_em, "%s: create render resume action: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_render_resume_set_anim_name(render_resume, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render resume action: set anim name %s fail!",
                ui_sprite_render_module_name(module), str_value);
            ui_sprite_render_resume_free(render_resume);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create render resume: anim-name not configured!",
            ui_sprite_render_module_name(module));
        ui_sprite_render_resume_free(render_resume);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(render_resume);
}

int ui_sprite_render_resume_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_RENDER_RESUME_NAME, sizeof(struct ui_sprite_render_resume));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_render_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_resume_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_resume_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_resume_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_resume_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_resume_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_RENDER_RESUME_NAME, ui_sprite_render_resume_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_render_resume_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_RENDER_RESUME_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_render_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_RENDER_RESUME_NAME = "render-resume";

