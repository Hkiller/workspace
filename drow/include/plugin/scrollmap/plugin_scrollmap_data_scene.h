#ifndef PLUGIN_SCROLLMAP_DATA_H
#define PLUGIN_SCROLLMAP_DATA_H
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream.h"
#include "cpe/vfs/vfs_types.h"
#include "render/model/ui_model_types.h"
#include "render/cache/ui_cache_types.h"
#include "plugin_scrollmap_types.h"
#include "protocol/plugin/scrollmap/scrollmap_data.h"
#include "protocol/plugin/scrollmap/scrollmap_script.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_data_tile_it {
    plugin_scrollmap_data_tile_t (*next)(struct plugin_scrollmap_data_tile_it * it);
    char m_data[64];
};

struct plugin_scrollmap_data_layer_it {
    plugin_scrollmap_data_layer_t (*next)(struct plugin_scrollmap_data_layer_it * it);
    char m_data[64];
};

struct plugin_scrollmap_data_block_it {
    plugin_scrollmap_data_block_t (*next)(struct plugin_scrollmap_data_block_it * it);
    char m_data[64];
};

struct plugin_scrollmap_data_script_it {
    plugin_scrollmap_data_script_t (*next)(struct plugin_scrollmap_data_script_it * it);
    char m_data[64];
};

/*scene */
plugin_scrollmap_data_scene_t
plugin_scrollmap_data_scene_create(plugin_scrollmap_module_t module, ui_data_src_t src);
void plugin_scrollmap_data_scene_free(plugin_scrollmap_data_scene_t env_data);
void plugin_scrollmap_data_scene_print(write_stream_t s, int dent, plugin_scrollmap_data_scene_t env_data);
const char * plugin_scrollmap_data_scene_dump(mem_buffer_t buffer, plugin_scrollmap_data_scene_t env_data);

/*tile */
uint32_t plugin_scrollmap_data_scene_tile_count(plugin_scrollmap_data_scene_t scene);
plugin_scrollmap_data_tile_t plugin_scrollmap_data_tile_create(plugin_scrollmap_data_scene_t scene);
void plugin_scrollmap_data_tile_free(plugin_scrollmap_data_tile_t tile);
plugin_scrollmap_data_tile_t plugin_scrollmap_data_tile_find(plugin_scrollmap_data_scene_t scene, uint16_t title_id);
SCROLLMAP_TILE * plugin_scrollmap_data_tile_data(plugin_scrollmap_data_tile_t tile);
void plugin_scrollmap_data_scene_tiles(plugin_scrollmap_data_scene_t scene, plugin_scrollmap_data_tile_it_t tile_it);
    
/*layer */
plugin_scrollmap_data_layer_t plugin_scrollmap_data_layer_create(plugin_scrollmap_data_scene_t scene);
void plugin_scrollmap_data_layer_free(plugin_scrollmap_data_layer_t layer);
plugin_scrollmap_data_layer_t plugin_scrollmap_data_layer_find(plugin_scrollmap_data_scene_t scene, const char * name);
SCROLLMAP_LAYER * plugin_scrollmap_data_layer_data(plugin_scrollmap_data_layer_t layer);
void plugin_scrollmap_data_scene_layers(plugin_scrollmap_data_scene_t scene, plugin_scrollmap_data_layer_it_t layer_it);

/*block */    
plugin_scrollmap_data_block_t plugin_scrollmap_data_block_create(plugin_scrollmap_data_layer_t layer);
void plugin_scrollmap_data_block_free(plugin_scrollmap_data_block_t block);    
SCROLLMAP_BLOCK * plugin_scrollmap_data_block_data(plugin_scrollmap_data_block_t block);
void plugin_scrollmap_data_layer_blocks(plugin_scrollmap_data_layer_t layer, plugin_scrollmap_data_block_it_t block_it);
void plugin_scrollmap_data_layer_sort_blocks(plugin_scrollmap_data_layer_t layer);

/*script*/    
plugin_scrollmap_data_script_t plugin_scrollmap_data_script_create(plugin_scrollmap_data_layer_t layer);
void plugin_scrollmap_data_script_free(plugin_scrollmap_data_script_t script);    
SCROLLMAP_SCRIPT * plugin_scrollmap_data_script_data(plugin_scrollmap_data_script_t script);
void plugin_scrollmap_data_layer_scripts(plugin_scrollmap_data_layer_t layer, plugin_scrollmap_data_script_it_t script_it);
void plugin_scrollmap_data_layer_sort_scripts(plugin_scrollmap_data_layer_t layer);

/*IT*/    
#define plugin_scrollmap_data_tile_it_next(it) ((it)->next ? (it)->next(it) : NULL)
#define plugin_scrollmap_data_layer_it_next(it) ((it)->next ? (it)->next(it) : NULL)
#define plugin_scrollmap_data_block_it_next(it) ((it)->next ? (it)->next(it) : NULL)
#define plugin_scrollmap_data_script_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
