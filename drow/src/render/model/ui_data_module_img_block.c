#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_res.h"
#include "ui_data_module_i.h"
#include "ui_data_src_i.h"

ui_data_img_block_t ui_data_img_block_create(ui_data_module_t module) {
    ui_data_mgr_t mgr = module->m_mgr;
    ui_data_img_block_t block;

    block = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_img_block));
    if (block == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in module %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, module->m_src));
        return NULL;
    }

    block->m_module = module;
    bzero(&block->m_data, sizeof(block->m_data));
    block->m_data.id = (uint32_t)-1;
    block->m_using_res = NULL;

    TAILQ_INSERT_TAIL(&module->m_img_blocks, block, m_next_for_module);

    return block;
}

void ui_data_img_block_free(ui_data_img_block_t block) {
    ui_data_module_t module = block->m_module;
    ui_data_mgr_t mgr = module->m_mgr;

    TAILQ_REMOVE(&module->m_img_blocks, block, m_next_for_module);

    if (block->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&mgr->m_img_blocks, block);
    }

    mem_free(mgr->m_alloc, block);
}

int ui_data_img_block_set_id(ui_data_img_block_t block, uint32_t id) {
    ui_data_mgr_t mgr = block->m_module->m_mgr;
    uint32_t old_id;

    old_id = block->m_data.id;

    if (block->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&mgr->m_img_blocks, block);
    }

    block->m_data.id = id;

    if (block->m_data.id != (uint32_t)-1) {
        cpe_hash_entry_init(&block->m_hh_for_mgr);
        if (cpe_hash_table_insert_unique(&mgr->m_img_blocks, block) != 0) {
            block->m_data.id = old_id;
            if (old_id != (uint32_t)-1) {
                cpe_hash_table_insert_unique(&mgr->m_img_blocks, block);
            }
            return -1;
        }
    }

    return 0;
}

ui_data_img_block_t ui_data_img_block_find_by_id(ui_data_module_t module, uint32_t id) {
    ui_data_img_block_t r;
    struct ui_data_img_block key;
    key.m_module = module;
    key.m_data.id = id;

    r = cpe_hash_table_find(&module->m_mgr->m_img_blocks, &key);

    if (r == NULL && module->m_src->m_base_src) {
        key.m_module = ui_data_src_product(module->m_src->m_base_src);
        r = cpe_hash_table_find(&module->m_mgr->m_img_blocks, &key);
    }

    return r;
}

ui_cache_res_t ui_data_img_block_using_texture(ui_data_img_block_t block) {
    if (block->m_using_res == NULL) {
        ui_data_mgr_t mgr = block->m_module->m_mgr;
        const char * path = ui_data_img_block_using_texture_path(block);

        if (path[0] == 0) {
            CPE_ERROR(
                mgr->m_em, "%s.%s(%u): no texture",
                ui_data_src_path_dump(&mgr->m_dump_buffer, block->m_module->m_src),
                ui_data_img_block_name(block), block->m_data.id);
            return NULL;
        }
        
        if (mgr->m_cache_mgr == NULL) {
            CPE_ERROR(
                      mgr->m_em, "%s.%s(%u): no cache mgr",
                      ui_data_src_path_dump(&mgr->m_dump_buffer, block->m_module->m_src),
                      ui_data_img_block_name(block), block->m_data.id);
            return NULL;
        }

        block->m_using_res = ui_cache_res_find_by_path(mgr->m_cache_mgr, path);
        if (block->m_using_res == NULL) {
            CPE_ERROR(
                mgr->m_em, "%s.%s(%u): texture %s not exist!",
                ui_data_src_path_dump(&mgr->m_dump_buffer, block->m_module->m_src),
                ui_data_img_block_name(block), block->m_data.id, path);
            return NULL;
        }
    }

    return block->m_using_res;
}

const char * ui_data_img_block_using_texture_path(ui_data_img_block_t block) {
    return ui_data_src_msg(block->m_module->m_src, block->m_data.use_img);
}

const char * ui_data_img_block_name(ui_data_img_block_t block) {
    return ui_data_src_msg(block->m_module->m_src, block->m_data.name_id);
}

ui_data_img_block_t ui_data_img_block_find_by_name(ui_data_module_t module, const char * name) {
    ui_data_img_block_t img_block;
    
    TAILQ_FOREACH(img_block, &module->m_img_blocks, m_next_for_module) {
        if (strcmp(ui_data_img_block_name(img_block), name) == 0) return img_block;
    }

    return NULL;
}

ui_data_module_t ui_data_img_block_module(ui_data_img_block_t block) {
    return block->m_module;
}

static ui_data_img_block_t ui_data_img_block_in_module_next(struct ui_data_img_block_it * it) {
    ui_data_img_block_t * data = (ui_data_img_block_t *)(it->m_data);
    ui_data_img_block_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_module);

    return r;
}

void ui_data_img_block_in_module(ui_data_img_block_it_t it, ui_data_module_t module) {
    *(ui_data_img_block_t *)(it->m_data) = TAILQ_FIRST(&module->m_img_blocks);
    it->next = ui_data_img_block_in_module_next;
}

ui_data_src_t ui_data_img_block_src(ui_data_img_block_t img_block) {
    return img_block->m_module->m_src;
}

UI_IMG_BLOCK * ui_data_img_block_data(ui_data_img_block_t img_block) {
    return &img_block->m_data;
}

uint8_t ui_data_img_block_is_scale_style(ui_data_img_block_t block) {
    return block->m_data.flag & UI_IMG_BLOCK_FLAG_SCALESTYPE;
}

uint8_t ui_data_img_block_is_alpha_blend(ui_data_img_block_t block) {
    return block->m_data.flag & UI_IMG_BLOCK_FLAG_ALPHA_BLEND;
}

LPDRMETA ui_data_img_block_meta(ui_data_mgr_t data_mgr) {
    return data_mgr->m_meta_img_block;
}

void ui_data_img_block_bounding_rect(ui_data_img_block_t img_block, ui_rect_t bounding) {
    bounding->lt.x = bounding->lt.y = 0.0f;
    bounding->rb.x = img_block->m_data.src_w;
    bounding->rb.y = img_block->m_data.src_h;
}

uint32_t ui_data_img_block_hash(const ui_data_img_block_t block) {
    return block->m_module->m_src->m_id & block->m_data.id;
}

int ui_data_img_block_eq(const ui_data_img_block_t l, const ui_data_img_block_t r) {
    return l->m_data.id == r->m_data.id && l->m_module == r->m_module;
}
