#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_src.h"
#include "ui_data_sprite_i.h"
#include "ui_data_module_i.h"
#include "ui_data_src_i.h"

ui_data_frame_t ui_data_frame_create(ui_data_sprite_t sprite) {
    ui_data_mgr_t mgr = sprite->m_mgr;
    ui_data_frame_t frame;

    frame = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_frame));
    if (frame == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in sprite %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, sprite->m_src));
        return NULL;
    }

    frame->m_sprite = sprite;
    bzero(&frame->m_data, sizeof(frame->m_data));
    frame->m_data.id = (uint32_t)-1;

    TAILQ_INIT(&frame->m_img_refs);
    frame->m_img_count = 0;

    TAILQ_INIT(&frame->m_collisions);
    frame->m_collision_count = 0;

    sprite->m_frame_count++;
    TAILQ_INSERT_TAIL(&sprite->m_frames, frame, m_next_for_sprite);

    return frame;
}

void ui_data_frame_free(ui_data_frame_t frame) {
    ui_data_sprite_t sprite = frame->m_sprite;
    ui_data_mgr_t mgr = sprite->m_mgr;

    while(!TAILQ_EMPTY(&frame->m_img_refs)) {
        ui_data_frame_img_free(TAILQ_FIRST(&frame->m_img_refs));
    }
    assert(frame->m_img_count == 0);

    while(!TAILQ_EMPTY(&frame->m_collisions)) {
        ui_data_frame_collision_free(TAILQ_FIRST(&frame->m_collisions));
    }
    assert(frame->m_collision_count == 0);
    
    if (frame->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&mgr->m_frames, frame);
    }

    TAILQ_REMOVE(&sprite->m_frames, frame, m_next_for_sprite);
    sprite->m_frame_count--;

    mem_free(mgr->m_alloc, frame);
}

int ui_data_frame_set_id(ui_data_frame_t frame, uint32_t id) {
    ui_data_mgr_t mgr = frame->m_sprite->m_mgr;
    uint32_t old_id;

    old_id = frame->m_data.id;

    if (frame->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&mgr->m_frames, frame);
    }

    frame->m_data.id = id;

    if (frame->m_data.id != (uint32_t)-1) {
        cpe_hash_entry_init(&frame->m_hh_for_mgr);
        if (cpe_hash_table_insert_unique(&mgr->m_frames, frame) != 0) {
            frame->m_data.id = old_id;
            if (old_id != (uint32_t)-1) {
                cpe_hash_table_insert_unique(&mgr->m_frames, frame);
            }
            return -1;
        }
    }

    return 0;
}

ui_data_sprite_t ui_data_frame_sprite(ui_data_frame_t frame) {
    return frame->m_sprite;
}

ui_data_src_t ui_data_frame_src(ui_data_frame_t frame) {
    return frame->m_sprite->m_src;
}

uint32_t ui_data_frame_img_count(ui_data_frame_t frame) {
    return frame->m_img_count;
}

uint32_t ui_data_frame_collision_count(ui_data_frame_t frame) {
    return frame->m_collision_count;
}

