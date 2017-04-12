#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_language.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_pack_packer_i.h"
#include "plugin_pack_language_i.h"
#include "plugin_pack_texture_i.h"
#include "plugin_pack_block_i.h"

plugin_pack_packer_t plugin_pack_packer_create(plugin_pack_module_t module, const char * target_path) {
    plugin_pack_packer_t packer;

    packer = mem_alloc(module->m_alloc, sizeof(struct plugin_pack_packer));
    if (packer == NULL) {
        CPE_ERROR(module->m_em, "plugin_pack_packer_create: create fail!");
        return NULL;
    }

    packer->m_module = module;
    packer->m_input_srcs = ui_data_src_group_create(module->m_data_mgr);
    packer->m_packed_srcs = ui_data_src_group_create(module->m_data_mgr);
    packer->m_pack_textures = ui_cache_group_create(module->m_cache_mgr);
    packer->m_generated_textures = ui_cache_group_create(module->m_cache_mgr);
    packer->m_texture_span = 4;
    packer->m_limit_width = 0;
    packer->m_limit_height = 0;
    packer->m_default_language_name[0] = 0;
    packer->m_default_language = NULL;
    packer->m_language_count = 0;
    TAILQ_INIT(&packer->m_languages);
    TAILQ_INIT(&packer->m_textures);

    packer->m_common_texture = plugin_pack_texture_create(packer, target_path);

    if (packer->m_input_srcs == NULL
        || packer->m_packed_srcs == NULL
        || packer->m_pack_textures == NULL
        || packer->m_generated_textures == NULL
        || packer->m_common_texture == NULL)
    {
        if (packer->m_input_srcs) ui_data_src_group_free(packer->m_input_srcs);
        if (packer->m_packed_srcs) ui_data_src_group_free(packer->m_packed_srcs);
        if (packer->m_pack_textures) ui_cache_group_free(packer->m_pack_textures);
        mem_free(module->m_alloc, packer);
        return NULL;
    }

    return packer;
}

void plugin_pack_packer_free(plugin_pack_packer_t packer) {
    plugin_pack_module_t module = packer->m_module;

    while(!TAILQ_EMPTY(&packer->m_languages)) {
        plugin_pack_language_free(TAILQ_FIRST(&packer->m_languages));
    }
    assert(packer->m_language_count == 0);
    assert(packer->m_default_language == NULL);

    plugin_pack_texture_free(packer->m_common_texture);
    ui_data_src_group_free(packer->m_input_srcs);
    ui_data_src_group_free(packer->m_packed_srcs);
    ui_cache_group_free(packer->m_pack_textures);
    
    mem_free(module->m_alloc, packer);
}

uint8_t plugin_pack_packer_texture_span(plugin_pack_packer_t packer) {
    return packer->m_texture_span;
}

void plugin_pack_packer_texture_set_span(plugin_pack_packer_t packer, uint8_t span) {
    packer->m_texture_span = span;
}

uint32_t plugin_pack_packer_texture_limit_width(plugin_pack_packer_t packer) {
    return packer->m_limit_width;
}

void plugin_pack_packer_texture_set_limit_width(plugin_pack_packer_t packer, uint32_t limit_width) {
    packer->m_limit_width = limit_width;
}

uint32_t plugin_pack_packer_texture_limit_height(plugin_pack_packer_t packer) {
    return packer->m_limit_height;
}

void plugin_pack_packer_texture_set_limit_height(plugin_pack_packer_t packer, uint32_t limit_height) {
    packer->m_limit_height = limit_height;
}

const char * plugin_pack_packer_default_language(plugin_pack_packer_t packer) {
    return packer->m_default_language_name;
}

void plugin_pack_packer_set_default_language(plugin_pack_packer_t packer, const char * language) {
    plugin_pack_language_t pack_language;
    
    cpe_str_dup(packer->m_default_language_name, sizeof(packer->m_default_language_name), language);
    
    packer->m_default_language = NULL;
    TAILQ_FOREACH(pack_language, &packer->m_languages, m_next) {
        if (strcmp(language, ui_data_language_name(pack_language->m_data_language)) == 0) {
            packer->m_default_language = pack_language;
            break;
        }
    }
}

ui_data_src_group_t plugin_pack_packer_input_srcs(plugin_pack_packer_t packer) {
    return packer->m_input_srcs;
}

ui_data_src_group_t plugin_pack_packer_packed_srcs(plugin_pack_packer_t packer) {
    return packer->m_packed_srcs;
}

ui_cache_group_t plugin_pack_packer_pack_textures(plugin_pack_packer_t packer) {
    return packer->m_pack_textures;
}

ui_cache_group_t plugin_pack_packer_generated_textures(plugin_pack_packer_t packer) {
    return packer->m_generated_textures;
}

