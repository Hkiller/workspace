#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_src.h"
#include "render/model/ui_data_mgr.h"
#include "plugin_scrollmap_data_scene_i.h"
#include "plugin_scrollmap_data_tile_i.h"
#include "plugin_scrollmap_data_layer_i.h"
#include "plugin_scrollmap_data_block_i.h"
#include "plugin_scrollmap_data_script_i.h"

plugin_scrollmap_data_scene_t
plugin_scrollmap_data_scene_create(plugin_scrollmap_module_t module, ui_data_src_t src) {
    plugin_scrollmap_data_scene_t scene;

    if (ui_data_src_type(src) != ui_data_src_type_scrollmap_scene) {
        CPE_ERROR(
            module->m_em, "create scene at %s: src not scene!",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create scene at %s: product already loaded!",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return NULL;
    }

    scene = (plugin_scrollmap_data_scene_t)mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_data_scene));
    if (scene == NULL) {
        CPE_ERROR(
            module->m_em, "create scene at %s: alloc scene fail!",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return NULL;
    }

    scene->m_module = module;
    scene->m_src = src;
    scene->m_tile_count = 0;
    scene->m_layer_count = 0;
    TAILQ_INIT(&scene->m_tiles);
    TAILQ_INIT(&scene->m_layers);
    
    ui_data_src_set_product(src, scene);
    return scene;
}

void plugin_scrollmap_data_scene_free(plugin_scrollmap_data_scene_t scene) {
    plugin_scrollmap_module_t module = scene->m_module;
    
    while(!TAILQ_EMPTY(&scene->m_layers)) {
        plugin_scrollmap_data_layer_free(TAILQ_FIRST(&scene->m_layers));
    }
    assert(scene->m_layer_count == 0);

    while(!TAILQ_EMPTY(&scene->m_tiles)) {
        plugin_scrollmap_data_tile_free(TAILQ_FIRST(&scene->m_tiles));
    }
    assert(scene->m_tile_count == 0);

    mem_free(module->m_alloc, scene);
}

uint32_t plugin_scrollmap_data_scene_tile_count(plugin_scrollmap_data_scene_t scene) {
    return scene->m_tile_count;
}

void plugin_scrollmap_data_print(write_stream_t s, int dent, plugin_scrollmap_data_scene_t scene) {
    plugin_scrollmap_data_tile_t tile;
    plugin_scrollmap_data_layer_t layer;
    
    stream_putc_count(s, ' ', dent);
    stream_printf(s, "tiles\n");
    TAILQ_FOREACH(tile, &scene->m_tiles, m_next) {
        stream_putc_count(s, ' ', dent + 4);
        dr_json_print(s, &tile->m_data, sizeof(tile->m_data), scene->m_module->m_meta_tile, DR_JSON_PRINT_MINIMIZE, NULL);
        stream_printf(s, "\n");
    }

    stream_putc_count(s, ' ', dent);
    stream_printf(s, "layers\n");
    TAILQ_FOREACH(layer, &scene->m_layers, m_next) {
        plugin_scrollmap_data_block_t block;
        plugin_scrollmap_data_script_t script;
        
        stream_putc_count(s, ' ', dent + 4);
        dr_json_print(s, &layer->m_data, sizeof(layer->m_data), scene->m_module->m_meta_layer, DR_JSON_PRINT_MINIMIZE, NULL);
        stream_printf(s, "\n");

        stream_putc_count(s, ' ', dent + 4);
        stream_printf(s, "blocks\n");
        TAILQ_FOREACH(block, &layer->m_blocks, m_next) {
            stream_putc_count(s, ' ', dent + 8);
            dr_json_print(s, &block->m_data, sizeof(block->m_data), scene->m_module->m_meta_block, DR_JSON_PRINT_MINIMIZE, NULL);
            stream_printf(s, "\n");
        }

        stream_putc_count(s, ' ', dent + 4);
        stream_printf(s, "scripts\n");
        TAILQ_FOREACH(script, &layer->m_scripts, m_next) {
            stream_putc_count(s, ' ', dent + 8);
            dr_json_print(s, &script->m_data, sizeof(script->m_data), scene->m_module->m_meta_script, DR_JSON_PRINT_MINIMIZE, NULL);
            stream_printf(s, "\n");
        }
    }
}

const char * plugin_scrollmap_data_scene_dump(mem_buffer_t buffer, plugin_scrollmap_data_scene_t scene) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    plugin_scrollmap_data_print((write_stream_t)&stream, 0, scene);
    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

int plugin_scrollmap_data_scene_update_usings(ui_data_src_t src) {
    plugin_scrollmap_data_scene_t scene = ui_data_src_product(src);
    plugin_scrollmap_module_t module = scene->m_module;
    plugin_scrollmap_data_tile_t tile;
    int rv = 0;

    TAILQ_FOREACH(tile, &scene->m_tiles, m_next) {
        uint32_t src_id;
        ui_data_src_type_t src_type;
        ui_data_src_t use_src;

        switch(tile->m_data.res_type) {
        case scrollmap_tile_res_type_tag:
            continue;
        case scrollmap_tile_res_type_module:
            src_id = tile->m_data.src_id;
            src_type = ui_data_src_type_module;
            break;
        case scrollmap_tile_res_type_sprite:
            src_id = tile->m_data.src_id;
            src_type = ui_data_src_type_sprite;
            break;
        default:
            CPE_ERROR(
                module->m_em, "%s: update usings: tile ref type %d is unknown!",
                ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src),
                tile->m_data.res_type);
            rv = -1;
            continue;
            break;
        }
    
        use_src = ui_data_src_find_by_id(module->m_data_mgr, tile->m_data.src_id);
        if (use_src == NULL) {
            CPE_ERROR(
                module->m_em, "%s: update usings: tile ref src "FMT_UINT32_T" not exist!",
                ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src),
                tile->m_data.src_id);
            rv = -1;
            continue;
        }

        if (ui_data_src_src_create(src, use_src) == NULL) {
            CPE_ERROR(
                module->m_em, "%s: update usings: create src-src fail!",
                ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
            rv = -1;
            continue;
        }
    }

    return rv;
}

int plugin_scrollmap_data_scene_regist(plugin_scrollmap_module_t module) {
    if (ui_data_mgr_register_type(
            module->m_data_mgr, ui_data_src_type_scrollmap_scene,
            (ui_data_product_create_fun_t)plugin_scrollmap_data_scene_create, module,
            (ui_data_product_free_fun_t)plugin_scrollmap_data_scene_free, module,
            plugin_scrollmap_data_scene_update_usings)
        != 0)
    {
        CPE_ERROR(module->m_em, "plugin_scrollmap_data_scene_regist: create: register type scrollmap fail!");
        return -1;
    }
    
    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_scrollmap_scene, plugin_scrollmap_data_scene_bin_load, module);

    ui_data_mgr_set_saver(
        module->m_data_mgr, ui_data_src_type_scrollmap_scene, plugin_scrollmap_data_scene_bin_save, plugin_scrollmap_data_scene_bin_rm, module);

    return 0;
}

void plugin_scrollmap_data_scene_unregist(plugin_scrollmap_module_t module) {
    ui_data_mgr_unregister_type(module->m_data_mgr, ui_data_src_type_scrollmap_scene);
    ui_data_mgr_set_loader(module->m_data_mgr, ui_data_src_type_scrollmap_scene, NULL, NULL);
    ui_data_mgr_set_saver(module->m_data_mgr, ui_data_src_type_scrollmap_scene, NULL, NULL, NULL);
}
