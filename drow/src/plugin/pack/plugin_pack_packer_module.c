#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_language.h"
#include "render/model/ui_data_module.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/cache/ui_cache_res.h"
#include "plugin_pack_texture_i.h"
#include "plugin_pack_language_i.h"
#include "plugin_pack_block_i.h"
#include "plugin_pack_block_ref_i.h"

static ui_ed_obj_t plugin_pack_packer_find_obj_by_name(ui_ed_obj_t module_obj, const char * name);
static uint8_t plugin_pack_packer_is_block_in_language(plugin_pack_packer_t packer, ui_data_src_t src, const char * name);

int plugin_pack_packer_load_module(plugin_pack_texture_t texture, ui_data_src_t src) {
    plugin_pack_packer_t packer = texture->m_packer;
    plugin_pack_module_t module = packer->m_module;
    ui_data_src_t base_src;
    ui_ed_src_t base_module_src;
    ui_ed_obj_t base_module_obj;
    ui_ed_src_t module_src;
    ui_ed_obj_t module_obj;
    ui_cache_res_t source_texture;
    ui_cache_pixel_buf_t source_texture_buf;
    struct ui_ed_obj_it img_block_obj_it;
    ui_ed_obj_t img_block_obj;
    int rv = 0;
    
    if ((base_src = ui_data_src_base_src(src))) {
        base_module_src = ui_ed_src_check_create(module->m_ed_mgr, ui_data_src_path_dump(&module->m_dump_buffer, base_src), ui_data_src_type_module);
        if (base_module_src == NULL) {
            CPE_ERROR(module->m_em, "plugin_pack_packer_pack_module: create module ed src fail!");
            return -1;
        }

        base_module_obj = ui_ed_src_root_obj(base_module_src);
        assert(base_module_obj);
    }
    else {
        base_module_src = NULL;
        base_module_obj = NULL;
    }
    
    module_src = ui_ed_src_check_create(module->m_ed_mgr, ui_data_src_path_dump(&module->m_dump_buffer, src), ui_data_src_type_module);
    if (module_src == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_packer_pack_module: create module ed src fail!");
        return -1;
    }

    module_obj = ui_ed_src_root_obj(module_src);
    assert(module_obj);

    /*如果是语言包，先把所有id清除，后续根据基础宝的id进行设置 */
    if (base_module_obj) {
        ui_ed_obj_childs(&img_block_obj_it, module_obj);
        while((img_block_obj = ui_ed_obj_it_next(&img_block_obj_it))) {
            ui_ed_obj_set_id(img_block_obj, (uint32_t)-1);
        }
    }
    
    ui_ed_obj_childs(&img_block_obj_it, module_obj);
    while((img_block_obj = ui_ed_obj_it_next(&img_block_obj_it))) {
        UI_IMG_BLOCK * img_block_data = ui_ed_obj_data(img_block_obj);
        const char * block_name = ui_ed_src_msg(module_src, img_block_data->name_id);
        struct ui_cache_pixel_buf_rect rect;
        plugin_pack_block_ref_t block_ref;
        
        if (base_module_obj) {
            /*如果有基础module，则这个是语言相关的module，确保替代的块存在并且id相等 */
            ui_ed_obj_t base_obj = plugin_pack_packer_find_obj_by_name(base_module_obj, block_name);
            if (base_obj) {
                ui_ed_obj_set_id(img_block_obj, ((UI_IMG_BLOCK *)ui_ed_obj_data(base_obj))->id);
            }
        }
        else if (plugin_pack_packer_is_block_in_language(texture->m_packer, src, block_name)) {
            if (texture->m_packer->m_default_language) {
                /*如果是一个语言相关的块，如果属于默认语言，则放置到默认语言，否则直接丢弃 */
                if (texture->m_packer->m_language_count > 1) {
                    //TODO: move to language module
                }
            }
            else {
                /*如果是一个语言相关的块，如果属于默认语言，则放置到默认语言，否则直接丢弃 */
                //printf("xxxxx: %s  %s\n", ui_data_src_path_dump(&module->m_dump_buffer, src), img_block_data->name);
                ui_ed_obj_remove(img_block_obj);
                continue;
            }
        }
        
        rect.level = 0;
        rect.boundary_lt = img_block_data->src_x;
        rect.boundary_tp = img_block_data->src_y;
        rect.boundary_rt = img_block_data->src_x + img_block_data->src_w;
        rect.boundary_bm = img_block_data->src_y + img_block_data->src_h;

        if (plugin_pack_packer_load_src_texture(
                texture->m_packer, src, ui_ed_src_msg(module_src, img_block_data->use_img),
                &source_texture, &source_texture_buf) != 0) continue;
        if (source_texture == NULL) continue;
        
        block_ref = plugin_pack_texture_add_block_ref(texture, src, img_block_obj, source_texture, source_texture_buf, &rect);
        if (block_ref == NULL) {
            rv = -1;
            continue;
        }

        if (block_ref->m_block->m_part == NULL) {
            CPE_ERROR(
                module->m_em, "module %s: block %s(%d): size=(%d,%d) limit=(%d,%d) place fail!",
                ui_data_src_path_dump(&module->m_dump_buffer, src),
                block_name, img_block_data->id,
                img_block_data->src_w, img_block_data->src_h,
                packer->m_limit_width, packer->m_limit_height);
            rv = -1;
            continue;
        }
    }

    return rv;
}

