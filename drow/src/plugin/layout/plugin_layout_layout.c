#include <assert.h>
#include "plugin_layout_layout_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_layout_meta_i.h"

plugin_layout_layout_t
plugin_layout_layout_create(plugin_layout_render_t render, plugin_layout_layout_meta_t meta) {
    plugin_layout_module_t module = render->m_module;
    plugin_layout_layout_t layout;
    
    assert(render->m_layout == NULL);

    layout = TAILQ_FIRST(&module->m_free_layouts);
    if (layout) {
        TAILQ_REMOVE(&module->m_free_layouts, layout, m_next_for_module);
    }
    else {
        layout = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_layout) + module->m_layout_data_capacity);
        if (layout == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_layout_create: alloc fail!");
            return NULL;
        }
    }

    layout->m_render = render;
    layout->m_meta = meta;
    render->m_layout = layout;
    
    if (layout->m_meta->m_init) {
        if (layout->m_meta->m_init(layout) != 0) {
            CPE_ERROR(module->m_em, "plugin_layout_layout_create: init fail!");
            render->m_layout = NULL;
            layout->m_render = (void*)module;
            TAILQ_INSERT_TAIL(&module->m_free_layouts, layout, m_next_for_module);
            return NULL;
        }
    }

    return layout;
}

void plugin_layout_layout_free(plugin_layout_layout_t layout) {
    plugin_layout_module_t module;
    
    assert(layout->m_render);
    assert(layout->m_render->m_layout == layout);

    module = layout->m_render->m_module;

    if (layout->m_meta->m_fini) {
        layout->m_meta->m_fini(layout);
    }

    layout->m_render->m_layout = NULL;

    layout->m_render = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_layouts, layout, m_next_for_module);
}

int plugin_layout_layout_setup(plugin_layout_layout_t layout, char * args) {
    plugin_layout_module_t module = layout->m_render->m_module;
    
    if (layout->m_meta->m_setup) {
        if (layout->m_meta->m_setup(layout, args) != 0) {
            CPE_ERROR(module->m_em, "plugin_layout_layout_setup: layout %s setup fail", layout->m_meta->m_name);
            return -1;
        }
        return 0;
    }
    else {
        CPE_ERROR(module->m_em, "plugin_layout_layout_setup: layout %s not support setup", layout->m_meta->m_name);
        return -1;
    }
}

void * plugin_layout_layout_data(plugin_layout_layout_t layout) {
    return layout + 1;
}

plugin_layout_layout_t plugin_layout_layout_from_data(void * data) {
    return ((plugin_layout_layout_t)data) - 1;
}

plugin_layout_render_t plugin_layout_layout_render(plugin_layout_layout_t layout) {
    return layout->m_render;
}

plugin_layout_layout_meta_t plugin_layout_layout_meta(plugin_layout_layout_t layout) {
    return layout->m_meta;
}

const char * plugin_layout_layout_meta_name(plugin_layout_layout_t layout) {
    return layout->m_meta->m_name;
}

void plugin_layout_layout_real_free(plugin_layout_layout_t layout) {
    plugin_layout_module_t module = (void*)layout->m_render;
    TAILQ_REMOVE(&module->m_free_layouts, layout, m_next_for_module);
    mem_free(module->m_alloc, layout);
}
