#ifndef PLUGIN_REMOTEANIM_GROUP_I_H
#define PLUGIN_REMOTEANIM_GROUP_I_H
#include "render/utils/ui_vector_2.h"
#include "plugin/remoteanim/plugin_remoteanim_group.h"
#include "plugin_remoteanim_module_i.h"

struct plugin_remoteanim_group {
    plugin_remoteanim_module_t m_module;
    struct cpe_hash_entry m_hh;
    const char * m_name;
    uint8_t m_auto_free;
    net_trans_group_t m_trans_group;
    ui_vector_2 m_capacity;
    ui_cache_res_t m_texture;
    binpack_maxrects_ctx_t m_texture_alloc;
    plugin_remoteanim_block_list_t m_blocks;
};

uint32_t plugin_remoteanim_group_hash(plugin_remoteanim_group_t group);
int plugin_remoteanim_group_eq(plugin_remoteanim_group_t l, plugin_remoteanim_group_t r);

#endif
