#include <assert.h>
#include "render/utils/ui_rect.h"
#include "plugin_layout_font_element_i.h"
#include "plugin_layout_font_face_i.h"
#include "plugin_layout_font_meta_i.h"

plugin_layout_font_element_t
plugin_layout_font_element_create(plugin_layout_font_face_t face, uint32_t charter) {
    plugin_layout_module_t module = face->m_module;
    plugin_layout_font_element_t element;
    
    /*创建元素 */
    element = TAILQ_FIRST(&module->m_free_font_elements);
    if (element) {
        assert(module->m_free_element_count > 0);
        module->m_free_element_count--;
        TAILQ_REMOVE(&module->m_free_font_elements, element, m_next);
    }
    else {
        element = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_font_element) + module->m_max_element_capacity);
        if (element == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_font_element_create: alloc fail!");
            return NULL;
        }
    }

    element->m_face = face;
    element->m_charter = charter;
    element->m_node_count = 0;
    
    if (face->m_meta->m_init_element(face->m_meta->m_ctx, element) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_layout_font_element_create: element %s-%d.%d init fail!",
            face->m_meta->m_name, face->m_id.size, charter);
        goto CREATE_FAIL;
    }

    cpe_hash_entry_init(&element->m_hh);
    if (cpe_hash_table_insert(&face->m_elements, element) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_layout_font_element_create: element name %s-%d.%d duplicate!",
            face->m_meta->m_name, face->m_id.size, charter);
        face->m_meta->m_fini_element(face->m_meta->m_ctx, element);
        goto CREATE_FAIL;
    }

    module->m_element_count++;
    return element;

CREATE_FAIL:
    element->m_face = (plugin_layout_font_face_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_font_elements, element, m_next);
    module->m_free_element_count++;
    return NULL; 
}

void plugin_layout_font_element_free(plugin_layout_font_element_t element) {
    plugin_layout_font_face_t face = element->m_face;
    plugin_layout_module_t module = face->m_module;

    assert(element->m_node_count == 0);
    face->m_meta->m_fini_element(face->m_meta->m_ctx, element);

    assert(module->m_element_count > 0);
    module->m_element_count--;
    cpe_hash_table_remove_by_ins(&face->m_elements, element);

    module->m_free_element_count++;
    element->m_face = (plugin_layout_font_face_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_font_elements, element, m_next);
}

void plugin_layout_font_element_real_free(plugin_layout_font_element_t element) {
    plugin_layout_module_t module = (plugin_layout_module_t)element->m_face;

    assert(module->m_free_element_count > 0);
    module->m_free_element_count--;
    TAILQ_REMOVE(&module->m_free_font_elements, element, m_next);

    mem_free(module->m_alloc, element);
}

void plugin_layout_font_element_free_all(plugin_layout_font_face_t face) {
    struct cpe_hash_it element_it;
    plugin_layout_font_element_t element;

    cpe_hash_it_init(&element_it, &face->m_elements);

    element = cpe_hash_it_next(&element_it);
    while (element) {
        plugin_layout_font_element_t next = cpe_hash_it_next(&element_it);
        plugin_layout_font_element_free(element);
        element = next;
    }
}

plugin_layout_font_element_t
plugin_layout_font_element_find(plugin_layout_font_face_t face, uint32_t charter) {
    struct plugin_layout_font_element key;
    key.m_face = face;
    key.m_charter = charter;
    return cpe_hash_table_find(&face->m_elements, &key);
}

plugin_layout_font_element_t
plugin_layout_font_element_check_create(plugin_layout_font_face_t face, uint32_t charter) {
    plugin_layout_font_element_t element;

    element = plugin_layout_font_element_find(face, charter);
    if (element == NULL) {
        element = plugin_layout_font_element_create(face, charter);
    }

    return element;
}

void * plugin_layout_font_element_data(plugin_layout_font_element_t element) {
    return element + 1;
}

uint32_t plugin_layout_font_element_hash(const plugin_layout_font_element_t element) {
    return (((uint32_t)(ptr_int_t)element->m_face) << 16) | (element->m_charter & 0xFFFF);
}

int plugin_layout_font_element_eq(const plugin_layout_font_element_t l, const plugin_layout_font_element_t r) {
    return l->m_face == r->m_face && l->m_charter == r->m_charter
        ? 1
        : 0;
}

void plugin_layout_font_element_clear_not_used(plugin_layout_module_t module) {
    plugin_layout_font_meta_t meta;
    plugin_layout_font_face_t face;

    TAILQ_FOREACH(meta, &module->m_font_metas, m_next) {
        TAILQ_FOREACH(face, &meta->m_faces, m_next_for_meta) {
            struct cpe_hash_it element_it;
            plugin_layout_font_element_t element;

            cpe_hash_it_init(&element_it, &face->m_elements);

            element = cpe_hash_it_next(&element_it);
            while (element) {
                plugin_layout_font_element_t next = cpe_hash_it_next(&element_it);
                if (element->m_node_count == 0) {
                    plugin_layout_font_element_free(element);
                }
                element = next;
            }
        }
    }
}
    
