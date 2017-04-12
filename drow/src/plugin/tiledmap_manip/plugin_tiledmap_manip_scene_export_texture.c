#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "plugin_tiledmap_manip_scene_export_texture_i.h"

plugin_tiledmap_scene_export_texture_t
plugin_tiledmap_scene_export_texture_create(plugin_tiledmap_scene_export_ctx_t ctx, ui_data_src_t module_src) {
    plugin_tiledmap_scene_export_texture_t texture;
    uint32_t src_id = ui_data_src_id(module_src);
    ui_cache_pixel_buf_t buf;
    char path_buf[128];

    buf = ui_cache_pixel_buf_create(ctx->m_cache_mgr);
    if (buf == NULL) {
        CPE_ERROR(ctx->m_em, "create pix buf fail!");
        return NULL;
    }

    assert(0); //TODO: Loki
    
    snprintf(
        path_buf, sizeof(path_buf), "%s/%s",
        ui_data_src_data(ui_data_mgr_src_root(ctx->m_data_mgr)),
        "TODO");
    //((UI_aMODULE const *)ui_data_module_data(module))->use_img);

    if (ui_cache_pixel_buf_load_from_file(buf, path_buf, ctx->m_em, ctx->m_alloc) != 0) {
        CPE_ERROR(ctx->m_em, "load module "FMT_UINT32_T" data buf from %s fail!", src_id, path_buf);
        ui_cache_pixel_buf_free(buf);
        return NULL;
    }

    texture = mem_alloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_scene_export_texture));
    if (texture == NULL) {
        CPE_ERROR(ctx->m_em, "load module "FMT_UINT32_T" data buf from %s: alloc module texture fail!", src_id, path_buf);
        ui_cache_pixel_buf_free(buf);
        return NULL;
    }

    texture->m_ctx = ctx;
    texture->m_src_id = src_id;
    texture->m_pixel_buf = buf;
    TAILQ_INSERT_TAIL(&ctx->m_textures, texture, m_next);

    return texture;
}

void plugin_tiledmap_scene_export_texutre_free(plugin_tiledmap_scene_export_texture_t texture) {
    plugin_tiledmap_scene_export_ctx_t ctx = texture->m_ctx;

    ui_cache_pixel_buf_free(texture->m_pixel_buf);
    TAILQ_REMOVE(&ctx->m_textures, texture, m_next);
    mem_free(ctx->m_alloc, texture);
}

plugin_tiledmap_scene_export_texture_t
plugin_tiledmap_scene_export_texture_find(plugin_tiledmap_scene_export_ctx_t ctx, uint32_t src_id) {
    plugin_tiledmap_scene_export_texture_t texture;

    TAILQ_FOREACH(texture, &ctx->m_textures, m_next) {
        if (texture->m_src_id == src_id) return texture;
    }

    return NULL;
}

ui_cache_pixel_buf_t
plugin_tiledmap_scene_export_load_pixel_buf(plugin_tiledmap_scene_export_ctx_t ctx, ui_data_src_t module_src) {
    plugin_tiledmap_scene_export_texture_t texture;
    
    texture = plugin_tiledmap_scene_export_texture_find(ctx, ui_data_src_id(module_src));
    if (texture == NULL) {
        texture = plugin_tiledmap_scene_export_texture_create(ctx, module_src);
        if (texture == NULL) return NULL;
    }

    return texture->m_pixel_buf;
}
