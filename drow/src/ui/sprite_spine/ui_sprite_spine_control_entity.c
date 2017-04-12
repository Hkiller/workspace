#include <assert.h>
#include "spine/Skeleton.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_control_entity_i.h"

static void ui_sprite_spine_control_entity_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s);

ui_sprite_spine_control_entity_t ui_sprite_spine_control_entity_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_CONTROL_ENTITY_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_control_entity_free(ui_sprite_spine_control_entity_t control_entity) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(control_entity);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_control_entity_set_obj_name(ui_sprite_spine_control_entity_t control_entity, const char * obj_name) {
    assert(obj_name);

    if (control_entity->m_cfg_obj_name) {
        mem_free(control_entity->m_module->m_alloc, control_entity->m_cfg_obj_name);
        control_entity->m_cfg_obj_name = NULL;
    }

    control_entity->m_cfg_obj_name = cpe_str_mem_dup(control_entity->m_module->m_alloc, obj_name);
    
    return 0;
}

int ui_sprite_spine_control_entity_set_prefix(ui_sprite_spine_control_entity_t control_entity, const char * prefix) {
    assert(prefix);

    if (control_entity->m_cfg_prefix) {
        mem_free(control_entity->m_module->m_alloc, control_entity->m_cfg_prefix);
        control_entity->m_cfg_prefix = NULL;
    }

    control_entity->m_cfg_prefix = cpe_str_mem_dup(control_entity->m_module->m_alloc, prefix);
     
    return 0;
}

static int ui_sprite_spine_control_entity_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_control_entity_t control_entity = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_transform_t transform;
    ui_sprite_render_anim_t render_anim;
    ui_sprite_render_sch_t render_sch;
    ui_runtime_render_obj_ref_t render_obj_ref;
    ui_runtime_render_obj_t render_obj;
    plugin_spine_obj_t spine_obj;
    uint16_t prefix_len;
    struct spSkeleton* skeleton;
    uint16_t slot_pos;

    assert(TAILQ_EMPTY(&control_entity->m_slots));
    
    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind entity: obj no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    
    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind entity: entity no anim sch!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    render_anim = ui_sprite_render_anim_find_by_name(render_sch, control_entity->m_cfg_obj_name);
    if (render_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind entity: runing anim %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_entity->m_cfg_obj_name);
        goto ENTER_FAIL;
    }

    render_obj_ref = ui_sprite_render_anim_obj(render_anim);
    assert(render_obj_ref);
    
    render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);
    assert(render_obj);
    
    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_find_obj: render obj %s is not spine obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_entity->m_cfg_obj_name);
        goto ENTER_FAIL;
    }
    
    spine_obj = ui_runtime_render_obj_data(render_obj);
    if (spine_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind entity: spine-obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_entity->m_cfg_obj_name);
        goto ENTER_FAIL;
    }

	skeleton = plugin_spine_obj_skeleton(spine_obj);

    if (control_entity->m_cfg_prefix == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind entity: prefix not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    prefix_len = strlen(control_entity->m_cfg_prefix);

    for(slot_pos = 0; slot_pos < skeleton->slotsCount; ++slot_pos) {
        spSlot* slot = skeleton->slots[slot_pos];
        const char * name;
        char * args;
        const char * str_value;
        enum ui_sprite_spine_control_entity_slot_mode mode;
        ui_sprite_spine_control_entity_slot_t entity_slot;
        
        if (!cpe_str_start_with(slot->data->name, control_entity->m_cfg_prefix)) continue;

        name = ui_sprite_spine_module_analize_name(module, slot->data->name + prefix_len, &args);
        if (name == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): bind entity: analize part name %s error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->data->name + prefix_len);
            goto ENTER_FAIL;
        }

        if (args == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): bind entity: analize part name %s no args!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->data->name + prefix_len);
            goto ENTER_FAIL;
        }

        str_value = cpe_str_read_and_remove_arg(args, "mode", ',', '=');
        if (str_value == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): bind entity: analize part name %s mode not configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->data->name + prefix_len);
            goto ENTER_FAIL;
        }

        if (strcmp(str_value, "attach") == 0) {
            mode = ui_sprite_spine_control_entity_slot_mode_attach;
        }
        else if (strcmp(str_value, "bind") == 0) {
            mode = ui_sprite_spine_control_entity_slot_mode_bind;
        }
        else if (strcmp(str_value, "flush") == 0) {
            mode = ui_sprite_spine_control_entity_slot_mode_flush;
        }
        else {
            CPE_ERROR(
                module->m_em, "entity %d(%s): bind entity: analize part name %s mode %s unknown!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->data->name + prefix_len, str_value);
            goto ENTER_FAIL;
        }

        entity_slot = ui_sprite_spine_control_entity_slot_create(control_entity, slot, mode, name);
        if (entity_slot == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): bind entity: create slot fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_FAIL;
        }

        str_value = cpe_str_trim_head(args);
        if (str_value[0]) {
            assert(entity_slot->m_setups == NULL);
            entity_slot->m_setups = cpe_str_mem_dup(module->m_alloc, str_value);
        }
    }
        
    control_entity->m_render_anim = render_anim;

    ui_sprite_fsm_action_start_update(fsm_action);
    ui_sprite_spine_control_entity_update(fsm_action, ctx, 0.0f);
    
    return 0;

