#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "plugin/moving/plugin_moving_node.h"
#include "plugin/moving/plugin_moving_plan_node.h"
#include "plugin/particle/plugin_particle_obj_plugin_data.h"
#include "plugin_scrollmap_obj_i.h"
#include "plugin_scrollmap_env_i.h"
#include "plugin_scrollmap_layer_i.h"
#include "plugin_scrollmap_team_i.h"

plugin_scrollmap_obj_t
plugin_scrollmap_obj_create(plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, ui_transform_t transform) {
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_obj_t obj;

    obj = TAILQ_FIRST(&env->m_free_objs);
    if (obj) {
        TAILQ_REMOVE(&env->m_free_objs, obj, m_next_for_env);
    }
    else {
        obj = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_obj) + env->m_obj_capacity);
        if (obj == NULL) return NULL;
    }

    obj->m_env = env;
    obj->m_layer = layer;

    if (transform) {
        obj->m_transform = *transform;
    }
    else {
        obj->m_transform = UI_TRANSFORM_IDENTITY;
    }

    obj->m_is_created = 0;
    obj->m_is_move_suspend = 0;
    obj->m_accept_scale = 0;
    obj->m_accept_angle = 0;
    obj->m_range = 0.0f;
    obj->m_move_state = plugin_scrollmap_obj_move_free;

    env->m_obj_count++;
    TAILQ_INSERT_TAIL(&env->m_objs, obj, m_next_for_env);

    return obj;
}

void plugin_scrollmap_obj_free(plugin_scrollmap_obj_t obj) {
    plugin_scrollmap_env_t env = obj->m_env;

    if (obj->m_is_created) {
        env->m_obj_on_destory(env->m_obj_factory_ctx, obj);
        obj->m_is_created = 0;
    }

    plugin_scrollmap_obj_set_move_free(obj);

    assert(env->m_obj_count > 0);
    env->m_obj_count--;
    TAILQ_REMOVE(&env->m_objs, obj, m_next_for_env);

    TAILQ_INSERT_HEAD(&env->m_free_objs, obj, m_next_for_env);
}

void plugin_scrollmap_obj_real_free(plugin_scrollmap_obj_t obj) {
    plugin_scrollmap_env_t env = obj->m_env;

    TAILQ_REMOVE(&env->m_free_objs, obj, m_next_for_env);

    mem_free(env->m_module->m_alloc, obj);
}

plugin_scrollmap_obj_move_state_t plugin_scrollmap_obj_move_state(plugin_scrollmap_obj_t obj) {
    return obj->m_move_state;
}

void * plugin_scrollmap_obj_data(plugin_scrollmap_obj_t obj) {
    return obj + 1;
}

ui_transform_t plugin_scrollmap_obj_transform(plugin_scrollmap_obj_t obj) {
    return &obj->m_transform;
}

void plugin_scrollmap_obj_set_transform(plugin_scrollmap_obj_t obj, ui_transform_t transform) {
    obj->m_transform = *transform;
    if (obj->m_is_created) {
        obj->m_env->m_obj_on_update(obj->m_env->m_obj_factory_ctx, obj);
    }
}

float plugin_scrollmap_obj_range(plugin_scrollmap_obj_t obj) {
    return obj->m_range;
}

void plugin_scrollmap_obj_set_range(plugin_scrollmap_obj_t obj, float range) {
    obj->m_range = range;
}

uint8_t plugin_scrollmap_obj_is_created(plugin_scrollmap_obj_t obj) {
    return obj->m_is_created;
}

int plugin_scrollmap_obj_do_create(plugin_scrollmap_obj_t obj, const char * obj_type, const char * args) {
    char buf[32];

    assert(!obj->m_is_created);
    assert(obj_type);

    if (args) {
        if (cpe_str_read_arg_range(buf, sizeof(buf), args, args + strlen(args), "angle", ',', '=') == 0) {
            obj->m_accept_angle = atoi(buf);
        }

        if (cpe_str_read_arg_range(buf, sizeof(buf), args, args + strlen(args), "scane", ',', '=') == 0) {
            obj->m_accept_scale = atoi(buf);
        }
    }
    
    if (obj->m_env->m_obj_on_init(obj->m_env->m_obj_factory_ctx, obj, obj_type, args) != 0) {
        CPE_ERROR(obj->m_env->m_module->m_em, "plugin_scrollmap_obj_create: init obj fail");
        return -1;
    }
    obj->m_is_created = 1;
        
    return 0;
}

void plugin_scrollmap_obj_do_remove(plugin_scrollmap_obj_t obj) {
    if (obj->m_is_created) {
        obj->m_env->m_obj_on_destory(obj->m_env->m_obj_factory_ctx, obj);
        obj->m_is_created = 0;
    }
}

