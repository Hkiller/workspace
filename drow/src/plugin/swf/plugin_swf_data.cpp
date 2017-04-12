#include <assert.h>
#include "gameswf/gameswf_movie_def.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_swf_data_i.hpp"

extern "C"
plugin_swf_data_t plugin_swf_data_create(plugin_swf_module_t module, ui_data_src_t src) {
    plugin_swf_data_t swf_data;

    if (ui_data_src_type(src) != ui_data_src_type_swf) {
        CPE_ERROR(
            module->m_em, "create swf at %s: src not swf!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create swf at %s: product already loaded!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    swf_data = (plugin_swf_data_t)mem_alloc(module->m_alloc, sizeof(struct plugin_swf_data));
    if (swf_data == NULL) {
        CPE_ERROR(
            module->m_em, "create swf at %s: alloc fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    swf_data->m_module = module;
    swf_data->m_src = src;
    new (&swf_data->m_movie_def) gc_ptr<movie_def_impl>();

    ui_data_src_set_product(src, (void*)swf_data);

    return swf_data;
}

extern "C"
void plugin_swf_data_free(plugin_swf_data_t swf_data) {
    plugin_swf_module_t module = swf_data->m_module;

    swf_data->m_movie_def.~gc_ptr<movie_def_impl>();
    
    mem_free(module->m_alloc, swf_data);
}

int plugin_swf_data_update_usings(ui_data_src_t src) {
    return 0;
}
