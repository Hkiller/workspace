#ifndef DROW_PLUGIN_REMOTEANIM_TYPES_H
#define DROW_PLUGIN_REMOTEANIM_TYPES_H
#include "cpe/pal/pal_types.h"
#include "gd/net_trans/net_trans_types.h"
#include "render/runtime/ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_remoteanim_block_state {
    plugin_remoteanim_block_state_init,
    plugin_remoteanim_block_state_downloading,
    plugin_remoteanim_block_state_installed,
    plugin_remoteanim_block_state_fail,
} plugin_remoteanim_block_state_t;
    
typedef struct plugin_remoteanim_module * plugin_remoteanim_module_t;
typedef struct plugin_remoteanim_obj * plugin_remoteanim_obj_t;
typedef struct plugin_remoteanim_group * plugin_remoteanim_group_t;
typedef struct plugin_remoteanim_block * plugin_remoteanim_block_t;

#ifdef __cplusplus
}
#endif

#endif