uint8_t plugin_scrollmap_obj_is_move_suspend(plugin_scrollmap_obj_t obj) {
    return obj->m_is_move_suspend;
}

void plugin_scrollmap_obj_set_move_suspend(plugin_scrollmap_obj_t obj, uint8_t move_suspend) {
    if (move_suspend) move_suspend = 1;

    if (move_suspend == obj->m_is_move_suspend) return;

    obj->m_is_move_suspend = move_suspend;

    if(obj->m_move_state == plugin_scrollmap_obj_move_by_team) {
        switch(obj->m_move_by_team.m_team->m_type) {
        case plugin_scrollmap_team_type_moving:
            assert(obj->m_move_by_team.m_node);
            plugin_moving_node_set_time_scale(obj->m_move_by_team.m_node, move_suspend ? 0.0f : 1.0f);
            break;
        case plugin_scrollmap_team_type_particle:
            assert(obj->m_move_by_team.m_particle);
            plugin_particle_obj_particle_set_time_scale(obj->m_move_by_team.m_particle, move_suspend ? 0.0f : 1.0f);
            break;
        case plugin_scrollmap_team_type_spine:
            break;
        default:
            break;
        }
    }
}

plugin_scrollmap_team_t plugin_scrollmap_obj_team(plugin_scrollmap_obj_t obj) {
    return obj->m_move_state == plugin_scrollmap_obj_move_by_team ? obj->m_move_by_team.m_team : NULL;
}

plugin_scrollmap_layer_t plugin_scrollmap_obj_layer(plugin_scrollmap_obj_t obj) {
    return obj->m_layer;
}

void plugin_scrollmap_obj_set_layer(plugin_scrollmap_obj_t obj, plugin_scrollmap_layer_t layer) {
    assert(layer);

    if (obj->m_layer == layer) return;

    if (obj->m_move_state == plugin_scrollmap_obj_move_by_layer) {
        TAILQ_REMOVE(&obj->m_layer->m_land_objs, obj, m_move_by_layer.m_next);
        TAILQ_INSERT_TAIL(&layer->m_land_objs, obj, m_move_by_layer.m_next);
    }

    obj->m_layer = layer;
}

void plugin_scrollmap_obj_set_move_free(plugin_scrollmap_obj_t obj) {
    switch(obj->m_move_state) {
    case plugin_scrollmap_obj_move_free:
        break;
    case plugin_scrollmap_obj_move_by_layer:
        TAILQ_REMOVE(&obj->m_layer->m_land_objs, obj, m_move_by_layer.m_next);
        obj->m_move_state = plugin_scrollmap_obj_move_free;
        break;
    case plugin_scrollmap_obj_move_by_team:
        TAILQ_REMOVE(&obj->m_move_by_team.m_team->m_members, obj, m_move_by_team.m_next);
        obj->m_move_state = plugin_scrollmap_obj_move_free;

        switch(obj->m_move_by_team.m_team->m_type) {
        case plugin_scrollmap_team_type_moving:
            assert(obj->m_move_by_team.m_node);
            plugin_moving_node_free(obj->m_move_by_team.m_node);
            obj->m_move_by_team.m_node = NULL;
            break;
        case plugin_scrollmap_team_type_particle:
            plugin_scrollmap_obj_unset_move_by_particle_team(obj);
            break;
        case plugin_scrollmap_team_type_spine:
            assert(obj->m_move_by_team.m_bone);
            obj->m_move_by_team.m_bone = NULL;
            break;
        default:
            break;
        }
        break;
    default:
        assert(0);
        break;
    }
}

void plugin_scrollmap_obj_set_move_by_layer(plugin_scrollmap_obj_t obj) {
    if (obj->m_move_state != plugin_scrollmap_obj_move_free) {
        plugin_scrollmap_obj_set_move_free(obj);
    }

    obj->m_move_state = plugin_scrollmap_obj_move_by_layer;
    TAILQ_INSERT_TAIL(&obj->m_layer->m_land_objs, obj, m_move_by_layer.m_next);
}

const char * plugin_scrollmap_obj_analize_name(const char * input, char * buf, size_t buf_len, char * * args) {
    char * sep_begin;
    char * sep_end;

    if (args) *args = NULL;

    sep_begin = strchr(input, '[');
    if (sep_begin == NULL) {
        return input;
    }

    sep_end = strchr(sep_begin, ']');
    if (sep_end == NULL) {
        if (args) *args = NULL;
        return NULL;
    }

    if (cpe_str_dup_range(buf, buf_len, input, sep_end) == NULL) return NULL;

    sep_begin = buf + (sep_begin - input);

    *sep_begin = 0;

    if (args) *args = sep_begin + 1;

    return buf;
}
