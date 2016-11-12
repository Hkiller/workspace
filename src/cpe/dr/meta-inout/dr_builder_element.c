#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "dr_builder_ops.h"

dr_metalib_source_element_t
dr_metalib_source_element_create(
    dr_metalib_source_t source,
    dr_metalib_source_element_type_t type,
    const char * name)
{
    dr_metalib_source_element_t element;
    size_t name_len;

    name_len = strlen(name) + 1;

    element = (dr_metalib_source_element_t)
        mem_alloc(source->m_builder->m_alloc, sizeof(struct dr_metalib_source_element) + name_len);
    if (element == NULL) return NULL;

    element->m_source = source;
    element->m_type = type;
    memcpy(element + 1, name, name_len);

    cpe_hash_entry_init(&element->m_hh);
    if (cpe_hash_table_insert_unique(&source->m_builder->m_elements, element) != 0) {
        mem_free(source->m_builder->m_alloc, element);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&source->m_elements, element, m_next);

    return element;
}

void dr_metalib_source_element_free(dr_metalib_source_element_t element) {
    cpe_hash_table_remove_by_ins(&element->m_source->m_builder->m_elements, element);
    TAILQ_REMOVE(&element->m_source->m_elements, element, m_next);
    mem_free(element->m_source->m_builder->m_alloc, element);
}

const char * dr_metalib_source_element_name(dr_metalib_source_element_t element) {
    return (const char *)(element + 1);
}

dr_metalib_source_element_type_t
dr_metalib_source_element_type(dr_metalib_source_element_t element) {
    return element->m_type;
}

static dr_metalib_source_element_t dr_metalib_source_element_next(struct dr_metalib_source_element_it * it) {
    dr_metalib_source_element_t r;

    r = it->m_data;
    if (r) it->m_data = TAILQ_NEXT(r, m_next);
    return r;
}

void dr_metalib_source_elements(dr_metalib_source_element_it_t it, dr_metalib_source_t using_source) {
    it->next = dr_metalib_source_element_next;
    it->m_data = TAILQ_FIRST(&using_source->m_elements);
}

uint32_t dr_metalib_source_element_hash(dr_metalib_source_element_t element) {
    const char * name = dr_metalib_source_element_name(element);
    uint32_t r = cpe_hash_str(name, strlen(name));

    return (((uint32_t)element->m_type) << 30) | (r >> 2);
}

int dr_metalib_source_element_cmp(dr_metalib_source_element_t l, dr_metalib_source_element_t r) {
    return l->m_type == r->m_type
        && strcmp(dr_metalib_source_element_name(l), dr_metalib_source_element_name(r)) == 0;
}
