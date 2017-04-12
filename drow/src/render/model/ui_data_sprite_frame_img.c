#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "ui_data_sprite_i.h"
#include "ui_data_src_i.h"
#include "ui_data_src_src_i.h"

ui_data_frame_img_t ui_data_frame_img_create(ui_data_frame_t frame) {
    ui_data_mgr_t mgr = frame->m_sprite->m_mgr;
    ui_data_frame_img_t img_ref;

    img_ref = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_frame_img));
    if (img_ref == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in frame %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, frame->m_sprite->m_src));
        return NULL;
    }

    img_ref->m_frame = frame;
    bzero(&img_ref->m_data, sizeof(img_ref->m_data));

    frame->m_img_count++;
    TAILQ_INSERT_TAIL(&frame->m_img_refs, img_ref, m_next_for_frame);

    return img_ref;
}

void ui_data_frame_img_free(ui_data_frame_img_t img_ref) {
    ui_data_frame_t frame = img_ref->m_frame;
    ui_data_mgr_t mgr = frame->m_sprite->m_mgr;

    TAILQ_REMOVE(&frame->m_img_refs, img_ref, m_next_for_frame);
    frame->m_img_count--;

    mem_free(mgr->m_alloc, img_ref);
}

ui_data_frame_img_t ui_data_frame_img_get_at(ui_data_frame_t frame, uint32_t index) {
    ui_data_frame_img_t img_ref;

    TAILQ_FOREACH(img_ref, &frame->m_img_refs, m_next_for_frame) {
        if (index == 0) return img_ref;
        index--;
    }

    return NULL;
}

static ui_data_frame_img_t ui_data_frame_img_in_frame_next(struct ui_data_frame_img_it * it) {
    ui_data_frame_img_t * data = (ui_data_frame_img_t *)(it->m_data);
    ui_data_frame_img_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_frame);

    return r;
}

void ui_data_frame_imgs(ui_data_frame_img_it_t it, ui_data_frame_t frame) {
    *(ui_data_frame_img_t *)(it->m_data) = TAILQ_FIRST(&frame->m_img_refs);
    it->next = ui_data_frame_img_in_frame_next;
}

UI_IMG_REF * ui_data_frame_img_data(ui_data_frame_img_t img_ref) {
    return &img_ref->m_data;
}

LPDRMETA ui_data_frame_img_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_frame_img;
}

ui_data_frame_t ui_data_frame_img_frame(ui_data_frame_img_t img_ref) {
    return img_ref->m_frame;
}

ui_data_src_t ui_data_frame_img_src(ui_data_frame_img_t img_ref) {
    return img_ref->m_frame->m_sprite->m_src;
}

ui_data_img_block_t ui_data_frame_img_using_img_block(ui_data_frame_img_t img_ref) {
    ui_data_mgr_t data_mgr = img_ref->m_frame->m_sprite->m_src->m_mgr;
    ui_data_src_t img_src;
    ui_data_module_t module;
    
    assert(data_mgr);

    img_src = ui_data_src_find_by_id(data_mgr, img_ref->m_data.module_id);
    if (img_src == NULL) return NULL;

    module = ui_data_src_product(img_src);
    if (module == NULL) return NULL;

    return ui_data_img_block_find_by_id(module, img_ref->m_data.img_block_id);
}

