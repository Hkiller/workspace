#include "cpe/utils/string_utils.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/binpack.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_group.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/model_ed/ui_ed_src.h"
#include "plugin_pack_texture_i.h"
#include "plugin_pack_texture_part_i.h"
#include "plugin_pack_block_i.h"
#include "plugin_pack_block_ref_i.h"

plugin_pack_texture_t
plugin_pack_texture_create(plugin_pack_packer_t packer, const char * path) {
    plugin_pack_module_t module = packer->m_module;
    plugin_pack_texture_t texture;

    texture = mem_alloc(packer->m_module->m_alloc, sizeof(struct plugin_pack_texture));
    if (texture == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_texture: create: alloc fail!");
        return NULL;
    }

    texture->m_packer = packer;
    texture->m_part_count = 0;
    texture->m_path = cpe_str_mem_dup(module->m_alloc, path);
    if (texture->m_path == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_texture: create: dup path %s fail!", path);
        mem_free(module->m_alloc, texture);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &texture->m_blocks,
            module->m_alloc,
            (cpe_hash_fun_t)plugin_pack_block_hash,
            (cpe_hash_eq_t)plugin_pack_block_eq,
            CPE_HASH_OBJ2ENTRY(plugin_pack_block, m_hh),
            -1) != 0)
    {
        CPE_ERROR(module->m_em, "plugin_pack_texture: create: init block hashtable fail!");
        mem_free(module->m_alloc, texture->m_path);
        mem_free(module->m_alloc, texture);
        return NULL;
    }
    
    TAILQ_INIT(&texture->m_parts);
    
    TAILQ_INSERT_TAIL(&packer->m_textures, texture, m_next);
    
    return texture;
}

void plugin_pack_texture_free(plugin_pack_texture_t texture) {
    plugin_pack_module_t module = texture->m_packer->m_module;
    
    plugin_pack_block_free_all(texture);
    cpe_hash_table_fini(&texture->m_blocks);

    while(!TAILQ_EMPTY(&texture->m_parts)) {
        plugin_pack_texture_part_free(TAILQ_FIRST(&texture->m_parts));
    }

    mem_free(module->m_alloc, texture->m_path);
    
    TAILQ_REMOVE(&texture->m_packer->m_textures, texture, m_next);

    mem_free(module->m_alloc, texture);
}

/* ui_cache_res_t plugin_pack_texture_cache_texture(plugin_pack_texture_t pack_texture) { */
/*     return pack_texture->m_cache_texture; */
/* } */
    
static plugin_pack_texture_t plugin_pack_texture_next(struct plugin_pack_texture_it * it) {
    plugin_pack_texture_t * data = (plugin_pack_texture_t *)(it->m_data);
    plugin_pack_texture_t r = *data;

    if (r) {
        *data = TAILQ_NEXT(r, m_next);
    }

    return r;
}

void plugin_pack_packer_textures(plugin_pack_texture_it_t it, plugin_pack_packer_t packer) {
    *(plugin_pack_texture_t *)(it->m_data) = TAILQ_FIRST(&packer->m_textures);
    it->next = plugin_pack_texture_next;
}

plugin_pack_block_ref_t
plugin_pack_texture_add_block_ref(
    plugin_pack_texture_t texture, ui_data_src_t src, ui_ed_obj_t ed_obj,
    ui_cache_res_t res, ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect)
{
    plugin_pack_module_t module = texture->m_packer->m_module;
    plugin_pack_block_t block;
    struct cpe_md5_value md5_value;
    plugin_pack_block_ref_t block_ref;

    /* printf("xxxx: add block ref (%d,%d)-(%d,%d)\n", */
    /*        rect->boundary.lt, rect->boundary.tp, rect->boundary.rt, rect->boundary.bm); */
    
    if (ui_cache_pixel_buf_rect_md5(&md5_value, buf, rect, module->m_em) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_pack_texture_add_block_ref: %s %s using texture %s calc block md5 fail!",
            ui_data_src_type_name(ui_data_src_type(src)),
            ui_data_src_path_dump(&module->m_dump_buffer, src),
            ui_cache_res_path(res));
        return NULL;
    }

    block = plugin_pack_block_find(texture, &md5_value);
    if (block == NULL) {
        block = plugin_pack_texture_place(texture, &md5_value, res, rect);
        if (block == NULL) return NULL;
    }

    block_ref = plugin_pack_block_ref_create(block, ed_obj);
    if (block_ref == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_pack_texture_add_block_ref: %s %s using texture %s create block ref fail!",
            ui_data_src_type_name(ui_data_src_type(src)),
            ui_data_src_path_dump(&module->m_dump_buffer, src),
            ui_cache_res_path(res));
        return NULL;
    }

    return block_ref;
}

