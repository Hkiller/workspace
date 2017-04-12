#include <assert.h>
#include "spine/Skeleton.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_obj_part.h"
#include "plugin/spine/plugin_spine_obj_part_state.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin_scrollmap_team_i.h"
#include "plugin_scrollmap_obj_i.h"
#include "plugin_scrollmap_env_i.h"
#include "plugin_scrollmap_obj_type_map_i.h"

int plugin_scrollmap_team_init_from_spine(plugin_scrollmap_team_t team, ui_data_src_t src, plugin_spine_data_skeleton_t spine_data) {
    plugin_scrollmap_module_t module = team->m_env->m_module;
    ui_runtime_render_obj_t render_obj;
    plugin_spine_obj_t spine_obj;
    struct spSkeleton* skeleton;
    uint16_t bone_pos;
    const char * cfg_prefix = "ETY_";
    
    render_obj = ui_runtime_render_obj_create_by_type(module->m_runtime, NULL, "skeleton");
    if (render_obj == NULL) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: create spine obj fail!");
        return -1;
    }
    ui_runtime_render_obj_set_src(render_obj, src);

    spine_obj = ui_runtime_render_obj_data(render_obj);
    if (plugin_spine_obj_set_data(spine_obj, spine_data) != 0) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: set spine obj fail!");
        ui_runtime_render_obj_free(render_obj);
        return -1;
    }

    if (plugin_spine_obj_scane_parts(spine_obj) != 0) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: scane parts fail!");
        ui_runtime_render_obj_free(render_obj);
        return -1;
    }
    ui_runtime_render_obj_update(render_obj, 0.0f);
    
    team->m_type = plugin_scrollmap_team_type_spine;
    team->m_spine.m_obj = spine_obj;

    skeleton = plugin_spine_obj_skeleton(spine_obj);

    for(bone_pos = 0; bone_pos < skeleton->bonesCount; ++bone_pos) {
        spBone* bone = skeleton->bones[bone_pos];
        plugin_scrollmap_obj_t obj;
        char * args;
        const char * proto_name;
        char buf[128];
        plugin_scrollmap_obj_type_map_t type_map;

        if (!cpe_str_start_with(bone->data->name, cfg_prefix)) continue;

        obj = plugin_scrollmap_obj_create(team->m_env, team->m_layer, NULL);
        if (obj == NULL) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: create obj fail!");
            ui_runtime_render_obj_free(render_obj);            
            return -1;
        }

        if (plugin_scrollmap_obj_set_move_by_spine_team(obj, team, bone) != 0) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: set obj to team fail!");
            plugin_scrollmap_obj_free(obj);
            ui_runtime_render_obj_free(render_obj);            
            return -1;
        }

        proto_name = bone->data->name + 4; /*ETY_*/
        proto_name = plugin_scrollmap_obj_analize_name(proto_name, buf, sizeof(buf), &args);

        type_map = plugin_scrollmap_obj_type_map_find_on_team(team, proto_name);
        if (plugin_scrollmap_obj_do_create(obj, type_map ? type_map->m_to_type : proto_name, args) != 0) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: create obj fail!");
            plugin_scrollmap_obj_free(obj);
            ui_runtime_render_obj_free(render_obj);            
            return -1;
        }
    }

    return 0;
}

static void plugin_scrollmap_team_update_spine_member(plugin_scrollmap_team_t team, plugin_scrollmap_obj_t member) {
    ui_transform bone_transform;
    struct spBone * bone;

    bone = member->m_move_by_team.m_bone;
    if (bone == NULL) return;
        
    if (plugin_spine_bone_calc_transform(bone, &bone_transform) != 0) return;

    if (!member->m_accept_scale) {
        ui_transform_set_scale(&bone_transform, &UI_VECTOR_3_IDENTITY);
    }

    if (!member->m_accept_angle) {
        ui_transform_set_quation(&bone_transform, &UI_QUATERNION_IDENTITY);
    }

    if (ui_transform_cmp(&team->m_transform, &UI_TRANSFORM_IDENTITY) != 0) {
        ui_transform_adj_by_parent(&bone_transform, &team->m_transform);
    }
    
    member->m_transform = bone_transform;

    member->m_env->m_obj_on_update(member->m_env->m_obj_factory_ctx, member);
}

void plugin_scrollmap_team_update_spine(plugin_scrollmap_team_t team, float delta) {
    ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_from_data(team->m_spine.m_obj);
    plugin_scrollmap_obj_t member;
    plugin_scrollmap_obj_t member_next;
    plugin_spine_obj_part_t main_part;
    plugin_spine_obj_part_state_t main_part_state;
    
    ui_runtime_render_obj_update(render_obj, delta);

    for(member = TAILQ_FIRST(&team->m_members); member; member = member_next) {
        member_next = TAILQ_NEXT(member, m_next_for_env);

        if (!member->m_is_created) continue;
        plugin_scrollmap_team_update_spine_member(team, member);
    }

    main_part = plugin_spine_obj_part_find(team->m_spine.m_obj, "main");
    if (main_part == NULL) {
        CPE_ERROR(team->m_env->m_module->m_em, "team no main part!");
        plugin_scrollmap_team_free(team);
        return;
    }

    if (plugin_spine_obj_part_is_in_enter(main_part)) return;
    
    main_part_state = plugin_spine_obj_part_cur_state(main_part);
    if (main_part_state == NULL) {
        CPE_ERROR(team->m_env->m_module->m_em, "team main part no state!");
        plugin_scrollmap_team_free(team);
        return;
    }

    if (strcmp(plugin_spine_obj_part_state_name(main_part_state), "done") == 0) {
        plugin_scrollmap_team_free(team);
    }
}

void plugin_scrollmap_team_fini_spine(plugin_scrollmap_team_t team) {
    assert(team->m_spine.m_obj);
    ui_runtime_render_obj_free(ui_runtime_render_obj_from_data(team->m_spine.m_obj));
    team->m_spine.m_obj = NULL;
}

int plugin_scrollmap_obj_set_move_by_spine_team(
    plugin_scrollmap_obj_t obj, plugin_scrollmap_team_t team,
    struct spBone * bone)
{
    assert(team->m_type == plugin_scrollmap_team_type_spine);

    if (obj->m_move_state != plugin_scrollmap_obj_move_free) {
        plugin_scrollmap_obj_set_move_free(obj);
    }

    obj->m_move_state = plugin_scrollmap_obj_move_by_team;
    obj->m_move_by_team.m_team = team;
    obj->m_move_by_team.m_bone = bone;
    TAILQ_INSERT_TAIL(&team->m_members, obj, m_move_by_team.m_next);

    if (obj->m_is_created) {
        plugin_scrollmap_team_update_spine_member(team, obj);
    }
    
    return 0;
}
