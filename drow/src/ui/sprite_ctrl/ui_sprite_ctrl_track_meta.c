#include "ui_sprite_ctrl_track_meta_i.h"
#include "ui_sprite_ctrl_track_mgr_i.h"

ui_sprite_ctrl_track_meta_t
ui_sprite_ctrl_track_meta_create(ui_sprite_ctrl_track_mgr_t track_mgr, const char * name, const char * anim_layer) {
    ui_sprite_ctrl_module_t module = track_mgr->m_module;
    ui_sprite_ctrl_track_meta_t track_meta;
    size_t name_len = strlen(name) + 1;
    size_t layer_len = strlen(anim_layer) + 1;

    track_meta = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_ctrl_track_meta) + name_len + layer_len);
    if (track_meta == NULL) {
        CPE_ERROR(module->m_em, "create track meta %s: alloc fail!", name);
        return NULL;
    }

    memcpy(track_meta + 1, name, name_len);
    memcpy(((char *)(track_meta + 1)) + name_len, anim_layer, layer_len);

    track_meta->m_track_mgr = track_mgr;
    track_meta->m_type_name = (const char *)(track_meta + 1);
    track_meta->m_anim_layer = track_meta->m_type_name + name_len;
    TAILQ_INIT(&track_meta->m_point_metas);

    TAILQ_INSERT_TAIL(&track_mgr->m_metas, track_meta, m_next);

    return track_meta;
}

void ui_sprite_ctrl_track_meta_free(ui_sprite_ctrl_track_meta_t track_meta) {
    ui_sprite_ctrl_track_mgr_t track_mgr = track_meta->m_track_mgr;

    while(!TAILQ_EMPTY(&track_meta->m_point_metas)) {
        ui_sprite_ctrl_track_meta_remove_point(track_meta, TAILQ_FIRST(&track_meta->m_point_metas));
    }

    TAILQ_REMOVE(&track_mgr->m_metas, track_meta, m_next);

    mem_free(track_mgr->m_module->m_alloc, track_meta);
}

ui_sprite_ctrl_track_meta_t ui_sprite_ctrl_track_meta_find(ui_sprite_ctrl_track_mgr_t track_mgr, const char * name) {
    ui_sprite_ctrl_track_meta_t track_meta;

    
    TAILQ_FOREACH(track_meta, &track_mgr->m_metas, m_next) {
        if (strcmp(track_meta->m_type_name, name) == 0) return track_meta;
    }

    return NULL;
}

int ui_sprite_ctrl_track_meta_add_point(ui_sprite_ctrl_track_meta_t track_meta, float interval, const char * res) {
    ui_sprite_ctrl_track_mgr_t track_mgr = track_meta->m_track_mgr;
    ui_sprite_ctrl_track_point_meta_t point_meta;
    size_t res_len = strlen(res) + 1;

    point_meta = mem_alloc(track_mgr->m_module->m_alloc, sizeof(struct ui_sprite_ctrl_track_point_meta) + res_len);
    if (point_meta == NULL) {
        CPE_ERROR(track_mgr->m_module->m_em, "track meta %s: add point, alloc fail!", track_meta->m_type_name);
        return -1;
    }

    memcpy(point_meta + 1, res, res_len);

    point_meta->m_interval = interval;
    point_meta->m_res = (const char *)(point_meta + 1);

    TAILQ_INSERT_TAIL(&track_meta->m_point_metas, point_meta, m_next);

    return 0;
};

void ui_sprite_ctrl_track_meta_remove_point(ui_sprite_ctrl_track_meta_t track_meta, ui_sprite_ctrl_track_point_meta_t point_meta) {
    ui_sprite_ctrl_track_mgr_t track_mgr = track_meta->m_track_mgr;
    TAILQ_REMOVE(&track_meta->m_point_metas, point_meta, m_next);
    mem_free(track_mgr->m_module->m_alloc, point_meta);
}
