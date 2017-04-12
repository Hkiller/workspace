#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/tailq_sort.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_render_env_i.h"
#include "ui_sprite_render_sch_i.h"
#include "ui_sprite_render_anim_i.h"
#include "ui_sprite_render_group_i.h"
#include "ui_sprite_render_def_i.h"

ui_sprite_render_sch_t ui_sprite_render_sch_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_RENDER_SCH_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_render_sch_t ui_sprite_render_sch_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_RENDER_SCH_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_render_sch_free(ui_sprite_render_sch_t render_sch) {
    ui_sprite_component_t component = ui_sprite_component_from_data(render_sch);
    if (component) {
        ui_sprite_component_free(component);
    }
}

ui_sprite_render_env_t ui_sprite_render_sch_env(ui_sprite_render_sch_t render_sch) {
    return render_sch->m_render_env;
}

float ui_sprite_render_sch_render_priority(ui_sprite_render_sch_t render_sch) {
    return render_sch->m_render_priority;
}

uint32_t ui_sprite_render_sch_entity_id(ui_sprite_render_sch_t render_sch) {
    return ui_sprite_entity_id(ui_sprite_component_entity(ui_sprite_component_from_data(render_sch)));
}

void ui_sprite_render_sch_set_render_priority(ui_sprite_render_sch_t render_sch, float render_priority) {
    if (render_sch->m_render_priority == render_priority) return;

    render_sch->m_render_priority = render_priority;

    if (ui_sprite_component_is_active(ui_sprite_component_from_data(render_sch))) {
        ui_sprite_render_anim_t anim;
        TAILQ_FOREACH(anim, &render_sch->m_anims, m_next_for_sch) {
            if (anim->m_group) {
                anim->m_priority = render_sch->m_render_priority + anim->m_group->m_adj_render_priority;
            }
            else {
                anim->m_priority = render_sch->m_render_priority;
            }
        }
    }
}

ui_sprite_render_anim_t
ui_sprite_render_sch_start_anim(
    ui_sprite_render_sch_t render_sch, const char * group_name, const char * res, const char * name)
{
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(render_sch));
    ui_sprite_render_module_t module = render_sch->m_module;
    ui_sprite_render_group_t group;
    ui_sprite_render_anim_t anim;

    assert(render_sch);
    assert(render_sch->m_default_layer);

    group = ui_sprite_render_group_find_by_name(render_sch, group_name ? group_name : "");
    if (group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): start anim  %s: group %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res, group_name ? group_name : "");
        return NULL;
    }
    
    if (res[0] == '@') {
        ui_sprite_render_def_t def = ui_sprite_render_def_find(render_sch, res + 1);
        if (def == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): start anim %s: anim def not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
            return NULL;
        }

        res = def->m_anim_res;
    }

    anim = ui_sprite_render_anim_create_by_res(render_sch->m_default_layer, res, group, name);
    if (anim == NULL) return 0;

    return anim;
}

static void ui_sprite_render_sch_on_transform_update(void * ctx) {
    ui_sprite_render_sch_t render_sch = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(render_sch));
    ui_sprite_render_group_t group;
    ui_sprite_2d_transform_t transform;
    ui_sprite_render_anim_t anim;
    uint8_t entity_trans_init = 0;
    ui_transform entity_trans;
 
    transform = ui_sprite_2d_transform_find(entity);
    assert(transform);
    
    TAILQ_FOREACH(group, &render_sch->m_groups, m_next_for_sch) {
        ui_sprite_render_group_update_world_trans(group, transform);
    }

    TAILQ_FOREACH(anim, &render_sch->m_anims, m_next_for_sch) {
        if (anim->m_group) {
            ui_sprite_render_anim_set_transform(anim, &anim->m_group->m_world_trans);
        }
        else {
            if (!entity_trans_init) {
                if (ui_sprite_2d_transform_calc_trans(transform, &entity_trans) != 0) return;
                entity_trans_init = 1;
            }
            ui_sprite_render_anim_set_transform(anim, &entity_trans);
        }
    }
}

static int ui_sprite_render_sch_do_init(ui_sprite_render_module_t module, ui_sprite_render_sch_t sch, ui_sprite_entity_t entity) {
    ui_sprite_world_t world = ui_sprite_entity_world(entity);

    ui_sprite_render_env_t env = ui_sprite_render_env_find(world);
    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render sch init: no backend!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    sch->m_module = module;
    sch->m_render_env = env;
    sch->m_default_layer = env->m_default_layer;
    sch->m_render_priority = 0;
    sch->m_is_dirty = 0;
    
    TAILQ_INIT(&sch->m_anims);
    TAILQ_INIT(&sch->m_defs);
    TAILQ_INIT(&sch->m_groups);

    return 0;
}

