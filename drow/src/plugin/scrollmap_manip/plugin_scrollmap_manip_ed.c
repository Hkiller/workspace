#include <assert.h>
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/scrollmap/plugin_scrollmap_module.h"
#include "plugin/scrollmap/plugin_scrollmap_data_scene.h"
#include "plugin_scrollmap_manip_i.h"

static ui_ed_obj_t plugin_scrollmap_data_tile_ed_obj_create(ui_ed_obj_t parent) {
    plugin_scrollmap_data_scene_t scene;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_scrollmap_data_tile_t tile;
    ui_ed_obj_t obj;

    scene = ui_ed_src_product(ed_src);
    assert(scene);

    tile = plugin_scrollmap_data_tile_create(scene);
    if (tile == NULL) return NULL;
    
    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_scrollmap_tile,
        tile, plugin_scrollmap_data_tile_data(tile), sizeof(*plugin_scrollmap_data_tile_data(tile)));
    if (obj == NULL) {
        plugin_scrollmap_data_tile_free(tile);
        return NULL;
    }

    return obj;
}

static ui_ed_obj_t plugin_scrollmap_data_layer_ed_obj_create(ui_ed_obj_t parent) {
    plugin_scrollmap_data_scene_t scene;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_scrollmap_data_layer_t layer;
    ui_ed_obj_t obj;

    scene = ui_ed_src_product(ed_src);
    assert(scene);

    layer = plugin_scrollmap_data_layer_create(scene);
    if (layer == NULL) return NULL;
    
    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_scrollmap_layer,
        layer, plugin_scrollmap_data_layer_data(layer), sizeof(*plugin_scrollmap_data_layer_data(layer)));
    if (obj == NULL) {
        plugin_scrollmap_data_layer_free(layer);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_scrollmap_data_block_ed_obj_create(ui_ed_obj_t parent) {
    plugin_scrollmap_data_layer_t layer;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_scrollmap_data_block_t block;
    ui_ed_obj_t obj;

    layer = ui_ed_obj_product(parent);
    assert(layer);

    block = plugin_scrollmap_data_block_create(layer);
    if (block == NULL) return NULL;
    
    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_scrollmap_block,
        block, plugin_scrollmap_data_block_data(block), sizeof(*plugin_scrollmap_data_block_data(block)));
    if (obj == NULL) {
        plugin_scrollmap_data_block_free(block);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_scrollmap_data_script_ed_obj_create(ui_ed_obj_t parent) {
    plugin_scrollmap_data_layer_t layer;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_scrollmap_data_script_t script;
    ui_ed_obj_t obj;

    layer = ui_ed_obj_product(parent);
    assert(layer);

    script = plugin_scrollmap_data_script_create(layer);
    if (script == NULL) return NULL;
    
    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_scrollmap_script,
        script, plugin_scrollmap_data_script_data(script), sizeof(*plugin_scrollmap_data_script_data(script)));
    if (obj == NULL) {
        plugin_scrollmap_data_script_free(script);
        return NULL;
    }

    return obj;
}

int plugin_scrollmap_ed_src_load(ui_ed_src_t src) {
    ui_ed_obj_t src_obj = ui_ed_src_root_obj(src);
    plugin_scrollmap_data_scene_t scene = ui_ed_src_product(src);
    struct plugin_scrollmap_data_tile_it tile_it;
    plugin_scrollmap_data_tile_t tile;
    struct plugin_scrollmap_data_layer_it layer_it;
    plugin_scrollmap_data_layer_t layer;

    assert(scene);

    /*加载tile*/
    plugin_scrollmap_data_scene_tiles(scene, &tile_it);
    while((tile = plugin_scrollmap_data_tile_it_next(&tile_it))) {
        ui_ed_obj_t obj_tile;
        
        obj_tile = ui_ed_obj_create_i(
            src, src_obj,
            ui_ed_obj_type_scrollmap_tile,
            tile, plugin_scrollmap_data_tile_data(tile), sizeof(*plugin_scrollmap_data_tile_data(tile)));
        if (obj_tile == NULL) {
            return -1;
        }
    }

    /*加载layer*/
    plugin_scrollmap_data_scene_layers(scene, &layer_it);
    while((layer = plugin_scrollmap_data_layer_it_next(&layer_it))) {
        ui_ed_obj_t obj_layer;
        struct plugin_scrollmap_data_block_it block_it;
        plugin_scrollmap_data_block_t block;
        struct plugin_scrollmap_data_script_it script_it;
        plugin_scrollmap_data_script_t script;

        obj_layer = ui_ed_obj_create_i(
            src, src_obj,
            ui_ed_obj_type_scrollmap_layer,
            layer, plugin_scrollmap_data_layer_data(layer), sizeof(*plugin_scrollmap_data_layer_data(layer)));
        if (obj_layer == NULL) {
            return -1;
        }

        plugin_scrollmap_data_layer_blocks(layer, &block_it);
        while((block = plugin_scrollmap_data_block_it_next(&block_it))) {
            ui_ed_obj_t obj_block;

            obj_block = ui_ed_obj_create_i(
                src, obj_layer,
                ui_ed_obj_type_scrollmap_block,
                block, plugin_scrollmap_data_block_data(block), sizeof(*plugin_scrollmap_data_block_data(block)));
            if (obj_block == NULL) {
                return -1;
            }
        }

        plugin_scrollmap_data_layer_scripts(layer, &script_it);
        while((script = plugin_scrollmap_data_script_it_next(&script_it))) {
            ui_ed_obj_t obj_script;

            obj_script = ui_ed_obj_create_i(
                src, obj_layer,
                ui_ed_obj_type_scrollmap_script,
                script, plugin_scrollmap_data_script_data(script), sizeof(*plugin_scrollmap_data_script_data(script)));
            if (obj_script == NULL) {
                return -1;
            }
        }
    }
    
    return 0;
}

int plugin_scrollmap_manip_ed_regist(plugin_scrollmap_manip_t module) {
    if (module->m_ed_mgr == NULL) return 0;
    
    if (ui_ed_mgr_register_src_type(module->m_ed_mgr, ui_data_src_type_scrollmap_scene, plugin_scrollmap_ed_src_load) != 0) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_manip: create: register type scrollmap fail!");
        return -1;
    }

    /*tile*/
    if (ui_ed_mgr_register_obj_type(
            module->m_ed_mgr, ui_data_src_type_scrollmap_scene,
            ui_ed_obj_type_scrollmap_tile, plugin_scrollmap_module_tile_meta(module->m_scrollmap_module),
            (ui_ed_obj_delete_fun_t)plugin_scrollmap_data_block_free,
            NULL, NULL) != 0
        || ui_ed_mgr_register_obj_child(
            module->m_ed_mgr, ui_ed_obj_type_src, ui_ed_obj_type_scrollmap_tile, plugin_scrollmap_data_tile_ed_obj_create) != 0
        )
    {
        CPE_ERROR(module->m_em, "plugin_scrollmap_manip: create: register obj type fail!");
        goto CLEAR_TYPES;
    }

    /*layer*/
    if (ui_ed_mgr_register_obj_type(
            module->m_ed_mgr, ui_data_src_type_scrollmap_scene,
            ui_ed_obj_type_scrollmap_layer, plugin_scrollmap_module_layer_meta(module->m_scrollmap_module),
            (ui_ed_obj_delete_fun_t)plugin_scrollmap_data_block_free,
            NULL, NULL) != 0
        || ui_ed_mgr_register_obj_child(
            module->m_ed_mgr, ui_ed_obj_type_src, ui_ed_obj_type_scrollmap_layer, plugin_scrollmap_data_layer_ed_obj_create) != 0
        )
    {
        CPE_ERROR(module->m_em, "plugin_scrollmap_manip: create: register obj type fail!");
        goto CLEAR_TYPES;
    }

    /*block*/
    if (ui_ed_mgr_register_obj_type(
            module->m_ed_mgr, ui_data_src_type_scrollmap_scene,
            ui_ed_obj_type_scrollmap_block, plugin_scrollmap_module_block_meta(module->m_scrollmap_module),
            (ui_ed_obj_delete_fun_t)plugin_scrollmap_data_block_free,
            NULL, NULL) != 0
        || ui_ed_mgr_register_obj_child(
            module->m_ed_mgr, ui_ed_obj_type_scrollmap_layer, ui_ed_obj_type_scrollmap_block, plugin_scrollmap_data_block_ed_obj_create) != 0
        )
    {
        CPE_ERROR(module->m_em, "plugin_scrollmap_manip: create: register obj type fail!");
        goto CLEAR_TYPES;
    }

    /*script*/
    if (ui_ed_mgr_register_obj_type(
            module->m_ed_mgr, ui_data_src_type_scrollmap_scene,
            ui_ed_obj_type_scrollmap_script, plugin_scrollmap_module_script_meta(module->m_scrollmap_module),
            (ui_ed_obj_delete_fun_t)plugin_scrollmap_data_script_free,
            NULL, NULL) != 0
        || ui_ed_mgr_register_obj_child(
            module->m_ed_mgr, ui_ed_obj_type_scrollmap_layer, ui_ed_obj_type_scrollmap_script, plugin_scrollmap_data_script_ed_obj_create) != 0
        )
    {
        CPE_ERROR(module->m_em, "plugin_scrollmap_manip: create: register obj type fail!");
        goto CLEAR_TYPES;
    }
    
    return 0;

CLEAR_TYPES:
    ui_ed_mgr_unregister_src_type(module->m_ed_mgr, ui_data_src_type_scrollmap_scene);
    ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_scrollmap_tile);
    ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_scrollmap_layer);
    ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_scrollmap_block);
    ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_scrollmap_script);
    return -1;
}

void plugin_scrollmap_manip_ed_unregist(plugin_scrollmap_manip_t module) {
    ui_ed_mgr_unregister_src_type(module->m_ed_mgr, ui_data_src_type_scrollmap_scene);
    ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_scrollmap_tile);
    ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_scrollmap_layer);
    ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_scrollmap_block);
    ui_ed_mgr_unregister_obj_type(module->m_ed_mgr, ui_ed_obj_type_scrollmap_script);
}
