#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_object_ref.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_with_collision_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_load_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_collision_t ui_sprite_chipmunk_with_collision_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_COLLISION_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_collision_free(ui_sprite_chipmunk_with_collision_t with_collision) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_collision);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_with_collision_enter_load_from_src(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_with_collision_t with_collision,
    ui_sprite_entity_t entity, ui_sprite_2d_transform_t transform,
    cpSpace * space, ui_sprite_chipmunk_obj_t chipmunk_obj,
    ui_sprite_chipmunk_with_collision_src_t from_src, UI_OBJECT_URL * url, const char * path)
{
    ui_data_src_t chipmunk_src;
    plugin_chipmunk_data_scene_t scene;
    plugin_chipmunk_data_body_t data_body;

    chipmunk_src = ui_data_src_find_by_path(
        plugin_chipmunk_module_data_mgr(module->m_chipmunk_module), path, ui_data_src_type_chipmunk_scene);
    if (chipmunk_src == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with collision: find chipmunk scene %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), path);
        return -1;
    }

    scene = (plugin_chipmunk_data_scene_t)ui_data_src_product(chipmunk_src);
    if (scene == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with collision: chipmunk scene %s not loaded!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), path);
        return -1;
    }
    
    if (url->data.chipmunk.body[0]) {
        data_body = plugin_chipmunk_data_body_find_by_name(scene, url->data.chipmunk.body);
        if (data_body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with collision: chipmunk scene %s body %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), path, url->data.chipmunk.body);
            return -1;
        }

        if (ui_sprite_chipmunk_with_collision_body_create(
                entity, chipmunk_obj, with_collision, transform, space, data_body, from_src)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with collision: create with collision body fail!!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }
    else {
        struct plugin_chipmunk_data_body_it data_body_it;
        plugin_chipmunk_data_body_t data_body;

        plugin_chipmunk_data_scene_bodies(&data_body_it, scene);
        while((data_body = plugin_chipmunk_data_body_it_next(&data_body_it))) {
            if (ui_sprite_chipmunk_with_collision_body_create(
                    entity, chipmunk_obj, with_collision, transform, space, data_body, from_src)
                == NULL)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk with collision: create with collision body fail!!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }
        }
    }

    return 0;
}

static int ui_sprite_chipmunk_with_collision_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_collision_t with_collision = (ui_sprite_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform;
    cpSpace * space;
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_with_collision_src_t from_src;

    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with collision: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    space = (cpSpace *)plugin_chipmunk_env_space(chipmunk_obj->m_env->m_env);
    assert(space);

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with collision: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    TAILQ_FOREACH(from_src, &with_collision->m_srcs, m_next) {
        if (from_src->m_res[0]) {
            UI_OBJECT_URL buf;
            UI_OBJECT_URL * url;
            const char * path;

            if (from_src->m_res[0] == ':') {
                struct mem_buffer buffer;
                const char * res;
                
                mem_buffer_init(&buffer, module->m_alloc);
                
                res = ui_sprite_fsm_action_try_calc_str(&buffer, from_src->m_res + 1, fsm_action, NULL, module->m_em);
                if (res == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk with collision: calc def %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), from_src->m_res + 1);
                    mem_buffer_clear(&buffer);
                    return -1;
                }

                url = ui_object_ref_parse(res, &buf, module->m_em);
                if (url == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk with collision: parse url %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
                    mem_buffer_clear(&buffer);
                    return -1;
                }

                mem_buffer_clear(&buffer);
            }
            else {
                url = ui_object_ref_parse(from_src->m_res, &buf, module->m_em);
                if (url == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): chipmunk with collision: parse url %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), from_src->m_res);
                    return -1;
                }
            }

            if (url->type != UI_OBJECT_TYPE_CHIPMUNK) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk with collision: url type error!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }
            path = url->data.chipmunk.src.data.by_path.path;
            
            if (ui_sprite_chipmunk_with_collision_enter_load_from_src(
                    module, with_collision, entity, transform, space, chipmunk_obj, from_src, url, path)
                != 0)
            {
                goto LOAD_ERROR;
            }
        }
        else {
            if (ui_sprite_chipmunk_with_collision_body_create(
                    entity, chipmunk_obj, with_collision, transform, space, NULL, from_src)
                == NULL)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk with collision: create with collision body fail!!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                goto LOAD_ERROR;
            }
        }        
    }
    
    return 0;