void plugin_pack_packer_update_img_block(plugin_pack_block_ref_t block_ref, const char * path) {
    ui_ed_obj_t module_obj = ui_ed_obj_parent(block_ref->m_ed_obj);
    UI_IMG_BLOCK * img_block_data = ui_ed_obj_data(block_ref->m_ed_obj);

    if (block_ref->m_block->m_part) {
        assert(path);

        img_block_data->src_x = block_ref->m_block->m_to_rect.boundary_lt;
        img_block_data->src_y = block_ref->m_block->m_to_rect.boundary_tp;
        img_block_data->src_w = block_ref->m_block->m_to_rect.boundary_rt - block_ref->m_block->m_to_rect.boundary_lt;
        img_block_data->src_h = block_ref->m_block->m_to_rect.boundary_bm - block_ref->m_block->m_to_rect.boundary_tp;
        img_block_data->use_img = ui_ed_src_msg_update(ui_ed_obj_src(module_obj), img_block_data->use_img, path);
    }
    else {
        img_block_data->src_x = 0;
        img_block_data->src_y = 0;
        img_block_data->src_w = 0;
        img_block_data->src_h = 0;
        img_block_data->use_img = 0;
    }
}

static ui_ed_obj_t plugin_pack_packer_find_obj_by_name(ui_ed_obj_t module_obj, const char * name) {
    struct ui_ed_obj_it img_block_obj_it;
    ui_ed_obj_t img_block_obj;
    
    ui_ed_obj_childs(&img_block_obj_it, module_obj);
    while((img_block_obj = ui_ed_obj_it_next(&img_block_obj_it))) {
        UI_IMG_BLOCK * img_block_data = ui_ed_obj_data(img_block_obj);
        if (strcmp(ui_ed_src_msg(ui_ed_obj_src(module_obj), img_block_data->name_id), name) == 0) return img_block_obj;
    }

    return NULL;
}

static uint8_t plugin_pack_packer_is_block_in_language(plugin_pack_packer_t packer, ui_data_src_t src, const char * name) {
    plugin_pack_module_t module = packer->m_module;
    plugin_pack_language_t language;

    TAILQ_FOREACH(language, &packer->m_languages, m_next) {
        ui_data_src_t language_src;
        ui_ed_src_t language_module_src;
        ui_ed_obj_t language_module_obj;

        language_src = ui_data_language_find_src(language->m_data_language, src);
        if (language_src == NULL) continue;

        language_module_src = ui_ed_src_find_by_data(module->m_ed_mgr, language_src);
        if (language_module_src == NULL) continue;

        language_module_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(language_module_src));
        assert(language_module_obj);

        if (plugin_pack_packer_find_obj_by_name(language_module_obj, name)) return 1;
    }

    return 0; 
}
