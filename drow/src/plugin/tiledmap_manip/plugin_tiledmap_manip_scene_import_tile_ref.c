#include <assert.h>
#include "plugin_tiledmap_manip_scene_import_tile_ref_i.h"

plugin_tiledmap_manip_import_tile_ref_t
plugin_tiledmap_manip_import_tile_ref_create(
    plugin_tiledmap_manip_import_tile_ref_list_t * owner,
    plugin_tiledmap_manip_import_tile_t tile, uint32_t x, uint32_t y)
{
    plugin_tiledmap_manip_import_ctx_t ctx = tile->m_ctx;
    plugin_tiledmap_manip_import_tile_ref_t tile_ref;

    tile_ref = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_manip_import_tile_ref));
    if (tile_ref == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: create tile ref: alloc fail!", ctx->m_proj_path);
        return NULL; 
    }

    tile_ref->m_owner = owner;
    TAILQ_INSERT_TAIL(owner, tile_ref, m_next_for_owner);

    tile_ref->m_tile = tile;
    TAILQ_INSERT_TAIL(&tile->m_refs, tile_ref, m_next_for_tile);
    tile->m_ref_count++;

    tile_ref->m_x = x;
    tile_ref->m_y = y;

    return tile_ref;
}

void plugin_tiledmap_manip_import_tile_ref_free(plugin_tiledmap_manip_import_tile_ref_t tile_ref) {
    plugin_tiledmap_manip_import_ctx_t ctx = tile_ref->m_tile->m_ctx;

    TAILQ_REMOVE(tile_ref->m_owner, tile_ref, m_next_for_owner);
    TAILQ_REMOVE(&tile_ref->m_tile->m_refs, tile_ref, m_next_for_tile);
    tile_ref->m_tile->m_ref_count--;

    mem_free(ctx->m_alloc, tile_ref);
}

void plugin_tiledmap_manip_import_tile_ref_set_tile(
    plugin_tiledmap_manip_import_tile_ref_t tile_ref, plugin_tiledmap_manip_import_tile_t tile)
{
    assert(tile_ref->m_tile != tile);
    
    TAILQ_REMOVE(&tile_ref->m_tile->m_refs, tile_ref, m_next_for_tile);
    tile_ref->m_tile->m_ref_count--;

    tile_ref->m_tile = tile;

    TAILQ_INSERT_TAIL(&tile->m_refs, tile_ref, m_next_for_tile);
    tile->m_ref_count++;
}

void plugin_tiledmap_manip_import_tile_ref_apply_op(plugin_tiledmap_manip_import_tile_ref_t tile_ref, uint8_t flip_type, uint8_t angle_type) {
    tile_ref->m_flip_type ^= flip_type;
    tile_ref->m_angle_type = (tile_ref->m_angle_type + angle_type) % 4;
}
