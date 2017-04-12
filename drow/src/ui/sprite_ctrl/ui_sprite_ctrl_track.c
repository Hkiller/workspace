#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui/sprite_render/ui_sprite_render_layer.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_ctrl_track_i.h"
#include "ui_sprite_ctrl_track_mgr_i.h"

ui_sprite_ctrl_track_t
ui_sprite_ctrl_track_create(
    ui_sprite_ctrl_track_mgr_t track_mgr, const char * name, const char * type_name)
{
    ui_sprite_ctrl_module_t module = track_mgr->m_module;
    ui_sprite_ctrl_track_meta_t track_meta;
    ui_sprite_ctrl_track_t track;
    size_t name_len = strlen(name) + 1;

    if (ui_sprite_ctrl_track_find(track_mgr, name) != NULL) {
        CPE_ERROR(module->m_em, "create track %s: already exit!", name);
        return NULL;
    } 

    track_meta = ui_sprite_ctrl_track_meta_find(track_mgr, type_name);
    if (track_meta == NULL) {
        CPE_ERROR(module->m_em, "create track %s: type %s not exit!", name, type_name);
        return NULL;
    }

    track = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_ctrl_track) + name_len);
    if (track == NULL) {
        CPE_ERROR(module->m_em, "create track %s: alloc fail!", name);
        return NULL;
    }

    memcpy(track + 1, name, name_len);

    track->m_track_mgr = track_mgr;
    track->m_name = (const char *)(track + 1);
    track->m_meta = track_meta;

    track->m_next_point_meta = NULL;

    track->m_is_show = 0;

    track->m_point_capacity = 0;
    track->m_point_count = 0;
    track->m_points = NULL;

    TAILQ_INSERT_TAIL(&track_mgr->m_tracks, track, m_next);

    return track;
}

void ui_sprite_ctrl_track_free(ui_sprite_ctrl_track_t track) {
    ui_sprite_ctrl_track_mgr_t track_mgr = track->m_track_mgr;

    if (ui_sprite_ctrl_track_is_show(track)) {
        ui_sprite_ctrl_track_hide(track);
    }

    if (track->m_points) {
        mem_free(track_mgr->m_module->m_alloc, track->m_points);
        track->m_points = NULL;
        track->m_point_capacity = 0;
        track->m_point_count = 0;
    }

    TAILQ_REMOVE(&track_mgr->m_tracks, track, m_next);

    mem_free(track_mgr->m_module->m_alloc, track);
}

ui_sprite_ctrl_track_t ui_sprite_ctrl_track_find(ui_sprite_ctrl_track_mgr_t track_mgr, const char * name) {
    ui_sprite_ctrl_track_t track;

    TAILQ_FOREACH(track, &track_mgr->m_tracks, m_next) {
        if (strcmp(track->m_name, name) == 0) return track;
    }

    return NULL;
}

int ui_sprite_ctrl_track_show(ui_sprite_ctrl_track_t track) {
    ui_sprite_ctrl_module_t module = track->m_track_mgr->m_module;
    ui_sprite_world_t world;
    ui_sprite_render_env_t render_env;
    uint16_t i;

    if (track->m_is_show) return 0;

    world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(track->m_track_mgr));
    render_env = ui_sprite_render_env_find(world);

    if (render_env == NULL) {
        CPE_ERROR(
            module->m_em, "track %s(%s): show: no anim, can`t show!",
            track->m_name, track->m_meta->m_type_name);
        return -1;
    }

    for(i = 0; i < track->m_point_count; ++i) {
        struct ui_sprite_ctrl_track_point * point = track->m_points + i;

        if (ui_sprite_ctrl_track_point_show(point, track, render_env) != 0) {
            ui_sprite_ctrl_track_hide(track);
            return -1;
        }
    }

    track->m_is_show = 1;

    return 0;
}

uint8_t ui_sprite_ctrl_track_is_show(ui_sprite_ctrl_track_t track) {
    return track->m_is_show;
}

void ui_sprite_ctrl_track_hide(ui_sprite_ctrl_track_t track) {
    uint16_t i;

    if (!track->m_is_show) return;

    for(i = 0; i < track->m_point_count; ++i) {
        ui_sprite_ctrl_track_point_hide(track->m_points + i, track);
    }

    track->m_is_show = 0;
}

static int ui_sprite_ctrl_track_append_point(
    ui_sprite_ctrl_track_t track, ui_vector_2 pt, ui_sprite_ctrl_track_point_meta_t point_meta)
{
    ui_sprite_ctrl_module_t module = track->m_track_mgr->m_module;
    struct ui_sprite_ctrl_track_point * point;

    if (track->m_point_count + 1 > track->m_point_capacity) {
        uint16_t new_capacity = track->m_point_capacity < 16 ? 16 : track->m_point_capacity * 2;
        struct ui_sprite_ctrl_track_point * new_points = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_ctrl_track_point) * new_capacity);
        if (new_points == NULL) {
            CPE_ERROR(
                module->m_em, "track %s(%s): append point: malloc fail!",
                track->m_name, track->m_meta->m_type_name);
            return -1;
        }

        if (track->m_points) {
            memcpy(new_points, track->m_points, sizeof(struct ui_sprite_ctrl_track_point) * track->m_point_count);
            mem_free(module->m_alloc, track->m_points);
        }

        track->m_points = new_points;
        track->m_point_capacity = new_capacity;
    }

    assert(track->m_points);
    assert(track->m_point_count + 1 <= track->m_point_capacity);

    point = &track->m_points[track->m_point_count++];
    point->m_point = pt;
    point->m_point_meta = point_meta;
    point->m_anim = 0;

    return 0;
}