int ui_data_frame_bounding_rect(ui_data_frame_t frame, ui_rect_t rect) {
    if (frame->m_data.bound_custom) {
        rect->lt.x = frame->m_data.bounding.lt;
        rect->lt.y = frame->m_data.bounding.tp;
        rect->rb.x = frame->m_data.bounding.rt;
        rect->rb.y = frame->m_data.bounding.bm;
    }
    else if (frame->m_data.accept_scale) {
        ui_data_frame_img_t img_lt = TAILQ_FIRST(&frame->m_img_refs);
        ui_data_frame_img_t img_ct = img_lt ? TAILQ_NEXT(img_lt, m_next_for_frame) : NULL;
        ui_data_frame_img_t img_rt = img_ct ? TAILQ_NEXT(img_ct, m_next_for_frame) : NULL;
        ui_data_frame_img_t img_lc = img_rt ? TAILQ_NEXT(img_rt, m_next_for_frame) : NULL;        
        ui_data_frame_img_t img_cc = img_lc ? TAILQ_NEXT(img_lc, m_next_for_frame) : NULL;        
        ui_data_frame_img_t img_rc = img_cc ? TAILQ_NEXT(img_cc, m_next_for_frame) : NULL;        
        ui_data_frame_img_t img_lb = img_rc ? TAILQ_NEXT(img_rc, m_next_for_frame) : NULL;        
        ui_data_frame_img_t img_cb = img_lb ? TAILQ_NEXT(img_lb, m_next_for_frame) : NULL;
        //ui_data_frame_img_t img_rb = img_cb ? TAILQ_NEXT(img_cb, m_next_for_frame) : NULL;

        ui_data_img_block_t block_lc = img_lc ? ui_data_frame_img_using_img_block(img_lc) : NULL;
        ui_data_img_block_t block_rc = img_rc ? ui_data_frame_img_using_img_block(img_rc) : NULL;
        ui_data_img_block_t block_ct = img_ct ? ui_data_frame_img_using_img_block(img_ct) : NULL;
        ui_data_img_block_t block_cb = img_cb ? ui_data_frame_img_using_img_block(img_cb) : NULL;
        ui_data_img_block_t block_cc = img_cc ? ui_data_frame_img_using_img_block(img_cc) : NULL;

		uint32_t lt = block_lc ? block_lc->m_data.src_w : 0;
		uint32_t rt = block_rc ? block_rc->m_data.src_w : 0;
		uint32_t tp = block_ct ? block_ct->m_data.src_h : 0;
		uint32_t bm = block_cb ? block_cb->m_data.src_h : 0;
		uint32_t cw = block_cc ? block_cc->m_data.src_w : 0;
		uint32_t ch = block_cc ? block_cc->m_data.src_h : 0;

        rect->lt.x = 0.0f;
        rect->rb.x = lt + cw + rt;
        rect->lt.y = 0.0f;
        rect->rb.y = tp + ch + bm;
	}
    else {
        ui_data_mgr_t data_mgr = frame->m_sprite->m_mgr;
        ui_data_frame_img_t img;
        ui_data_module_t cur_module;
        uint32_t cur_module_id = 0;

        bzero(rect, sizeof(*rect));
    
        TAILQ_FOREACH(img, &frame->m_img_refs, m_next_for_frame) {
            ui_data_img_block_t img_block;
            ui_rect tmp_rect;

            if (cur_module_id != img->m_data.module_id) {
                ui_data_src_t module_src = ui_data_src_find_by_id(data_mgr, img->m_data.module_id);
                if (module_src == NULL) {
                    CPE_ERROR(data_mgr->m_em, "frame_bounding_rect: using module "FMT_UINT32_T" not exist!", img->m_data.module_id);
                    return -1;
                }

                cur_module = ui_data_src_product(module_src);
                if (cur_module == NULL) {
                    CPE_ERROR(data_mgr->m_em, "frame_bounding_rect: using module "FMT_UINT32_T" not loaded!", img->m_data.module_id);
                    return -1;
                }
            }

            img_block = ui_data_img_block_find_by_id(cur_module, img->m_data.img_block_id);
            if (img_block == NULL) {
                CPE_ERROR(data_mgr->m_em, "frame_bounding_rect: module "FMT_UINT32_T": img "FMT_UINT32_T" not exist!", img->m_data.module_id, img->m_data.img_block_id);
                return -1;
            }

			///@retval						0 ²»·­×ª 
			///@retval						1 xÖá 
			///@retval						2 yÖá 
			///@retval						3 ¾µÏñ 
            switch(img->m_data.trans.world_trans.flips ^ img->m_data.trans.local_trans.flips) {
            case 0:
                tmp_rect.lt.x = img->m_data.trans.world_trans.trans.value[0];
                tmp_rect.lt.y = img->m_data.trans.world_trans.trans.value[1];
                tmp_rect.rb.x = tmp_rect.lt.x + img_block->m_data.src_w * img->m_data.trans.world_trans.scale.value[0];
                tmp_rect.rb.y= tmp_rect.lt.y + img_block->m_data.src_h * img->m_data.trans.world_trans.scale.value[1];
                break;
            case 1:
                tmp_rect.lt.x = img->m_data.trans.world_trans.trans.value[0] - img_block->m_data.src_w * img->m_data.trans.world_trans.scale.value[0];
                tmp_rect.lt.y = img->m_data.trans.world_trans.trans.value[1];
                tmp_rect.rb.x = img->m_data.trans.world_trans.trans.value[0];
                tmp_rect.rb.y = tmp_rect.lt.y + img_block->m_data.src_h * img->m_data.trans.world_trans.scale.value[1];
                break;
            case 2:
                tmp_rect.lt.x = img->m_data.trans.world_trans.trans.value[0];
                tmp_rect.lt.y = img->m_data.trans.world_trans.trans.value[1] - img_block->m_data.src_h * img->m_data.trans.world_trans.scale.value[1];
                tmp_rect.rb.x = tmp_rect.lt.x + img_block->m_data.src_w * img->m_data.trans.world_trans.scale.value[0];
                tmp_rect.rb.y = img->m_data.trans.world_trans.trans.value[1];
                break;
            case 3:
                tmp_rect.lt.x = img->m_data.trans.world_trans.trans.value[0] - img_block->m_data.src_w * img->m_data.trans.world_trans.scale.value[0];
                tmp_rect.lt.y = img->m_data.trans.world_trans.trans.value[1] - img_block->m_data.src_h * img->m_data.trans.world_trans.scale.value[1];
                tmp_rect.rb.x = img->m_data.trans.world_trans.trans.value[0];
                tmp_rect.rb.y = img->m_data.trans.world_trans.trans.value[1];
                break;
            }
			//printf("name =%s, w=%d, value=%d\n", img_block->m_data.name, img_block->m_data.src_w, img_block->m_data.src_h);
			//printf("xxxxx: tmp_rect: (%d,%d)-(%d,%d)\n", tmp_rect.lt.x, tmp_rect.lt.y, tmp_rect.rb.x, tmp_rect.rb.y);
            if (!ui_rect_is_valid(rect)) {
                *rect = tmp_rect;
            }
            else {
                ui_rect_inline_union(rect, &tmp_rect);
            }

   //         printf("xxxxx at (%f,%f), w=%d, h=%d, scale=(%f,%f)\n",
   //                img->m_data.trans.world_trans.trans.value[0], img->m_data.trans.world_trans.trans.value[1],
   //                img_block->m_data.src_w, img_block->m_data.src_h, img->m_data.trans.world_trans.scale.value[0], img->m_data.trans.world_trans.scale.value[1]);
   //         
        }
    }

    return 0;
}

