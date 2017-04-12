#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_language.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_pack_language_i.h"
#include "plugin_pack_texture_i.h"

plugin_pack_language_t plugin_pack_language_create(plugin_pack_packer_t packer, ui_data_language_t language) {
    plugin_pack_language_t pack_language;
    char path_buf[128];
    
    pack_language = mem_alloc(packer->m_module->m_alloc, sizeof(struct plugin_pack_packer));
    if (pack_language == NULL) {
        CPE_ERROR(packer->m_module->m_em, "plugin_pack_language_create: alloc fail!");
        return NULL;
    }

    cpe_str_dup(path_buf, sizeof(path_buf), packer->m_common_texture->m_path);
    if (language) {
        char * sep;
        const char * language_name = ui_data_language_name(language);
        size_t language_name_len = strlen(language_name);
        
        sep = strrchr(path_buf, '.');
        if (sep == NULL) {
            CPE_ERROR(packer->m_module->m_em, "plugin_pack_language_create: path %s format error!", path_buf);
            return NULL;
        }

        if (strlen(path_buf) + language_name_len + 2 >= CPE_ARRAY_SIZE(path_buf)) {
            CPE_ERROR(packer->m_module->m_em, "plugin_pack_language_create: path %s language %s buf overflow!", path_buf, language_name);
            return NULL;
        }

        memmove(sep + language_name_len + 1, sep, strlen(sep) + 1);
        sep[0] = '_';
        memcpy(sep + 1, language_name, language_name_len);
    }

    pack_language->m_packer = packer;
    pack_language->m_data_language = language;
    pack_language->m_texture = plugin_pack_texture_create(packer, path_buf);
    pack_language->m_input_srcs = ui_data_src_group_create(packer->m_module->m_data_mgr);
    pack_language->m_textures = ui_cache_group_create(packer->m_module->m_cache_mgr);
    //ui_cache_group_add_res(pack_language->m_textures, pack_language->m_texture->m_cache_texture);

    TAILQ_INSERT_TAIL(&packer->m_languages, pack_language, m_next);

    if (strcmp(packer->m_default_language_name, ui_data_language_name(language)) == 0) {
        packer->m_default_language = pack_language;
    }
    
    return pack_language;
}

void plugin_pack_language_free(plugin_pack_language_t language) {
    plugin_pack_packer_t packer = language->m_packer;

    plugin_pack_texture_free(language->m_texture);
    ui_data_src_group_free(language->m_input_srcs);
    ui_cache_group_free(language->m_textures);
    
    if (packer->m_default_language == language) {
        packer->m_default_language = NULL;
    }
    
    TAILQ_REMOVE(&packer->m_languages, language, m_next);
    mem_free(packer->m_module->m_alloc, language);
}

plugin_pack_language_t plugin_pack_language_find(plugin_pack_packer_t packer, ui_data_language_t data_language) {
    plugin_pack_language_t pack_language;
    
    TAILQ_FOREACH(pack_language, &packer->m_languages, m_next) {
        if (pack_language->m_data_language == data_language) return pack_language;
    }

    return NULL;
}

const char * plugin_pack_language_name(plugin_pack_language_t language) {
    return ui_data_language_name(language->m_data_language);
}

ui_data_src_group_t plugin_pack_language_input_srcs(plugin_pack_language_t language) {
    return language->m_input_srcs;
}

ui_cache_group_t plugin_pack_language_textures(plugin_pack_language_t language) {
    return language->m_textures;
}

static plugin_pack_language_t plugin_pack_language_next(struct plugin_pack_language_it * it) {
    plugin_pack_language_t * data = (plugin_pack_language_t *)(it->m_data);
    plugin_pack_language_t r = *data;

    if (r) {
        *data = TAILQ_NEXT(r, m_next);
    }

    return r;
}

void plugin_pack_packer_languages(plugin_pack_language_it_t it, plugin_pack_packer_t packer) {
    *(plugin_pack_language_t *)(it->m_data) = TAILQ_FIRST(&packer->m_languages);
    it->next = plugin_pack_language_next;
}

