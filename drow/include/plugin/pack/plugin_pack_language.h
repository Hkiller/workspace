#ifndef DROW_PLUGIN_PACK_LANGUAGE_H
#define DROW_PLUGIN_PACK_LANGUAGE_H
#include "plugin_pack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_pack_language_it {
    plugin_pack_language_t (*next)(struct plugin_pack_language_it * it);
    char m_data[64];
};

plugin_pack_language_t plugin_pack_language_create(plugin_pack_packer_t packer, ui_data_language_t language);
void plugin_pack_language_free(plugin_pack_language_t language);

plugin_pack_language_t plugin_pack_language_find(plugin_pack_packer_t packer, ui_data_language_t language);

const char * plugin_pack_language_name(plugin_pack_language_t language);
ui_data_src_group_t plugin_pack_language_input_srcs(plugin_pack_language_t language);
ui_cache_group_t plugin_pack_language_textures(plugin_pack_language_t language);

void plugin_pack_packer_languages(plugin_pack_language_it_t it, plugin_pack_packer_t packer);
    
#define plugin_pack_language_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

