#include "cpe/pal/pal_strings.h"
#include "plugin_tiledmap_manip_scene_import_merge_i.h"

static uint32_t plugin_tiledmap_manip_import_merge_tile_hash(plugin_tiledmap_manip_import_merge_tile_t tile);
static int plugin_tiledmap_manip_import_merge_tile_eq(plugin_tiledmap_manip_import_merge_tile_t l, plugin_tiledmap_manip_import_merge_tile_t r);

/*ctx*/
int plugin_tiledmap_manip_import_merge_ctx_init(
    plugin_tiledmap_manip_import_merge_ctx_t merge_ctx, plugin_tiledmap_manip_import_ctx_t import_ctx)
{
    bzero(merge_ctx, sizeof(*merge_ctx));
    merge_ctx->m_import_ctx = import_ctx;

    merge_ctx->m_tmp_pixel_buf = ui_cache_pixel_buf_create(import_ctx->m_cache_mgr);
    if (merge_ctx->m_tmp_pixel_buf == NULL) {
        return -1;
    }

    if (cpe_hash_table_init(
            &merge_ctx->m_tiles,
            import_ctx->m_alloc,
            (cpe_hash_fun_t) plugin_tiledmap_manip_import_merge_tile_hash,
            (cpe_hash_eq_t) plugin_tiledmap_manip_import_merge_tile_eq,
            CPE_HASH_OBJ2ENTRY(plugin_tiledmap_manip_import_merge_tile, m_hh),
            -1) != 0)
    {
        ui_cache_pixel_buf_free(merge_ctx->m_tmp_pixel_buf);
        return -1;
    }

    return 0;
}

void plugin_tiledmap_manip_import_merge_ctx_fini(plugin_tiledmap_manip_import_merge_ctx_t merge_ctx) {
    struct cpe_hash_it tile_it;
    plugin_tiledmap_manip_import_merge_tile_t tile;

    cpe_hash_it_init(&tile_it, &merge_ctx->m_tiles);

    tile = cpe_hash_it_next(&tile_it);
    while (tile) {
        plugin_tiledmap_manip_import_merge_tile_t next = cpe_hash_it_next(&tile_it);
        plugin_tiledmap_manip_import_merge_tile_free(tile);
        tile = next;
    }
    cpe_hash_table_fini(&merge_ctx->m_tiles);

    ui_cache_pixel_buf_free(merge_ctx->m_tmp_pixel_buf);
}

/*tile*/
plugin_tiledmap_manip_import_merge_tile_t
plugin_tiledmap_manip_import_merge_tile_create(
    plugin_tiledmap_manip_import_merge_ctx_t merge_ctx, cpe_md5_value_t md5, plugin_tiledmap_manip_import_tile_t tile)
{
    plugin_tiledmap_manip_import_merge_tile_t merge_tile;

    merge_tile = mem_alloc(merge_ctx->m_import_ctx->m_alloc, sizeof(struct plugin_tiledmap_manip_import_merge_tile));
    if (merge_tile == NULL) return NULL;

    merge_tile->m_ctx = merge_ctx;
    merge_tile->m_tile = tile;
    merge_tile->m_md5 = *md5;

    cpe_hash_entry_init(&merge_tile->m_hh);
    if (cpe_hash_table_insert_unique(&merge_ctx->m_tiles, merge_tile) != 0) {
        CPE_ERROR(merge_ctx->m_import_ctx->m_em, "create merge tile: insert fail!");
        mem_free(merge_ctx->m_import_ctx->m_alloc, merge_tile);
        return NULL;
    }

    return merge_tile;
}

plugin_tiledmap_manip_import_merge_tile_t
plugin_tiledmap_manip_import_merge_tile_find(plugin_tiledmap_manip_import_merge_ctx_t merge_ctx, cpe_md5_value_t md5) {
    struct plugin_tiledmap_manip_import_merge_tile key;
    key.m_md5 = *md5;
    return cpe_hash_table_find(&merge_ctx->m_tiles, &key);
}

void plugin_tiledmap_manip_import_merge_tile_free(plugin_tiledmap_manip_import_merge_tile_t merge_tile) {
    cpe_hash_table_remove_by_ins(&merge_tile->m_ctx->m_tiles, merge_tile);
    mem_free(merge_tile->m_ctx->m_import_ctx->m_alloc, merge_tile);
}

uint32_t plugin_tiledmap_manip_import_merge_tile_hash(plugin_tiledmap_manip_import_merge_tile_t tile) {
    return cpe_hash_md5(&tile->m_md5);
}

int plugin_tiledmap_manip_import_merge_tile_eq(plugin_tiledmap_manip_import_merge_tile_t l, plugin_tiledmap_manip_import_merge_tile_t r) {
    return cpe_md5_cmp(&l->m_md5, &r->m_md5) == 0;
}