ui_data_frame_t ui_data_frame_find_by_id(ui_data_sprite_t sprite, uint32_t id) {
    struct ui_data_frame key;
    key.m_sprite = sprite;
    key.m_data.id = id;

    return cpe_hash_table_find(&sprite->m_mgr->m_frames, &key);
}

ui_data_frame_t ui_data_frame_find_by_name(ui_data_sprite_t sprite, const char * name) {
    ui_data_frame_t frame;

    TAILQ_FOREACH(frame, &sprite->m_frames, m_next_for_sprite) {
        if (strcmp(frame->m_data.name, name) == 0) return frame;
    }

    return NULL;
}

const char * ui_data_frame_name(ui_data_frame_t frame) {
    return frame->m_data.name;
}

static ui_data_frame_t ui_data_frame_in_sprite_next(ui_data_frame_it_t it) {
    ui_data_frame_t * data = (ui_data_frame_t *)(it->m_data);
    ui_data_frame_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_sprite);

    return r;
}

void ui_data_sprite_frames(ui_data_frame_it_t it, ui_data_sprite_t sprite) {
    *(ui_data_frame_t *)(it->m_data) = TAILQ_FIRST(&sprite->m_frames);
    it->next = ui_data_frame_in_sprite_next;
}

UI_FRAME * ui_data_frame_data(ui_data_frame_t frame) {
    return &frame->m_data;
}

LPDRMETA ui_data_frame_meta(ui_data_mgr_t mgr) {
    return mgr->m_meta_frame;
}

uint32_t ui_data_frame_hash(const ui_data_frame_t frame) {
    return frame->m_sprite->m_src->m_id & frame->m_data.id;
}

int ui_data_frame_eq(const ui_data_frame_t l, const ui_data_frame_t r) {
    return l->m_data.id == r->m_data.id && l->m_sprite == r->m_sprite;
}
