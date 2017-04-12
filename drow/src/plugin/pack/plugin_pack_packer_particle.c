#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_res.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin_pack_packer_i.h"
#include "plugin_pack_block_i.h"
#include "plugin_pack_block_ref_i.h"

int plugin_pack_packer_load_particle(plugin_pack_texture_t texture, ui_data_src_t src) {
    plugin_pack_packer_t packer = texture->m_packer;
    plugin_pack_module_t module = packer->m_module;
    ui_ed_src_t particle_src;
    struct ui_ed_obj_it emitter_obj_it;
    ui_ed_obj_t emitter_obj;
    int rv = 0;

    particle_src = ui_ed_src_check_create(module->m_ed_mgr, ui_data_src_path_dump(&module->m_dump_buffer, src), ui_data_src_type_particle);
    if (particle_src == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_packer_pack_particle: create particle ed src fail!");
        return -1;
    }

    ui_ed_obj_childs(&emitter_obj_it, ui_ed_src_root_obj(particle_src));
    while((emitter_obj = ui_ed_obj_it_next(&emitter_obj_it))) {
        UI_PARTICLE_EMITTER * emitter_data = ui_ed_obj_data(emitter_obj);
        ui_cache_res_t emitter_res;
        ui_cache_pixel_buf_t emitter_texture_buf;
        struct ui_cache_pixel_buf_rect rect;
        struct ui_ed_obj_it mod_obj_it;
        ui_ed_obj_t mod_obj;
        uint8_t skip_pack = 0;
        const char * texture_path;
        plugin_pack_block_ref_t block_ref;
        
        if (!emitter_data->is_render) continue;

        ui_ed_obj_childs(&mod_obj_it, emitter_obj);
        while((mod_obj = ui_ed_obj_it_next(&mod_obj_it))) {
            UI_PARTICLE_MOD * mod_data = ui_ed_obj_data(mod_obj);
            switch(mod_data->type) {
            case ui_particle_mod_type_texcoord_scroll_anim:
                skip_pack = 1;
                break;
            }
        }

        texture_path = plugin_particle_data_emitter_msg(ui_ed_obj_product(emitter_obj), emitter_data->texture_id);
        if (plugin_pack_packer_load_src_texture(packer, src, texture_path, &emitter_res, &emitter_texture_buf) != 0) {
            rv = -1;
            continue;
        }

        if (emitter_res == NULL) continue;

        if (skip_pack) {
            plugin_pack_packer_remove_pack_texture(packer, emitter_res);
            continue;
        }
        
        rect.level = 0;
        rect.boundary_lt = emitter_data->texture_x;
        rect.boundary_tp = emitter_data->texture_y;
        rect.boundary_rt = rect.boundary_lt + (emitter_data->texture_w ? emitter_data->texture_w : ui_cache_texture_width(emitter_res));
        rect.boundary_bm = rect.boundary_tp + (emitter_data->texture_h ? emitter_data->texture_h : ui_cache_texture_height(emitter_res));
        
        //printf("xxxxxx: rect=(%d,%d)-(%d,%d)\n", rect.boundary_lt, rect.boundary_tp, rect.boundary_rt, rect.boundary_bm);

        block_ref = plugin_pack_texture_add_block_ref(texture, src, emitter_obj, emitter_res, emitter_texture_buf, &rect);
        if (block_ref == NULL) {
            rv = -1;
            continue;
        }

        if (block_ref->m_block->m_part == NULL) {
            CPE_ERROR(
                module->m_em, "particle %s: emitter %s: size=(%d,%d), limit=(%d,%d) place fail!",
                ui_data_src_path_dump(&module->m_dump_buffer, src),
                ui_data_src_msg(src, emitter_data->name_id),
                rect.boundary_rt - rect.boundary_lt,
                rect.boundary_bm - rect.boundary_tp,
                packer->m_limit_width, packer->m_limit_height);
            rv = -1;
            continue;
        }
    }
    
    return rv;
}

void plugin_pack_packer_update_particle_emitter(plugin_pack_block_ref_t block_ref, const char * path) {
    plugin_pack_block_t block = block_ref->m_block;
    UI_PARTICLE_EMITTER * emitter_data = ui_ed_obj_data(block_ref->m_ed_obj);

    if (block->m_part) {
        emitter_data->texture_id = ui_ed_src_msg_update(ui_ed_obj_src(block_ref->m_ed_obj), emitter_data->texture_id, path);

        if (emitter_data->collision_atlas_w > 0 || emitter_data->collision_atlas_h > 0) {
            emitter_data->collision_atlas_x += block->m_to_rect.boundary_lt;
            emitter_data->collision_atlas_y += block->m_to_rect.boundary_tp;
        }

        if (emitter_data->atlas_w == 0) {
            emitter_data->atlas_w = block->m_to_rect.boundary_rt - block->m_to_rect.boundary_lt;
        }
        
        if (emitter_data->atlas_h == 0) {
            emitter_data->atlas_h = block->m_to_rect.boundary_bm - block->m_to_rect.boundary_tp;
        }
    
        emitter_data->atlas_x += block->m_to_rect.boundary_lt;
        emitter_data->atlas_y += block->m_to_rect.boundary_tp;

        emitter_data->texture_x = block->m_to_rect.boundary_lt;
        emitter_data->texture_y = block->m_to_rect.boundary_tp;
        emitter_data->texture_w = block->m_to_rect.boundary_rt - block->m_to_rect.boundary_lt;
        emitter_data->texture_h = block->m_to_rect.boundary_bm - block->m_to_rect.boundary_tp;
    }
    else {
        emitter_data->texture_id = 0;
    }
}
