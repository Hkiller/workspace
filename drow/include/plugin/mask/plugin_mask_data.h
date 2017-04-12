#ifndef DROW_PLUGIN_MASK_DATA_H
#define DROW_PLUGIN_MASK_DATA_H
#include "plugin_mask_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_mask_data_block_it {
    plugin_mask_data_block_t (*next)(struct plugin_mask_data_block_it * it);
    char m_data[64];
};
    
/*data */
plugin_mask_data_t plugin_mask_data_create(plugin_mask_module_t module, ui_data_src_t src);
void plugin_mask_data_free(plugin_mask_data_t sprite);

plugin_mask_module_t plugin_mask_data_module(plugin_mask_data_t data);
plugin_mask_data_format_t plugin_mask_data_format(plugin_mask_data_t data);
int plugin_mask_data_set_format(plugin_mask_data_t data, plugin_mask_data_format_t foramt);
uint32_t plugin_mask_data_blocks_count(plugin_mask_data_t data);

/*block*/
plugin_mask_data_block_t
plugin_mask_data_block_create(
    plugin_mask_data_t data, uint32_t name,
    int32_t x, int32_t y,
    uint32_t width, uint32_t heigh,
    uint32_t buf_x, uint32_t buf_y, uint32_t buf_width, uint32_t buf_height);
void plugin_mask_data_block_free(plugin_mask_data_block_t block);

plugin_mask_data_block_t
plugin_mask_data_block_find(plugin_mask_data_t data, const char * name);
    
const char * plugin_mask_data_block_name(plugin_mask_data_block_t block);
    
int32_t plugin_mask_data_block_x(plugin_mask_data_block_t block);
int32_t plugin_mask_data_block_y(plugin_mask_data_block_t block);
uint32_t plugin_mask_data_block_width(plugin_mask_data_block_t block);
uint32_t plugin_mask_data_block_height(plugin_mask_data_block_t block);

void plugin_mask_data_blocks(plugin_mask_data_t data, plugin_mask_data_block_it_t block_it);
    
uint32_t plugin_mask_data_block_buf_x(plugin_mask_data_block_t block);
uint32_t plugin_mask_data_block_buf_y(plugin_mask_data_block_t block);
uint32_t plugin_mask_data_block_buf_width(plugin_mask_data_block_t block);
uint32_t plugin_mask_data_block_buf_height(plugin_mask_data_block_t block);
void * plugin_mask_data_block_buf(plugin_mask_data_block_t block);
void * plugin_mask_data_block_check_create_buf(plugin_mask_data_block_t block);
size_t plugin_mask_data_block_buf_size(plugin_mask_data_block_t block);

uint32_t plugin_mask_data_block_buf_line_size(plugin_mask_data_block_t block);

/**/    
int plugin_mask_data_format_from_str(const char * str, plugin_mask_data_format_t * format);
const char * plugin_mask_data_format_to_str(plugin_mask_data_format_t foramt);
    
/*it*/
#define plugin_mask_data_block_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
