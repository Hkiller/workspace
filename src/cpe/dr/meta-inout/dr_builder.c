#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_build.h"
#include "dr_builder_ops.h"

dr_metalib_builder_t
dr_metalib_builder_create(mem_allocrator_t alloc, error_monitor_t em) {
    dr_metalib_builder_t builder;
    builder = mem_alloc(alloc, sizeof(struct dr_metalib_builder));
    if (builder == NULL) return NULL;

    builder->m_inbuild_lib = dr_inbuild_create_lib();
    if (builder->m_inbuild_lib == NULL) {
        mem_free(alloc, builder);
        return NULL;
    }

    builder->m_alloc = alloc;
    builder->m_em = em;

    if (cpe_hash_table_init(
            &builder->m_sources,
            alloc,
            (cpe_hash_fun_t)dr_metalib_source_hash,
            (cpe_hash_eq_t)dr_metalib_source_cmp,
            CPE_HASH_OBJ2ENTRY(dr_metalib_source, m_hh),
            256) != 0)
    {
        dr_inbuild_free_lib(builder->m_inbuild_lib);
        mem_free(alloc, builder);
        return NULL;
    }

    if (cpe_hash_table_init(
            &builder->m_elements,
            alloc,
            (cpe_hash_fun_t)dr_metalib_source_element_hash,
            (cpe_hash_eq_t)dr_metalib_source_element_cmp,
            CPE_HASH_OBJ2ENTRY(dr_metalib_source_element, m_hh),
            256) != 0)
    {
        cpe_hash_table_fini(&builder->m_sources);
        dr_inbuild_free_lib(builder->m_inbuild_lib);
        mem_free(alloc, builder);
        return NULL;
    }

    TAILQ_INIT(&builder->m_sources_in_order);

    return builder;
}

void dr_metalib_builder_free_sources(dr_metalib_builder_t builder) {
    struct cpe_hash_it sourceIt;
    dr_metalib_source_t source;

    cpe_hash_it_init(&sourceIt, &builder->m_sources);
    source = cpe_hash_it_next(&sourceIt);
    while(source) {
        dr_metalib_source_t next = cpe_hash_it_next(&sourceIt);
        dr_metalib_source_free(source);
        source = next;
    }

    cpe_hash_table_fini(&builder->m_sources);
}

void dr_metalib_builder_free_elements(dr_metalib_builder_t builder) {
    struct cpe_hash_it elementIt;
    dr_metalib_source_element_t element;

    cpe_hash_it_init(&elementIt, &builder->m_elements);
    element = cpe_hash_it_next(&elementIt);
    while(element) {
        dr_metalib_source_element_t next = cpe_hash_it_next(&elementIt);
        dr_metalib_source_element_free(element);
        element = next;
    }

    cpe_hash_table_fini(&builder->m_elements);
}

void dr_metalib_builder_free(dr_metalib_builder_t builder) {
    dr_metalib_builder_free_sources(builder);
    dr_metalib_builder_free_elements(builder);
    dr_inbuild_free_lib(builder->m_inbuild_lib);
    mem_free(builder->m_alloc, builder);
}

void dr_metalib_builder_analize(dr_metalib_builder_t builder) {
    dr_metalib_source_t source;

    TAILQ_FOREACH(source, &builder->m_sources_in_order, m_next) {
        dr_metalib_source_analize(source);
    }
}

struct DRInBuildMetaLib * dr_metalib_bilder_lib(dr_metalib_builder_t builder) {
    return builder->m_inbuild_lib;
}

static
dr_metalib_source_t
dr_metalib_builder_source_next(struct dr_metalib_source_it * it) {
    struct cpe_hash_it * sourceIt;

    assert(sizeof(it->m_data) >= sizeof(struct cpe_hash_it));

    sourceIt = (struct cpe_hash_it *)(&it->m_data);

    return (dr_metalib_source_t)cpe_hash_it_next(sourceIt);
}

void dr_metalib_builder_sources(dr_metalib_source_it_t it, dr_metalib_builder_t builder) {
    struct cpe_hash_it * sourceIt;

    assert(sizeof(it->m_data) >= sizeof(struct cpe_hash_it));

    sourceIt = (struct cpe_hash_it *)(&it->m_data);

    cpe_hash_it_init(sourceIt, &builder->m_sources);
    it->next = dr_metalib_builder_source_next;
}
