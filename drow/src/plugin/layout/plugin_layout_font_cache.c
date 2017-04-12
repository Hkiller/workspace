#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/utils/binpack.h"
#include "render/utils/ui_rect.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin_layout_font_cache_i.h"
#include "plugin_layout_font_meta_i.h"

plugin_layout_font_cache_t plugin_layout_font_cache_get(plugin_layout_module_t module) {
    return module->m_font_cache;
}

int plugin_layout_font_cache_register(plugin_layout_module_t module) {
    plugin_layout_font_cache_t cache;
    
    assert(module->m_font_cache == NULL);

    cache = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_font_cache));
    if (cache == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_cache_register: alloc fail!");
        return -1;
    }

    cache->m_module = module;
    cache->m_width = 0;
    cache->m_height = 0;
    
    cache->m_texture_alloc = binpack_maxrects_ctx_create(module->m_alloc, module->m_em);
    if (cache->m_texture_alloc == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_cache_register: create bin pack fail!");
        mem_free(module->m_alloc, cache);
        return -1;
    }
    binpack_maxrects_ctx_set_span(cache->m_texture_alloc, 1);

    cache->m_texture = ui_cache_res_create(module->m_cache_mgr, ui_cache_res_type_texture);
    if (cache->m_texture == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_cache_register: create cache fail!");
        binpack_maxrects_ctx_free(cache->m_texture_alloc);
        mem_free(module->m_alloc, cache);
        return -1;
    }
    ui_cache_texture_set_need_part_update(cache->m_texture, 1);
    /*在set_size后进行安装 */
    
    module->m_font_cache = cache;

    return 0;
}

void plugin_layout_font_cache_unregister(plugin_layout_module_t module) {
    plugin_layout_font_cache_t cache = module->m_font_cache;

    assert(cache);
    
    ui_cache_res_free(cache->m_texture);
    binpack_maxrects_ctx_free(cache->m_texture_alloc);
    free(cache);
    module->m_font_cache = NULL;
}

int plugin_layout_font_cache_insert(plugin_layout_font_cache_t cache, ui_rect_t r_rect, uint32_t width, uint32_t height, void const * data, size_t data_size) {
    binpack_rect_t place_rect;

    place_rect = binpack_maxrects_ctx_insert(cache->m_texture_alloc, width, height, binpack_maxrects_best_short_side_fit, 0);
    if (place_rect == NULL) {
        CPE_ERROR(cache->m_module->m_em, "plugin_layout_font_cache_insert: cache full!!!, no left space to place, size=(%d,%d)!", width, height);
        return -1;
    }

    r_rect->lt.x = place_rect->x;
    r_rect->lt.y = place_rect->y;
    r_rect->rb.x = r_rect->lt.x + place_rect->width;
    r_rect->rb.y = r_rect->lt.y + place_rect->height;

    if (ui_cache_texture_upload(cache->m_texture, r_rect, ui_cache_pf_a8, data, data_size) != 0) {
        CPE_ERROR(cache->m_module->m_em, "plugin_layout_font_cache_insert: upload fail!");
        return -1;
    }

    return 0;
}

int plugin_layout_font_cache_set_size(plugin_layout_font_cache_t cache, uint32_t width, uint32_t height) {
    if (cpe_hash_table_count(&cache->m_module->m_font_faces) != 0) {
        CPE_ERROR(cache->m_module->m_em, "plugin_layout_font_cache_set_size: already have face created!");
        return -1;
    }

    if (!cpe_math_32_is_pow2(width) || !cpe_math_32_is_pow2(height)) {
        CPE_ERROR(cache->m_module->m_em, "plugin_layout_font_cache_set_size: size (%d,%d) error!", width, height);
        return -1;
    }

    cache->m_width = width;
    cache->m_height = height;
    
    if (binpack_maxrects_ctx_init(cache->m_texture_alloc, width, height) != 0) {
        CPE_ERROR(cache->m_module->m_em, "plugin_layout_font_cache_set_size: init alloc cache fail!");
        return -1;
    }

    ui_cache_texture_set_summary(cache->m_texture, width, height, ui_cache_pf_a8);
    ui_cache_res_load_sync(cache->m_texture, NULL);
    return 0;
}

int plugin_layout_font_cache_clear(plugin_layout_font_cache_t cache) {
    plugin_layout_module_t module = cache->m_module;
    binpack_maxrects_ctx_t new_texture_alloc;
    plugin_layout_font_meta_t font_meta;
    
    new_texture_alloc = binpack_maxrects_ctx_create(module->m_alloc, module->m_em);
    if (new_texture_alloc == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_cache_register: create bin pack fail!");
        return -1;
    }
    binpack_maxrects_ctx_set_span(new_texture_alloc, 1);

    if (binpack_maxrects_ctx_init(new_texture_alloc, cache->m_width, cache->m_height) != 0) {
        CPE_ERROR(cache->m_module->m_em, "plugin_layout_font_cache_clear: init new alloc cache fail!");
        binpack_maxrects_ctx_free(new_texture_alloc);
        return -1;
    }

    TAILQ_FOREACH(font_meta, &module->m_font_metas, m_next) {
        if (font_meta->m_on_cache_clear) {
            font_meta->m_on_cache_clear(font_meta->m_ctx, font_meta);
        }
    }

    if (cache->m_texture_alloc) binpack_maxrects_ctx_free(cache->m_texture_alloc);
    cache->m_texture_alloc = new_texture_alloc;

    CPE_ERROR(cache->m_module->m_em, "plugin_layout_font_cache_clear: init new alloc cache fail!");
    
    return 0;
}
