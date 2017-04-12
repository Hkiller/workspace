#ifndef PLUGIN_PACK_LANGUAGE_I_H
#define PLUGIN_PACK_LANGUAGE_I_H
#include "plugin/pack/plugin_pack_language.h"
#include "plugin_pack_packer_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_pack_language {
    plugin_pack_packer_t m_packer;
    TAILQ_ENTRY(plugin_pack_language) m_next;
    ui_data_language_t m_data_language;
    ui_data_src_group_t m_input_srcs;
    ui_cache_group_t m_textures;
    plugin_pack_texture_t m_texture;
};
    
#ifdef __cplusplus
}
#endif

#endif
