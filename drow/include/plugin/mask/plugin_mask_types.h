#ifndef DROW_PLUGIN_MASK_TYPES_H
#define DROW_PLUGIN_MASK_TYPES_H
#include "cpe/pal/pal_types.h"
#include "render/model/ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_mask_module * plugin_mask_module_t;
typedef struct plugin_mask_data * plugin_mask_data_t;
typedef struct plugin_mask_data_block * plugin_mask_data_block_t;
typedef struct plugin_mask_data_block_it * plugin_mask_data_block_it_t;
typedef struct plugin_mask_render_obj * plugin_mask_render_obj_t;

typedef enum plugin_mask_data_format {
    plugin_mask_data_format_bit,
    plugin_mask_data_format_1,
    plugin_mask_data_format_2,
    plugin_mask_data_format_4,
} plugin_mask_data_format_t;
    
#ifdef __cplusplus
}
#endif

#endif
