#ifndef UI_MODEL_MANIP_SCENE_IMPORT_TILE_REF_H
#define UI_MODEL_MANIP_SCENE_IMPORT_TILE_REF_H
#include "plugin_tiledmap_manip_scene_import_layer_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_i.h"

struct plugin_tiledmap_manip_import_tile_ref {
    plugin_tiledmap_manip_import_tile_ref_list_t * m_owner;
    TAILQ_ENTRY(plugin_tiledmap_manip_import_tile_ref) m_next_for_owner;
    plugin_tiledmap_manip_import_tile_t m_tile;
    TAILQ_ENTRY(plugin_tiledmap_manip_import_tile_ref) m_next_for_tile;
    uint32_t m_x;
    uint32_t m_y;
    uint8_t m_flip_type;
    uint8_t m_angle_type;
};

plugin_tiledmap_manip_import_tile_ref_t
plugin_tiledmap_manip_import_tile_ref_create(
    plugin_tiledmap_manip_import_tile_ref_list_t * owner,
    plugin_tiledmap_manip_import_tile_t tile, uint32_t x, uint32_t y);

void plugin_tiledmap_manip_import_tile_ref_free(plugin_tiledmap_manip_import_tile_ref_t tile_ref);

void plugin_tiledmap_manip_import_tile_ref_set_tile(plugin_tiledmap_manip_import_tile_ref_t tile_ref, plugin_tiledmap_manip_import_tile_t tile);
void plugin_tiledmap_manip_import_tile_ref_apply_op(plugin_tiledmap_manip_import_tile_ref_t tile_ref, uint8_t flip_type, uint8_t angle_type);

#endif