static int plugin_pack_packer_process_src(
    plugin_pack_module_t module,
    ui_cache_group_t not_pack_textures, plugin_pack_texture_t texture, ui_data_src_t process_src)
{
    if (!ui_data_src_is_loaded(process_src)) {
        if (ui_data_src_load(process_src, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_pack_packer_pack: load src %s fail",
                ui_data_src_path_dump(&module->m_dump_buffer, process_src));
            return -1;
        }
    }

    if (ui_data_src_group_add_src(texture->m_packer->m_packed_srcs, process_src) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_pack_packer_pack: add src %s to packed src fail",
            ui_data_src_path_dump(&texture->m_packer->m_module->m_dump_buffer, process_src));
        return -1;
    }
    
    switch(ui_data_src_type(process_src)) {
    case ui_data_src_type_module:
        if (plugin_pack_packer_load_module(texture, process_src) != 0) return -1;
        break;
    case ui_data_src_type_particle:
        if (plugin_pack_packer_load_particle(texture, process_src) != 0) return -1;
        break;
    default:
        break;
    }

    return 0;
}

int plugin_pack_packer_pack(plugin_pack_packer_t packer) {
    plugin_pack_module_t module = packer->m_module;
    plugin_pack_language_t pack_language;
    struct ui_data_src_it input_src_it;
    ui_data_src_t input_src;
    plugin_pack_texture_t texture;
    int rv = 0;

    /*先处理语言相关的资源 */
    TAILQ_FOREACH(pack_language, &packer->m_languages, m_next) {
        /*合并基础包 */
        ui_data_src_group_srcs(&input_src_it, pack_language->m_input_srcs);
        while((input_src = ui_data_src_it_next(&input_src_it))) {
            assert(ui_data_src_type(input_src) != ui_data_src_type_dir);
            if (ui_data_src_in_group(input_src, packer->m_packed_srcs)) continue;
            if (plugin_pack_packer_process_src(module, pack_language->m_textures, pack_language->m_texture, input_src) != 0) rv = -1;
        }
    }
        
    ui_data_src_group_srcs(&input_src_it, packer->m_input_srcs);
    while((input_src = ui_data_src_it_next(&input_src_it))) {
        assert(ui_data_src_type(input_src) != ui_data_src_type_dir);
        if (ui_data_src_in_group(input_src, packer->m_packed_srcs)) continue;
        if (plugin_pack_packer_process_src(module, NULL, packer->m_common_texture, input_src) != 0) rv = -1;
    }

    /*提交所有变化 */
    TAILQ_FOREACH(texture, &packer->m_textures, m_next) {
        if (plugin_pack_texture_commit(texture) != 0) rv = -1;
    }

    return rv;
}

int plugin_pack_packer_load_src_texture(
    plugin_pack_packer_t packer, ui_data_src_t src, const char * path,
    ui_cache_res_t * output_res, ui_cache_pixel_buf_t * output_buf)
{
    plugin_pack_module_t module = packer->m_module;
    ui_cache_res_t source_texture;
    ui_cache_pixel_buf_t source_texture_buf;

    source_texture = ui_cache_res_find_by_path(module->m_cache_mgr, path);
    if (source_texture == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_pack_packer_load_src_texture: %s %s using texture %s not exist!",
            ui_data_src_type_name(ui_data_src_type(src)),
            ui_data_src_path_dump(&module->m_dump_buffer, src),
            path);
        return -1;
    }
    
    if (!ui_cache_res_in_group(source_texture, packer->m_pack_textures)) {
        *output_res = NULL;
        *output_buf = NULL;
        return 0;
    }
    
    assert(source_texture);
    
    source_texture_buf = ui_cache_texture_data_buf(source_texture);
    if (source_texture_buf == NULL) {
        ui_cache_texture_set_keep_data_buf(source_texture, 1);
        if (ui_cache_res_load_sync(source_texture, NULL) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_pack_packer_load_src_texture: %s %s using texture %s load fail!",
                ui_data_src_type_name(ui_data_src_type(src)),
                ui_data_src_path_dump(&module->m_dump_buffer, src),
                path);
            return -1;
        }

        source_texture_buf = ui_cache_texture_data_buf(source_texture);
        if (source_texture_buf == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_pack_packer_load_src_texture: %s %s using texture %s load success, no data buf!",
                ui_data_src_type_name(ui_data_src_type(src)),
                ui_data_src_path_dump(&module->m_dump_buffer, src),
                path);
            return -1;
        }
        
        assert(source_texture_buf);
    }

    *output_res = source_texture;
    *output_buf = source_texture_buf;

    return 0;
}

void plugin_pack_packer_remove_pack_texture(plugin_pack_packer_t packer, ui_cache_res_t res) {
    plugin_pack_texture_t pack_texture;
    
    ui_cache_group_remove_res(packer->m_pack_textures, res);

    TAILQ_FOREACH(pack_texture, &packer->m_textures, m_next) {
        plugin_pack_block_free_by_src_texture(pack_texture, res);
    }
}