int plugin_pack_texture_commit(plugin_pack_texture_t texture) {
    plugin_pack_module_t module = texture->m_packer->m_module;
    plugin_pack_texture_part_t part;
    int rv = 0;
    struct cpe_hash_it block_it;
    plugin_pack_block_t block;
    
    if (texture->m_part_count == 1) {
        if (plugin_pack_texture_part_commit(TAILQ_FIRST(&texture->m_parts), texture->m_path) != 0) rv = -1;
    }
    else {
        const char * sep;
        size_t path_len;
        int i = 0;
        void * data;

        sep = strrchr(texture->m_path, '.');
        if (sep == NULL) {
            CPE_ERROR(module->m_em, "plugin_pack_texture_commit: find sep fail!");
            return -1;
        }
        path_len = sep - texture->m_path;
        
        mem_buffer_clear_data(&module->m_dump_buffer);
        data = mem_buffer_alloc(&module->m_dump_buffer, path_len + 30);
        if (data == NULL) {
            CPE_ERROR(module->m_em, "plugin_pack_texture_commit: alloc buf fail!");
            return -1;
        }
        memcpy(data, texture->m_path, path_len);

        TAILQ_FOREACH(part, &texture->m_parts, m_next) {
            snprintf(((char*)data) + path_len, 30, "-%d%s", i++, sep);
            if (plugin_pack_texture_part_commit(part, data) != 0) rv = -1;
        }
    }

    cpe_hash_it_init(&block_it, &texture->m_blocks);
    while((block = cpe_hash_it_next(&block_it))) {
        plugin_pack_block_ref_t block_ref;

        if (block->m_part) continue;
        
        TAILQ_FOREACH(block_ref, &block->m_refs, m_next) {
            switch(ui_ed_obj_type_id(block_ref->m_ed_obj)) {
            case ui_ed_obj_type_img_block:
                ui_ed_src_touch(ui_ed_obj_src(block_ref->m_ed_obj));
                plugin_pack_packer_update_img_block(block_ref, NULL);
                break;
            case ui_ed_obj_type_particle_emitter:
                ui_ed_src_touch(ui_ed_obj_src(block_ref->m_ed_obj));
                plugin_pack_packer_update_particle_emitter(block_ref, NULL);
                break;
            default:
                break;
            }
        }
    }
    
    return rv;
}

plugin_pack_block_t
plugin_pack_texture_place(
    plugin_pack_texture_t texture, cpe_md5_value_t md5, ui_cache_res_t src_texture, ui_cache_pixel_buf_rect_t src_rect)
{
    plugin_pack_texture_part_t part;
    struct binpack_rect placed_pack;
    plugin_pack_block_t block;
    
    part = plugin_pack_texture_alloc(texture, src_rect, &placed_pack);

    block = plugin_pack_block_create(texture, part, md5, src_texture, src_rect);
    if (block == NULL) return NULL;
    
    if (part) {
        block->m_to_rect.level = 0;
        block->m_to_rect.boundary_lt = placed_pack.x;
        block->m_to_rect.boundary_tp = placed_pack.y;
        block->m_to_rect.boundary_rt = block->m_to_rect.boundary_lt + placed_pack.width;
        block->m_to_rect.boundary_bm = block->m_to_rect.boundary_tp + placed_pack.height;
    }
    
    return block;
}