LOAD_ERROR:
    while(!TAILQ_EMPTY(&with_collision->m_bodies)) {
        ui_sprite_chipmunk_with_collision_body_free(with_collision, TAILQ_FIRST(&with_collision->m_bodies));
    }

    return -1;
}

static void ui_sprite_chipmunk_with_collision_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_collision_t with_collision = (ui_sprite_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;

    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with collision: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    while(!TAILQ_EMPTY(&with_collision->m_bodies)) {
        ui_sprite_chipmunk_with_collision_body_free(with_collision, TAILQ_FIRST(&with_collision->m_bodies));
    }
}

static int ui_sprite_chipmunk_with_collision_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    //ui_sprite_chipmunk_module_t module = ctx;
    //ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_with_collision_t with_collision = (ui_sprite_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
    with_collision->m_module = (ui_sprite_chipmunk_module_t)ctx;

    TAILQ_INIT(&with_collision->m_srcs);
    TAILQ_INIT(&with_collision->m_bodies);

    return 0;
}

static void ui_sprite_chipmunk_with_collision_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_collision_t with_collision = (ui_sprite_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);

    assert(TAILQ_EMPTY(&with_collision->m_bodies));

    while(!TAILQ_EMPTY(&with_collision->m_srcs)) {
        ui_sprite_chipmunk_with_collision_src_free(TAILQ_FIRST(&with_collision->m_srcs));
    }
}

static int ui_sprite_chipmunk_with_collision_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_with_collision_t to_with_collision = (ui_sprite_chipmunk_with_collision_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_collision_t from_with_collision = (ui_sprite_chipmunk_with_collision_t)ui_sprite_fsm_action_data(from);
    ui_sprite_chipmunk_with_collision_src_t from_src;
    ui_sprite_chipmunk_with_collision_shape_t from_shape;

    if (ui_sprite_chipmunk_with_collision_init(to, ctx)) return -1;

    TAILQ_FOREACH(from_src, &from_with_collision->m_srcs, m_next) {
        ui_sprite_chipmunk_with_collision_src_t to_src = ui_sprite_chipmunk_with_collision_src_create(to_with_collision);
        if (to_src == NULL) {
            ui_sprite_chipmunk_with_collision_clear(to, ctx);
            return -1;
        }

        cpe_str_dup(to_src->m_res, sizeof(to_src->m_res), from_src->m_res);
        cpe_str_dup(to_src->m_name, sizeof(to_src->m_name), from_src->m_name);
        to_src->m_body_attrs = from_src->m_body_attrs;
        to_src->m_is_main = from_src->m_is_main;

        TAILQ_FOREACH(from_shape, &from_src->m_shapes, m_next) {
            ui_sprite_chipmunk_with_collision_shape_t to_shape
                = ui_sprite_chipmunk_with_collision_shape_clone(to_src, from_shape);
            if (to_shape == NULL) {
                ui_sprite_chipmunk_with_collision_clear(to, ctx);
                return -1;
            }
        }
    }
    
    return 0;
}

int ui_sprite_chipmunk_with_collision_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_COLLISION_NAME, sizeof(struct ui_sprite_chipmunk_with_collision));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with collision register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_collision_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_collision_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_collision_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_collision_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_collision_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_COLLISION_NAME, ui_sprite_chipmunk_with_collision_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_collision_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_COLLISION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_COLLISION_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_COLLISION_NAME = "chipmunk-with-collision";


#ifdef __cplusplus
}
#endif
    
