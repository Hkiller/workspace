#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "ui_data_sprite_i.h"
#include "ui_data_src_i.h"

ui_data_frame_collision_t ui_data_frame_collision_create(ui_data_frame_t frame) {
    ui_data_mgr_t mgr = frame->m_sprite->m_mgr;
    ui_data_frame_collision_t collision;

    collision = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_frame_collision));
    if (collision == NULL) {
        CPE_ERROR(
            mgr->m_em, "create collision in frame %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, frame->m_sprite->m_src));
        return NULL;
    }

    collision->m_frame = frame;
    bzero(&collision->m_data, sizeof(collision->m_data));

    frame->m_collision_count++;
    TAILQ_INSERT_TAIL(&frame->m_collisions, collision, m_next_for_frame);

    return collision;
}

void ui_data_frame_collision_free(ui_data_frame_collision_t collision) {
    ui_data_frame_t frame = collision->m_frame;
    ui_data_mgr_t mgr = frame->m_sprite->m_mgr;

    TAILQ_REMOVE(&frame->m_collisions, collision, m_next_for_frame);
    frame->m_collision_count--;

    mem_free(mgr->m_alloc, collision);
}

ui_data_frame_collision_t ui_data_frame_collision_get_at(ui_data_frame_t frame, uint32_t index) {
    ui_data_frame_collision_t collision;

    TAILQ_FOREACH(collision, &frame->m_collisions, m_next_for_frame) {
        if (index == 0) return collision;
        index--;
    }

    return NULL;
}

static ui_data_frame_collision_t ui_data_frame_collision_in_frame_next(struct ui_data_frame_collision_it * it) {
    ui_data_frame_collision_t * data = (ui_data_frame_collision_t *)(it->m_data);
    ui_data_frame_collision_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_frame);

    return r;
}

void ui_data_frame_collisions(ui_data_frame_collision_it_t it, ui_data_frame_t frame) {
    *(ui_data_frame_collision_t *)(it->m_data) = TAILQ_FIRST(&frame->m_collisions);
    it->next = ui_data_frame_collision_in_frame_next;
}

UI_COLLISION * ui_data_frame_collision_data(ui_data_frame_collision_t collision) {
    return &collision->m_data;
}

LPDRMETA ui_data_frame_collision_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_collision;
}

ui_data_frame_t ui_data_frame_collision_frame(ui_data_frame_collision_t collision) {
    return collision->m_frame;
}

ui_data_src_t ui_data_frame_collision_src(ui_data_frame_collision_t collision) {
    return collision->m_frame->m_sprite->m_src;
}