static int ui_sprite_render_sch_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_render_sch_t sch = ui_sprite_component_data(component);

    if (ui_sprite_render_sch_do_init(module, sch, entity) != 0) return -1;

    if (ui_sprite_render_group_create(sch, "") == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render sch init: create default group fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_render_sch_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_render_sch_t render_sch = ui_sprite_component_data(component);

    while(!TAILQ_EMPTY(&render_sch->m_anims)) {
        ui_sprite_render_anim_free(TAILQ_FIRST(&render_sch->m_anims));
    }
    
    while(!TAILQ_EMPTY(&render_sch->m_groups)) {
        ui_sprite_render_group_t group = TAILQ_FIRST(&render_sch->m_groups);
        ui_sprite_render_group_free(group);
    }

    while(!TAILQ_EMPTY(&render_sch->m_defs)) {
        ui_sprite_render_def_free(TAILQ_FIRST(&render_sch->m_defs));
    }
}

static int ui_sprite_render_sch_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(to);
    ui_sprite_render_sch_t to_render_sch = ui_sprite_component_data(to);
    ui_sprite_render_sch_t from_sch = ui_sprite_component_data(from);
    ui_sprite_render_def_t from_render_def;
    ui_sprite_render_group_t from_render_group;

    if (ui_sprite_render_sch_do_init(module, to_render_sch, entity) != 0) return -1;

    TAILQ_FOREACH(from_render_def, &from_sch->m_defs, m_next_for_sch) {
        if (ui_sprite_render_def_create(
                to_render_sch, from_render_def->m_anim_name, from_render_def->m_anim_res, from_render_def->m_auto_start)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): render sch copy: copy def %s ==> %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                from_render_def->m_anim_name, from_render_def->m_anim_res);
            ui_sprite_render_sch_fini(to, ctx);
            return -1;
        }
    }
    
    TAILQ_FOREACH(from_render_group, &from_sch->m_groups, m_next_for_sch) {
        if (ui_sprite_render_group_clone(to_render_sch, from_render_group) == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): render sch copy: copy group %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), from_render_group->m_name);
            ui_sprite_render_sch_fini(to, ctx);
            return -1;
        }
    }
    
    to_render_sch->m_default_layer = from_sch->m_default_layer;
    to_render_sch->m_render_priority = from_sch->m_render_priority;

    return 0;
}

static void ui_sprite_render_sch_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_render_sch_t render_sch = ui_sprite_component_data(component);
    ui_sprite_render_group_t group;

    while(!TAILQ_EMPTY(&render_sch->m_anims)) {
        ui_sprite_render_anim_free(TAILQ_FIRST(&render_sch->m_anims));
    }

    TAILQ_FOREACH(group, &render_sch->m_groups, m_next_for_sch) {
        ui_sprite_render_group_exit(group);
    }
}

static int ui_sprite_render_sch_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_sch_t render_sch = ui_sprite_component_data(component);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_render_def_t render_def;
    ui_sprite_render_group_t group;
    ui_sprite_2d_transform_t transform;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): sch enter: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    TAILQ_FOREACH(group, &render_sch->m_groups, m_next_for_sch) {
        if (ui_sprite_render_group_enter(group, transform) != 0) {
            ui_sprite_render_sch_exit(component, ctx);
            return -1;
        }
    }

    group = ui_sprite_render_group_find_by_name(render_sch, "");
    assert(group);

    TAILQ_FOREACH(render_def, &render_sch->m_defs, m_next_for_sch) {
        if (!render_def->m_auto_start) continue;

        if (ui_sprite_render_anim_create_by_res(render_sch->m_default_layer, render_def->m_anim_res, group, NULL) == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): auto start anim %s(%s): start fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                render_def->m_anim_name, render_def->m_anim_res);
            ui_sprite_render_sch_exit(component, ctx);
            return -1;
        }
    }

    if (ui_sprite_2d_transform_find(entity)) {
        if (ui_sprite_component_add_attr_monitor(
                component,
                "transform.pos,transform.scale,transform.angle,transform.flip_x,transform.flip_y",
                ui_sprite_render_sch_on_transform_update,
                render_sch)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): add attr monitor fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_render_sch_exit(component, ctx);
            return -1;
        }
    }

    return 0;
}

static void ui_sprite_render_sch_update(ui_sprite_component_t component, void * ctx, float delta) {
    /* ui_sprite_render_sch_t render_sch = ui_sprite_component_data(component); */
    /* ui_sprite_entity_t entity = ui_sprite_component_entity(component); */
    /* ui_sprite_render_anim_list_t anim; */

    /* TAILQ_FOREACH(anim, render_sch->m_anims, m_next_for_sch) { */
    /*     sch->m_backend = backend; */
    /* } */
}

