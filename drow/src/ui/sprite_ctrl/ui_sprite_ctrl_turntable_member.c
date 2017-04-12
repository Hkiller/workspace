#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_ctrl_turntable_member_i.h"
#include "ui_sprite_ctrl_turntable_i.h"

ui_sprite_ctrl_turntable_member_t ui_sprite_ctrl_turntable_member_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
}

ui_sprite_ctrl_turntable_member_t ui_sprite_ctrl_turntable_member_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
}

void ui_sprite_ctrl_turntable_member_free(ui_sprite_ctrl_turntable_member_t member) {
    ui_sprite_component_t component = ui_sprite_component_from_data(member);
    ui_sprite_component_free(component);
}

int ui_sprite_ctrl_turntable_member_set_on_select(ui_sprite_ctrl_turntable_member_t member, const char * on_select) {
    ui_sprite_ctrl_module_t module = member->m_module;

    if (member->m_on_select) mem_free(module->m_alloc, member->m_on_select);

    if (on_select) {
        member->m_on_select = cpe_str_mem_dup(module->m_alloc, on_select);
        return member->m_on_select == NULL ? -1 : 0;
    }
    else {
        member->m_on_select = NULL;
        return 0;
    }
}

const char * ui_sprite_ctrl_turntable_member_on_select(ui_sprite_ctrl_turntable_member_t member) {
    return member->m_on_select;
}

int ui_sprite_ctrl_turntable_member_set_on_unselect(ui_sprite_ctrl_turntable_member_t member, const char * on_unselect) {
    ui_sprite_ctrl_module_t module = member->m_module;

    if (member->m_on_unselect) mem_free(module->m_alloc, member->m_on_unselect);

    if (on_unselect) {
        member->m_on_unselect = cpe_str_mem_dup(module->m_alloc, on_unselect);
        return member->m_on_unselect == NULL ? -1 : 0;
    }
    else {
        member->m_on_unselect = NULL;
        return 0;
    }
}

const char * ui_sprite_ctrl_turntable_member_on_unselect(ui_sprite_ctrl_turntable_member_t member) {
    return member->m_on_unselect;
}

static int ui_sprite_ctrl_turntable_member_enter(ui_sprite_component_t component, void * ctx) {
    return 0;
}

static void ui_sprite_ctrl_turntable_member_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_component_data(component);

    if (member->m_turntable) {
        ui_sprite_ctrl_turntable_remove_member(member->m_turntable, member);
        assert(member->m_turntable == NULL);
    }
}

static int ui_sprite_ctrl_turntable_member_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_component_data(component);

    bzero(member, sizeof(*member));

    member->m_module = ctx;

    return 0;
}

static void ui_sprite_ctrl_turntable_member_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_component_data(component);
    assert(member->m_turntable == NULL);
}

static int ui_sprite_ctrl_turntable_member_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_member_t from_member = ui_sprite_component_data(from);
    ui_sprite_ctrl_turntable_member_t to_member = ui_sprite_component_data(to);

    if (ui_sprite_ctrl_turntable_member_init(to, ctx)) return -1;

    if (from_member->m_on_select) {
        to_member->m_on_select = cpe_str_mem_dup(module->m_alloc, from_member->m_on_select);
    }

    if (from_member->m_on_unselect) {
        to_member->m_on_unselect = cpe_str_mem_dup(module->m_alloc, from_member->m_on_unselect);
    }

    return 0;
}

static int ui_sprite_ctrl_turntable_member_load(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_member_t turntable_member = ui_sprite_component_data(component);

    if (ui_sprite_ctrl_turntable_member_set_on_select(turntable_member, cfg_get_string(cfg, "on-select", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create turntable_member action: set on-enter fail",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_turntable_member_free(turntable_member);
        return -1;
    }

    if (ui_sprite_ctrl_turntable_member_set_on_unselect(turntable_member, cfg_get_string(cfg, "on-unselect", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create turntable_member action: set on-unselect fail",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_turntable_member_free(turntable_member);
        return -1;
    }

    return 0;
}

int ui_sprite_ctrl_turntable_member_regist(ui_sprite_ctrl_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(module->m_repo, UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME, sizeof(struct ui_sprite_ctrl_turntable_member));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: tuntable component register: meta create fail",
            ui_sprite_ctrl_module_name(module));
        return -1;
    }

    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_ctrl_turntable_member_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_ctrl_turntable_member_exit, module);
    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_ctrl_turntable_member_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_ctrl_turntable_member_copy, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_ctrl_turntable_member_fini, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_comp_loader(
                module->m_loader, UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME, ui_sprite_ctrl_turntable_member_load, module)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: %s register: register loader fail",
                ui_sprite_ctrl_module_name(module), UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME);
            ui_sprite_component_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_ctrl_turntable_member_unregist(ui_sprite_ctrl_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl unregister: meta not exist",
            ui_sprite_ctrl_module_name(module));
        return;
    }

    ui_sprite_component_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME);
    }
}

const char * UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME = "TurntableMember";

