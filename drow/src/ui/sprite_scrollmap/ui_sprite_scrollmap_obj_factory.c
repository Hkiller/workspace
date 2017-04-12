#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/random.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/scrollmap/plugin_scrollmap_layer.h"
#include "plugin/scrollmap/plugin_scrollmap_obj.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_scrollmap_obj_factory_i.h"

static int ui_sprite_scrollmap_obj_set_anim(ui_sprite_scrollmap_env_t env, ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj);
static int ui_sprite_scrollmap_obj_set_pos(ui_sprite_scrollmap_env_t env, ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj);
static int ui_sprite_scrollmap_obj_set_range(ui_sprite_scrollmap_env_t env, ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj);
static int ui_sprite_scrollmap_obj_bind(ui_sprite_scrollmap_env_t env, ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj);
static void ui_sprite_scrollmap_obj_unbind(ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj);

const char * ui_sprite_scrollmap_obj_name(void * ctx, plugin_scrollmap_obj_t obj) {
    struct ui_sprite_scrollmap_obj_stub * stub = (struct ui_sprite_scrollmap_obj_stub *)plugin_scrollmap_obj_data(obj);

    if (stub->m_sprite_obj) {
        ui_sprite_entity_t entity;
        
        assert(stub->m_sprite_obj->m_obj == obj);

        entity = ui_sprite_component_entity(ui_sprite_component_from_data(stub->m_sprite_obj));
        return ui_sprite_entity_name(entity);
    }
    else {
        return NULL;
    }
}

static int ui_sprite_scrollmap_obj_setup_entity(ui_sprite_scrollmap_module_t module, ui_sprite_entity_t entity, char * def) {
    char * name_end;
    char * name_sep;
    
    name_end = strchr(def, '=');
    if (name_end == NULL) {
        CPE_ERROR(
            module->m_em, "sprite_scrollmap_env: create entity %d(%s): setup: def %s format error",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def);
        return -1;
    }

    name_end = cpe_str_trim_tail(name_end, def);
    *name_end = 0;

    if ((name_sep = strchr(def, '.'))) {
        ui_sprite_render_sch_t render_sch;
        ui_sprite_render_anim_t render_anim;
        ui_runtime_render_obj_ref_t render_obj_ref;
        ui_runtime_render_obj_t render_obj;

        render_sch = ui_sprite_render_sch_find(entity);
        if (render_sch == NULL) {
            CPE_ERROR(
                module->m_em, "ui_sprite_scrollmap_env: create entity %d(%s): setup: no render_sch",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        *name_sep = 0;
        
        render_anim = ui_sprite_render_anim_find_by_name(render_sch, def);
        if (render_anim == NULL) {
            CPE_ERROR(
                module->m_em, "ui_sprite_scrollmap_env: create entity %d(%s): setup: anim %s not exist",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def);
            return -1;
        }

        render_obj_ref = ui_sprite_render_anim_obj(render_anim);
        assert(render_obj_ref);
        
        render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);

        *name_end = '=';
        if (ui_runtime_render_obj_setup(render_obj, name_sep + 1) != 0) {
            CPE_ERROR(
                module->m_em, "ui_sprite_scrollmap_env: create entity %d(%s): setup: render obj setup %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name_sep + 1);
            return -1;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "ui_sprite_scrollmap_env: create entity %d(%s): setup: unknown attr %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def);
        return -1;
    }
    
    return 0;
}

int ui_sprite_scrollmap_obj_on_init(void * ctx, plugin_scrollmap_obj_t obj, const char * obj_type, const char * args) {
    ui_sprite_scrollmap_env_t env = ctx;
    ui_sprite_world_t world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(env));
    char entity_name[64];
    ui_sprite_entity_t entity;

    snprintf(entity_name, sizeof(entity_name), "%05d-%s", env->m_entity_index++, obj_type);
    entity = ui_sprite_entity_create(world, entity_name, obj_type);
    if (entity == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_scrollmap_env: create obj %s: create fail", obj_type);
        return -1;
    }
    
    if (ui_sprite_scrollmap_obj_set_anim(env, entity, obj) != 0
        || ui_sprite_scrollmap_obj_set_range(env, entity, obj) != 0
        || ui_sprite_scrollmap_obj_set_pos(env, entity, obj) != 0
        || ui_sprite_scrollmap_obj_bind(env, entity, obj) != 0
        )
    {
        ui_sprite_entity_free(entity);
        return -1;
    }

    if (ui_sprite_entity_enter(entity) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_scrollmap_env: new entity %d(%s) enter fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_scrollmap_obj_unbind(entity, obj);        
        ui_sprite_entity_free(entity);
        return -1;
    }

    if (args) {
        char * v = cpe_str_mem_dup(env->m_module->m_alloc, args);
        char * s;
        
        while((s = strchr(v, ','))) {
            *s = 0;
            if (ui_sprite_scrollmap_obj_setup_entity(env->m_module, entity, v) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "ui_sprite_scrollmap_env: new entity %d(%s): setup from %s fail",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), v);
                mem_free(env->m_module->m_alloc, v);
                ui_sprite_entity_free(entity);
                return -1;
            }
            
            v = cpe_str_trim_head(s + 1);
        }

        if (v[0]) {
            if (ui_sprite_scrollmap_obj_setup_entity(env->m_module, entity, v) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "ui_sprite_scrollmap_env: new entity %d(%s): setup from %s fail",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), v);
                mem_free(env->m_module->m_alloc, v);
                ui_sprite_entity_free(entity);
                return -1;
            }
        }

        mem_free(env->m_module->m_alloc, v);
    }
    
    return 0;
}

