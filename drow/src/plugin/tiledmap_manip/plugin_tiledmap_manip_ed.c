#include <assert.h>
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin_tiledmap_manip_i.h"

ui_ed_obj_t plugin_tiledmap_data_layer_ed_obj_create(ui_ed_obj_t parent) {
    plugin_tiledmap_data_scene_t scene;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_tiledmap_data_layer_t layer;
    ui_ed_obj_t obj;

    scene = ui_ed_obj_product(parent);
    assert(scene);

    layer = plugin_tiledmap_data_layer_create(scene);
    if (layer == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_tiledmap_layer,
        layer, plugin_tiledmap_data_layer_data(layer), sizeof(*plugin_tiledmap_data_layer_data(layer)));
    if (obj == NULL) {
        plugin_tiledmap_data_layer_free(layer);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_tiledmap_data_tile_ed_obj_create(ui_ed_obj_t parent) {
    plugin_tiledmap_data_layer_t layer;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_tiledmap_data_tile_t tile;
    ui_ed_obj_t obj;

    layer = ui_ed_obj_product(parent);
    assert(layer);

    tile = plugin_tiledmap_data_tile_create(layer);
    if (tile == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_tiledmap_tile,
        tile, plugin_tiledmap_data_tile_data(tile), sizeof(*plugin_tiledmap_data_tile_data(tile)));
    if (obj == NULL) {
        plugin_tiledmap_data_tile_free(tile);
        return NULL;
    }

    return obj;
}

int plugin_tiledmap_scene_ed_src_load(ui_ed_src_t src) {
    ui_ed_obj_t src_obj = ui_ed_src_root_obj(src);
    plugin_tiledmap_data_scene_t scene = ui_ed_src_product(src);
    ui_ed_obj_t scene_obj;
    struct plugin_tiledmap_data_layer_it layers_it;
    plugin_tiledmap_data_layer_t layer;
    ui_ed_obj_t layer_obj;
    struct plugin_tiledmap_data_tile_it tiles_it;
    plugin_tiledmap_data_tile_t tile;
    ui_ed_obj_t tile_obj;

    assert(scene);

    scene_obj = ui_ed_obj_create_i(
            src, src_obj,
            ui_ed_obj_type_tiledmap_scene,
            scene, plugin_tiledmap_data_scene_data(scene), sizeof(*plugin_tiledmap_data_scene_data(scene)));
    if (scene_obj == NULL) {
        return -1;
    }

    plugin_tiledmap_data_scene_layers(&layers_it, scene);
    while((layer = plugin_tiledmap_data_layer_it_next(&layers_it))) {
        layer_obj = ui_ed_obj_create_i(
            src, scene_obj,
            ui_ed_obj_type_tiledmap_layer,
            layer, plugin_tiledmap_data_layer_data(layer), sizeof(*plugin_tiledmap_data_layer_data(layer)));
        if (layer_obj == NULL) {
            return -1;
        }

        plugin_tiledmap_data_layer_tiles(&tiles_it, layer);
        while((tile = plugin_tiledmap_data_tile_it_next(&tiles_it))) {
            tile_obj = ui_ed_obj_create_i(
                src, layer_obj,
                ui_ed_obj_type_tiledmap_tile,
                tile, plugin_tiledmap_data_tile_data(tile), sizeof(*plugin_tiledmap_data_tile_data(tile)));
            if (tile_obj == NULL) {
                return -1;
            }
        }
    }

    return 0;
}