static int ui_sprite_ctrl_track_add_point_on_new_point(ui_sprite_ctrl_track_t track) {
    ui_sprite_ctrl_module_t module = track->m_track_mgr->m_module;
    ui_sprite_ctrl_track_point_t new_point = track->m_points + (track->m_point_count - 1);

    track->m_next_point_meta = TAILQ_NEXT(new_point->m_point_meta, m_next);
    if (track->m_next_point_meta == TAILQ_END(&track->m_meta->m_point_metas)) {
        track->m_next_point_meta = TAILQ_FIRST(&track->m_meta->m_point_metas);
    }

    if (ui_sprite_ctrl_track_is_show(track)) {
        ui_sprite_world_t world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(track->m_track_mgr));
        ui_sprite_render_env_t render_env = ui_sprite_render_env_find(world);

        if (render_env == NULL) {
            CPE_ERROR(
                module->m_em, "track %s(%s): append point: no anim, can`t show!",
                track->m_name, track->m_meta->m_type_name);
            return -1;
        }
        else {
            ui_sprite_ctrl_track_point_show(new_point, track, render_env);
        }
    }

    return 0;
}

int ui_sprite_ctrl_track_add_point(ui_sprite_ctrl_track_t track, ui_vector_2 pt) {
    ui_sprite_ctrl_module_t module = track->m_track_mgr->m_module;

    if (TAILQ_EMPTY(&track->m_meta->m_point_metas)) {
        CPE_ERROR(
            module->m_em, "track %s(%s): append point: point meta empty!",
            track->m_name, track->m_meta->m_type_name);
        return -1;
    }

    if (track->m_next_point_meta == NULL) {
        assert(track->m_point_count == 0);

        if (ui_sprite_ctrl_track_append_point(track, pt, TAILQ_FIRST(&track->m_meta->m_point_metas)) != 0) return -1;

        track->m_last_input_point = pt;
        track->m_last_interval = 0.0f;

        if (ui_sprite_ctrl_track_add_point_on_new_point(track) != 0) return -1;
    }
    else {
        float inc_interval;

        assert(track->m_point_count > 0);

        inc_interval = cpe_math_distance(track->m_last_input_point.x, track->m_last_input_point.y, pt.x, pt.y);
        if (track->m_next_point_meta->m_interval <= 0.0f) {
            if (ui_sprite_ctrl_track_append_point(track, pt, track->m_next_point_meta) != 0) return -1;
            track->m_last_input_point = pt;
            track->m_last_interval = 0.0f;
            if (ui_sprite_ctrl_track_add_point_on_new_point(track) != 0) return -1;
        }
        else {
            while(track->m_last_interval + inc_interval > track->m_next_point_meta->m_interval) {
                float require_interval = track->m_next_point_meta->m_interval - track->m_last_interval;
                float percent = require_interval / inc_interval;
                ui_vector_2 target_pt;

                target_pt.x = track->m_last_input_point.x + (pt.x - track->m_last_input_point.x) * percent;
                target_pt.y = track->m_last_input_point.y + (pt.y - track->m_last_input_point.y) * percent;
                
                if (ui_sprite_ctrl_track_append_point(track, target_pt, track->m_next_point_meta) != 0) return -1;

                track->m_last_input_point = target_pt;
                track->m_last_interval = 0.0f;
                inc_interval -= require_interval;
                if (ui_sprite_ctrl_track_add_point_on_new_point(track) != 0) return -1;
            }

            track->m_last_input_point = pt;
            track->m_last_interval += inc_interval;
        }
    }

    return 0;
}

int ui_sprite_ctrl_track_point_show(
    ui_sprite_ctrl_track_point_t point, ui_sprite_ctrl_track_t track, ui_sprite_render_env_t render_env)
{
    ui_sprite_ctrl_module_t module = track->m_track_mgr->m_module;
    ui_sprite_render_layer_t render_layer;
    
    if (point->m_anim) return 0;

    if (render_env == NULL) {
        CPE_ERROR(
            module->m_em, "track %s(%s): show point: no anim, can`t show!",
            track->m_name, track->m_meta->m_type_name);
        return -1;
    }

    render_layer = ui_sprite_render_layer_find(render_env, track->m_meta->m_anim_layer);
    if (render_layer == NULL) {
        CPE_ERROR(
            module->m_em, "track %s: show: layer %s not exist!",
            track->m_name, track->m_meta->m_anim_layer);
        return -1;
    }

    point->m_anim = ui_sprite_render_anim_create_by_res(render_layer, point->m_point_meta->m_res, NULL, NULL);
    if (point->m_anim == NULL) {
        CPE_ERROR(
            module->m_em, "track %s(%s): show: show point %s fail!",
            track->m_name, track->m_meta->m_type_name, point->m_point_meta->m_res);
        return -1;
    } 

    return 0;
}

void ui_sprite_ctrl_track_point_hide(ui_sprite_ctrl_track_point_t point, ui_sprite_ctrl_track_t track) {
    if (point->m_anim) {
        ui_sprite_render_anim_free(point->m_anim);
        point->m_anim = NULL;
    }
}
