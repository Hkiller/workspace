#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "plugin/moving/plugin_moving_env.h"
#include "plugin/moving/plugin_moving_control.h"
#include "plugin/moving/plugin_moving_plan.h"
#include "plugin/moving/plugin_moving_plan_node.h"
#include "plugin/moving/plugin_moving_plan_segment.h"
#include "plugin/moving/plugin_moving_node.h"
#include "plugin_scrollmap_team_i.h"
#include "plugin_scrollmap_obj_i.h"
#include "plugin_scrollmap_env_i.h"
#include "plugin_scrollmap_obj_type_map_i.h"

int plugin_scrollmap_team_init_from_moving(plugin_scrollmap_team_t team, ui_data_src_t src, plugin_moving_plan_t plan) {
    plugin_scrollmap_module_t module = team->m_env->m_module;
    plugin_moving_control_t moving_control;
    struct plugin_moving_plan_node_it node_it;
    plugin_moving_plan_node_t plan_node;

    moving_control = plugin_moving_control_create(team->m_env->m_moving_env, plan);
    if (moving_control == NULL) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: create moving control fail!");
        return -1;
    }
    
    team->m_type = plugin_scrollmap_team_type_moving;
    team->m_moving.m_control = moving_control;

    /*load objs*/
    plugin_moving_plan_nodes(&node_it, plan);
    while((plan_node = plugin_moving_plan_node_it_next(&node_it))) {
        plugin_scrollmap_obj_t obj;

        obj = plugin_scrollmap_obj_create(team->m_env, team->m_layer, NULL);
        if (obj == NULL) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: create obj fail!");
            return -1;
        }

        if (plugin_scrollmap_obj_set_move_by_moving_team(obj, team, plan_node, 1) != 0) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: set obj to team fail!");
            plugin_scrollmap_obj_free(obj);
            return -1;
        }
    }

    return 0;
}

void plugin_scrollmap_team_update_moving(plugin_scrollmap_team_t team, float delta) {
    plugin_scrollmap_env_t env = team->m_env;
    plugin_scrollmap_obj_t member;
    plugin_scrollmap_obj_t member_next;

    plugin_moving_control_update(team->m_moving.m_control, delta);
    for(member = TAILQ_FIRST(&team->m_members); member; member = member_next) {
        ui_vector_2 pos;

        member_next = TAILQ_NEXT(member, m_move_by_team.m_next);
    
        switch(plugin_moving_node_state(member->m_move_by_team.m_node)) {
        case plugin_moving_node_state_init:
            break;
        case plugin_moving_node_state_working:
            if (!member->m_is_created) {
                const char * obj_type;
                const char * proto_name;
                char * args;
                char buf[128];
                plugin_scrollmap_obj_type_map_t type_map;
                
                obj_type = plugin_moving_plan_node_data(plugin_moving_node_plan_node(member->m_move_by_team.m_node))->name;

                proto_name = plugin_scrollmap_obj_analize_name(obj_type, buf, sizeof(buf), &args);
                if (proto_name == NULL) {
                    CPE_ERROR(team->m_env->m_module->m_em, "on member update: parse type %s fail", obj_type);
                    plugin_scrollmap_obj_free(member);
                    continue;
                }

                type_map = plugin_scrollmap_obj_type_map_find_on_team(team, proto_name);
                if (plugin_scrollmap_obj_do_create(member, type_map ? type_map->m_to_type : proto_name, args) != 0) {
                    CPE_ERROR(team->m_env->m_module->m_em, "on member update: do create %s fail", proto_name);
                    plugin_scrollmap_obj_free(member);
                    continue;
                }
            }

            pos = *plugin_moving_node_pos(member->m_move_by_team.m_node);

            if (env->m_resize_policy_x == plugin_scrollmap_resize_policy_percent) pos.x *= env->m_logic_size_adj.x;
            if (env->m_resize_policy_y == plugin_scrollmap_resize_policy_percent) pos.y *= env->m_logic_size_adj.y;

            assert(member->m_is_created);
            ui_transform_set_pos_2(&member->m_transform, &pos);

            if (ui_transform_cmp(&team->m_transform, &UI_TRANSFORM_IDENTITY) != 0) {
                ui_transform_adj_by_parent(&member->m_transform, &team->m_transform);
            }
    
            env->m_obj_on_update(env->m_obj_factory_ctx, member);
            break;
        case plugin_moving_node_state_done:
            plugin_scrollmap_obj_free(member);
            break;
        default:
            CPE_ERROR(
                env->m_module->m_em, "on member update: unknown node state %d",
                plugin_moving_node_state(member->m_move_by_team.m_node));
            plugin_scrollmap_obj_free(member);
            break;
        }
    }

    if (TAILQ_EMPTY(&team->m_members)) {
        plugin_scrollmap_team_free(team);
    }
}

void plugin_scrollmap_team_fini_moving(plugin_scrollmap_team_t team) {
    assert(team->m_moving.m_control);
    plugin_moving_control_free(team->m_moving.m_control);
    team->m_moving.m_control = NULL;
}

static void plugin_scrollmap_obj_on_moving_node_update(void * ctx, plugin_moving_node_t node, plugin_moving_node_event_t evt) {
    plugin_scrollmap_obj_t obj = ctx;
    plugin_scrollmap_env_t env = obj->m_move_by_team.m_team->m_env;
    plugin_moving_plan_segment_t segment;
    MOVING_PLAN_SEGMENT const * segment_data;

    if (evt == plugin_moving_node_event_segment_begin) {
        segment = plugin_moving_node_cur_segment(node);
        assert(segment);

        segment_data = plugin_moving_plan_segment_data(segment);
        assert(segment);

        if (segment_data->on_begin[0]) {
            env->m_obj_on_event(env->m_obj_factory_ctx, obj, segment_data->on_begin);
        }
    }
    else if (evt == plugin_moving_node_event_segment_end) {
        segment = plugin_moving_node_cur_segment(node);
        assert(segment);

        segment_data = plugin_moving_plan_segment_data(segment);
        assert(segment);

        if (segment_data->on_end[0]) {
            env->m_obj_on_event(env->m_obj_factory_ctx, obj, segment_data->on_end);
        }
    }
}

int plugin_scrollmap_obj_set_move_by_moving_team(
    plugin_scrollmap_obj_t obj, plugin_scrollmap_team_t team, plugin_moving_plan_node_t plan_node, uint8_t loop_count)
{
    plugin_moving_node_t moving_node;
    ui_transform transform = UI_TRANSFORM_IDENTITY;
    
    assert(team->m_type == plugin_scrollmap_team_type_moving);
    
    moving_node = plugin_moving_node_create(team->m_moving.m_control, plan_node, loop_count);
    if (moving_node == NULL) return -1;
    
    if (obj->m_move_state != plugin_scrollmap_obj_move_free) {
        plugin_scrollmap_obj_set_move_free(obj);
    }

    obj->m_move_state = plugin_scrollmap_obj_move_by_team;
    obj->m_move_by_team.m_team = team;
    obj->m_move_by_team.m_node = moving_node;
    TAILQ_INSERT_TAIL(&team->m_members, obj, m_move_by_team.m_next);

    plugin_moving_node_set_update_fun(moving_node, plugin_scrollmap_obj_on_moving_node_update, obj);

    ui_transform_set_pos_2(&transform, plugin_moving_node_pos(obj->m_move_by_team.m_node));
    plugin_scrollmap_obj_set_transform(obj, &transform);
    
    return 0;
}