ENTER_FAIL:
    while(!TAILQ_EMPTY(&control_entity->m_slots)) {
        ui_sprite_spine_control_entity_slot_free(TAILQ_FIRST(&control_entity->m_slots));
    }

    return -1;
}

static void ui_sprite_spine_control_entity_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_control_entity_t control_entity = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_control_entity_slot_t slot;
    ui_transform anim_transform;
    
    ui_sprite_render_anim_calc_obj_world_transform(control_entity->m_render_anim, &anim_transform);

    TAILQ_FOREACH(slot, &control_entity->m_slots, m_next) {
        ui_sprite_spine_control_entity_slot_update(entity, slot, &anim_transform);
    }
}

static void ui_sprite_spine_control_entity_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_control_entity_t control_entity = ui_sprite_fsm_action_data(fsm_action);

    control_entity->m_render_anim = NULL;

    while(!TAILQ_EMPTY(&control_entity->m_slots)) {
        ui_sprite_spine_control_entity_slot_free(TAILQ_FIRST(&control_entity->m_slots));
    }
}

static int ui_sprite_spine_control_entity_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_control_entity_t control_entity = ui_sprite_fsm_action_data(fsm_action);
    control_entity->m_module = ctx;
    control_entity->m_cfg_obj_name = NULL;
    control_entity->m_cfg_prefix = NULL;
    control_entity->m_cfg_debug = 0;
    control_entity->m_render_anim = NULL;
    TAILQ_INIT(&control_entity->m_slots);
    return 0;
}

static void ui_sprite_spine_control_entity_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;
    ui_sprite_spine_control_entity_t control_entity = ui_sprite_fsm_action_data(fsm_action);

    assert(control_entity->m_render_anim == NULL);
    assert(TAILQ_EMPTY(&control_entity->m_slots));

    if (control_entity->m_cfg_obj_name) {
        mem_free(modue->m_alloc, control_entity->m_cfg_obj_name);
        control_entity->m_cfg_obj_name = NULL;
    }

    if (control_entity->m_cfg_prefix) {
        mem_free(modue->m_alloc, control_entity->m_cfg_prefix);
        control_entity->m_cfg_prefix = NULL;
    }
}

static int ui_sprite_spine_control_entity_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;    
	ui_sprite_spine_control_entity_t to_control_entity = ui_sprite_fsm_action_data(to);
	ui_sprite_spine_control_entity_t from_control_entity = ui_sprite_fsm_action_data(from);

	if (ui_sprite_spine_control_entity_init(to, ctx)) return -1;

    to_control_entity->m_cfg_debug = from_control_entity->m_cfg_debug;

    if (from_control_entity->m_cfg_obj_name) {
        to_control_entity->m_cfg_obj_name = cpe_str_mem_dup(modue->m_alloc, from_control_entity->m_cfg_obj_name);
    }

    if (from_control_entity->m_cfg_prefix) {
        to_control_entity->m_cfg_prefix = cpe_str_mem_dup(modue->m_alloc, from_control_entity->m_cfg_prefix);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_spine_control_entity_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_control_entity_t control_entity = ui_sprite_spine_control_entity_create(fsm_state, name);
    const char * str_value;
    
    if (control_entity == NULL) {
        CPE_ERROR(module->m_em, "%s: create control_entity action: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    control_entity->m_cfg_debug = cfg_get_uint8( cfg, "debug", control_entity->m_cfg_debug);

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_spine_control_entity_set_obj_name(control_entity, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create control_entity action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_control_entity_free(control_entity);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create control_entity action: anim-name not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_control_entity_free(control_entity);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_spine_control_entity_set_prefix(control_entity, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create control_entity action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_control_entity_free(control_entity);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create control_entity action: prefix not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_control_entity_free(control_entity);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(control_entity);
}

int ui_sprite_spine_control_entity_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_CONTROL_ENTITY_NAME, sizeof(struct ui_sprite_spine_control_entity));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_control_entity_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_control_entity_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_control_entity_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_control_entity_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_control_entity_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_control_entity_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_CONTROL_ENTITY_NAME, ui_sprite_spine_control_entity_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_control_entity_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_CONTROL_ENTITY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SPINE_CONTROL_ENTITY_NAME);
}

const char * UI_SPRITE_SPINE_CONTROL_ENTITY_NAME = "spine-control-entity";
