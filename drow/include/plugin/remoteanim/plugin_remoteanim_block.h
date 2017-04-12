#ifndef UI_PLUGIN_REMOTEANIM_BLOCK_H
#define UI_PLUGIN_REMOTEANIM_BLOCK_H
#include "plugin_remoteanim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_remoteanim_block_t
plugin_remoteanim_block_create(plugin_remoteanim_group_t group, const char * block_name);
void plugin_remoteanim_block_free(plugin_remoteanim_block_t block);

plugin_remoteanim_block_t
plugin_remoteanim_block_find(plugin_remoteanim_group_t group, const char * name);

plugin_remoteanim_block_state_t plugin_remoteanim_block_state(plugin_remoteanim_block_t block);

int plugin_remoteanim_block_start(plugin_remoteanim_block_t block, const char * url);

#ifdef __cplusplus
}
#endif

#endif