void ui_sprite_scrollmap_obj_on_update(void * ctx, plugin_scrollmap_obj_t obj) {
    struct ui_sprite_scrollmap_obj_stub * stub = (struct ui_sprite_scrollmap_obj_stub *)plugin_scrollmap_obj_data(obj);

    if (stub->m_sprite_obj) {
        ui_sprite_entity_t entity;
        
        assert(stub->m_sprite_obj->m_obj == obj);

        entity = ui_sprite_component_entity(ui_sprite_component_from_data(stub->m_sprite_obj));

        ui_sprite_scrollmap_obj_set_pos(ctx, entity, obj);
    }
}
    
void ui_sprite_scrollmap_obj_on_event(void * ctx, plugin_scrollmap_obj_t obj, const char * event) {
    struct ui_sprite_scrollmap_obj_stub * stub = (struct ui_sprite_scrollmap_obj_stub *)plugin_scrollmap_obj_data(obj);

    if (stub->m_sprite_obj) {
        ui_sprite_entity_t entity;
        
        assert(stub->m_sprite_obj->m_obj == obj);

        entity = ui_sprite_component_entity(ui_sprite_component_from_data(stub->m_sprite_obj));
        
        ui_sprite_entity_build_and_send_event(entity, event, NULL);
    }
}

void ui_sprite_scrollmap_obj_on_destory(void * ctx, plugin_scrollmap_obj_t obj) {
    struct ui_sprite_scrollmap_obj_stub * stub = (struct ui_sprite_scrollmap_obj_stub *)plugin_scrollmap_obj_data(obj);

    if (stub->m_sprite_obj) {
        ui_sprite_entity_t entity;
        
        assert(stub->m_sprite_obj->m_obj == obj);

        entity = ui_sprite_component_entity(ui_sprite_component_from_data(stub->m_sprite_obj));
        stub->m_sprite_obj->m_obj = NULL;
        
        ui_sprite_entity_free(entity);
    }
}

static int ui_sprite_scrollmap_obj_set_anim(
    ui_sprite_scrollmap_env_t env, ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj)
{
    ui_sprite_render_sch_t render_sch;

    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_scrollmap_env: new entity %d(%s) no render_sch",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_render_sch_set_default_layer_by_name(render_sch, plugin_scrollmap_layer_name(plugin_scrollmap_obj_layer(obj)));
    ui_sprite_render_sch_set_render_priority(render_sch, (float)env->m_entity_index);

    return 0;
}

static int ui_sprite_scrollmap_obj_set_range(ui_sprite_scrollmap_env_t env, ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj) {
    ui_sprite_2d_transform_t transform;
    ui_rect rect;
    
    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) return 0;

    rect = ui_sprite_2d_transform_rect(transform);

    plugin_scrollmap_obj_set_range(
        obj,
        cpe_math_distance(
            0.0f, 0.0f,
            cpe_max(fabs(rect.lt.x), fabs(rect.rb.x)),
            cpe_max(fabs(rect.lt.y), fabs(rect.rb.y))));

    return 0;
}

static int ui_sprite_scrollmap_obj_set_pos(ui_sprite_scrollmap_env_t env, ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj) {
    ui_vector_2 world_pos;
    ui_sprite_2d_transform_t transform;
    ui_sprite_render_env_t render_env;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_scrollmap_env: new entity %d(%s) no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_transform_get_pos_2(plugin_scrollmap_obj_transform(obj), &world_pos);
 
    if ((render_env = ui_sprite_render_env_find(ui_sprite_entity_world(entity)))) {
        world_pos = ui_sprite_render_env_logic_to_world(render_env, &world_pos);
    }

    ui_sprite_2d_transform_set_origin_pos(transform, world_pos);

    return 0;
}

static int ui_sprite_scrollmap_obj_bind(ui_sprite_scrollmap_env_t env, ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj) {
    struct ui_sprite_scrollmap_obj_stub * stub = (struct ui_sprite_scrollmap_obj_stub *)plugin_scrollmap_obj_data(obj);
    ui_sprite_scrollmap_obj_t sprite_obj;

    sprite_obj = ui_sprite_scrollmap_obj_find(entity);
    if (sprite_obj == NULL) {
        sprite_obj = ui_sprite_scrollmap_obj_create(entity);
        if (sprite_obj == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_scrollmap_env: new entity %d(%s) create scrollmap obj fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }
    
    sprite_obj->m_obj = obj;
    plugin_scrollmap_obj_set_move_suspend(obj, sprite_obj->m_is_suspend);
    
    stub->m_sprite_obj = sprite_obj;

    return 0;
}

static void ui_sprite_scrollmap_obj_unbind(ui_sprite_entity_t entity, plugin_scrollmap_obj_t obj) {
    struct ui_sprite_scrollmap_obj_stub * stub = (struct ui_sprite_scrollmap_obj_stub *)plugin_scrollmap_obj_data(obj);
    ui_sprite_scrollmap_obj_t sprite_obj;

    sprite_obj = ui_sprite_scrollmap_obj_find(entity);
    if (sprite_obj) {
        sprite_obj->m_obj = NULL;
    }
    
    stub->m_sprite_obj = NULL;
}
