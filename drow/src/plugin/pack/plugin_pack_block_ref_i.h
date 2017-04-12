#ifndef PLUGIN_PACK_BLOCK_REF_I_H
#define PLUGIN_PACK_BLOCK_REF_I_H
#include "plugin/particle/plugin_particle_types.h"
#include "plugin_pack_block_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_pack_block_ref {
    plugin_pack_block_t m_block;
    TAILQ_ENTRY(plugin_pack_block_ref) m_next;
    ui_ed_obj_t m_ed_obj;
};

plugin_pack_block_ref_t plugin_pack_block_ref_create(plugin_pack_block_t block, ui_ed_obj_t ed_obj);
    
void plugin_pack_block_ref_free(plugin_pack_block_ref_t block_ref);

#ifdef __cplusplus
}
#endif

#endif