int ui_sprite_render_sch_set_default_layer_by_name(ui_sprite_render_sch_t render_sch, const char * layer_name) {
    if (layer_name) {
        ui_sprite_render_layer_t layer = ui_sprite_render_layer_find(render_sch->m_render_env, layer_name);
        if (layer == NULL) {
            ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(render_sch));
            ui_sprite_render_module_t module = render_sch->m_module;
            CPE_ERROR(
                module->m_em, "entity %d(%s): set default layer: layer %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), layer_name);
            return -1;
        }
        render_sch->m_default_layer = layer;
    }
    else {
        render_sch->m_default_layer = render_sch->m_render_env->m_default_layer;
    }

    return 0;
}

void ui_sprite_render_sch_set_default_layer(ui_sprite_render_sch_t render_sch, ui_sprite_render_layer_t layer) {
    render_sch->m_default_layer = layer ? layer : render_sch->m_render_env->m_default_layer;
}

ui_sprite_render_layer_t ui_sprite_render_sch_default_layer(ui_sprite_render_sch_t render_sch) {
    return render_sch->m_default_layer;
}

int ui_sprite_render_sch_sort_anim_cmp(ui_sprite_render_anim_t l, ui_sprite_render_anim_t r, void * p) {
    if (l->m_priority == r->m_priority) return 0;
    return l->m_priority < r->m_priority ? -1 : 1;
}

void ui_sprite_render_sch_sort_anims(ui_sprite_render_sch_t sch) {
    TAILQ_SORT(
        &sch->m_anims, ui_sprite_render_anim, ui_sprite_render_anim_list, m_next_for_sch,
        ui_sprite_render_sch_sort_anim_cmp, NULL);
}

static int ui_sprite_render_sch_setup_one(ui_sprite_render_sch_t render_sch, char * def) {
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(render_sch));
    char * name_end;
    char * name_sep;
    
    name_end = strchr(def, '=');
    if (name_end == NULL) {
        CPE_ERROR(
            render_sch->m_module->m_em, "entity %d(%s): render sch setup: def %s format error",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def);
        return -1;
    }

    name_end = cpe_str_trim_tail(name_end, def);
    *name_end = 0;

    if ((name_sep = strchr(def, '.'))) {
        ui_sprite_render_anim_t render_anim;
        ui_runtime_render_obj_ref_t render_obj_ref;
        ui_runtime_render_obj_t render_obj;

        *name_sep = 0;
        
        render_anim = ui_sprite_render_anim_find_by_name(render_sch, def);
        if (render_anim == NULL) {
            CPE_ERROR(
                render_sch->m_module->m_em, "entity %d(%s): render sch setup: anim %s not exist",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def);
            *name_end = '=';
            return -1;
        }

        render_obj_ref = ui_sprite_render_anim_obj(render_anim);
        assert(render_obj_ref);
        
        render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);

        *name_end = '=';
        if (ui_runtime_render_obj_setup(render_obj, name_sep + 1) != 0) {
            CPE_ERROR(
                render_sch->m_module->m_em, "ui_sprite_scrollmap_env: create entity %d(%s): setup: render obj setup %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name_sep + 1);
            *name_end = '=';
            return -1;
        }
        return 0;
    }
    else {
        *name_end = '=';
        return 0;
    }
}

int ui_sprite_render_sch_setup(ui_sprite_render_sch_t render_sch, char * args) {
    int rv = 0;
    char * sep;
    char * end;

    while((sep = strchr(args, ','))) {
        char save;
        
        end = cpe_str_trim_tail(sep - 1, args);

        save = *end;
        *end = 0;
        if (ui_sprite_render_sch_setup_one(render_sch, args) != 0) rv = -1;
        *end = save;
        
        args = cpe_str_trim_head(sep + 1);
    }

    if (args[0]) {
        if (ui_sprite_render_sch_setup_one(render_sch, args) != 0) rv = -1;
    }
    
    return rv;
}

int ui_sprite_render_sch_regist(ui_sprite_render_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_RENDER_SCH_NAME, sizeof(struct ui_sprite_render_sch));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_render_module_name(module), UI_SPRITE_RENDER_SCH_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_render_sch_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_render_sch_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_render_sch_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_render_sch_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_render_sch_fini, module);
    ui_sprite_component_meta_set_update_fun(meta, ui_sprite_render_sch_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_comp_loader(
                module->m_loader, UI_SPRITE_RENDER_SCH_NAME, ui_sprite_render_sch_load, module)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: %s register: register loader fail",
                ui_sprite_render_module_name(module), UI_SPRITE_RENDER_SCH_NAME);
            ui_sprite_component_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_render_sch_unregist(ui_sprite_render_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_RENDER_SCH_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_render_module_name(module), UI_SPRITE_RENDER_SCH_NAME);
        return;
    }

    ui_sprite_component_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_RENDER_SCH_NAME);
    }
}

const char * UI_SPRITE_RENDER_SCH_NAME = "Animation";
