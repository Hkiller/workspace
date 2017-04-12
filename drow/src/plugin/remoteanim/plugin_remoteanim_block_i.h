#ifndef PLUGIN_REMOTEANIM_BLOCK_I_H
#define PLUGIN_REMOTEANIM_BLOCK_I_H
#include "render/utils/ui_rect.h"
#include "plugin/remoteanim/plugin_remoteanim_block.h"
#include "plugin_remoteanim_group_i.h"

struct plugin_remoteanim_block {
    plugin_remoteanim_group_t m_group;
    TAILQ_ENTRY(plugin_remoteanim_block) m_next;
    struct cpe_hash_entry m_hh;
    plugin_remoteanim_block_state_t m_state;
    net_trans_task_t m_trans_task;
    const char * m_name;
    char m_name_buf[32];
    struct ui_vector_2 m_size;
    struct ui_rect m_place;
    plugin_remoteanim_obj_list_t m_objs;
};

void plugin_remoteanim_block_real_free(plugin_remoteanim_block_t block);

uint32_t plugin_remoteanim_block_hash(plugin_remoteanim_block_t block);
int plugin_remoteanim_block_eq(plugin_remoteanim_block_t l, plugin_remoteanim_block_t r);

#endif
