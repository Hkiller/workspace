#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_build.h"
#include "dr_builder_ops.h"

struct dr_metalib_source_relation *
dr_metalib_source_relation_create(dr_metalib_source_t user, dr_metalib_source_t using);
void dr_metalib_source_relation_free(struct dr_metalib_source_relation * relation);

uint32_t dr_metalib_source_hash(dr_metalib_source_t source) {
    return cpe_hash_str(source->m_name, strlen(source->m_name));
}

int dr_metalib_source_cmp(dr_metalib_source_t l, dr_metalib_source_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

static dr_metalib_source_t
dr_metalib_source_create(
    dr_metalib_builder_t builder, const char * name, size_t capacity,
    dr_metalib_source_type_t type,
    dr_metalib_source_format_t format,
    dr_metalib_source_from_t from)
{
    char * buf;
    dr_metalib_source_t source;
    size_t name_len;

    assert(builder);

    name_len = strlen(name) + 1;
    CPE_PAL_ALIGN_DFT(name_len);

    buf = mem_alloc(builder->m_alloc, name_len + sizeof(struct dr_metalib_source) + capacity);
    if (buf == NULL) return NULL;

    cpe_str_dup(buf, name_len, name);

    source = (dr_metalib_source_t)(buf + name_len);
    source->m_builder = builder;
    source->m_name = buf;
    source->m_type = type;
    source->m_format = format;
    source->m_from = from;
    source->m_state = dr_metalib_source_state_not_analize;
    source->m_capacity = capacity;

    TAILQ_INIT(&source->m_includes);
    TAILQ_INIT(&source->m_include_by);
    TAILQ_INIT(&source->m_elements);

    cpe_hash_entry_init(&source->m_hh);
    if (cpe_hash_table_insert_unique(&builder->m_sources, source) != 0) {
        mem_free(builder->m_alloc, buf);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&builder->m_sources_in_order, source, m_next);

    return source;
}

dr_metalib_source_t
dr_metalib_builder_add_file(dr_metalib_builder_t builder, const char * name, const char * file) {
    dr_metalib_source_t source;
    struct mem_buffer name_buffer;
    size_t file_len;
    const char * suffix;
    dr_metalib_source_format_t format;

    assert(builder);
    assert(file);

    suffix = file_name_suffix(file);
    if (suffix == NULL) return 0;
    if (strcmp(suffix, "xml") == 0) {
        format = dr_metalib_source_format_xml;
    }
    else {
        return NULL;
    }

    mem_buffer_init(&name_buffer, 0);
    if (name == NULL) {
        name = file_name_base(file, &name_buffer);
        if (name == NULL) {
            mem_buffer_clear(&name_buffer);
            return NULL;
        }
    }

    assert(name);

    file_len = strlen(file) + 1;
    source = dr_metalib_source_create(builder, name, file_len, dr_metalib_source_type_file, format, dr_metalib_source_from_user);
    if (source == NULL) {
        mem_buffer_clear(&name_buffer);
        return NULL;
    }

    memcpy(source + 1, file, file_len);

    mem_buffer_clear(&name_buffer);
    return source;
}

dr_metalib_source_t
dr_metalib_builder_add_buf(dr_metalib_builder_t builder, const char * name, dr_metalib_source_format_t format, const char * buf) {
    dr_metalib_source_t source;
    size_t buf_len;

    assert(builder);
    assert(buf);

    buf_len = strlen(buf) + 1;
    source = dr_metalib_source_create(builder, name, buf_len, dr_metalib_source_type_memory, format, dr_metalib_source_from_user);
    if (source == NULL) return NULL;

    memcpy(source + 1, buf, buf_len);

    return source;
}

void dr_metalib_source_free(dr_metalib_source_t source) {
    assert(source);

    while(!TAILQ_EMPTY(&source->m_elements)) {
        dr_metalib_source_element_free(TAILQ_FIRST(&source->m_elements));
    }

    while(!TAILQ_EMPTY(&source->m_includes)) {
        dr_metalib_source_relation_free(TAILQ_FIRST(&source->m_includes));
    }

    while(!TAILQ_EMPTY(&source->m_include_by)) {
        dr_metalib_source_relation_free(TAILQ_FIRST(&source->m_include_by));
    }

    TAILQ_REMOVE(&source->m_builder->m_sources_in_order, source, m_next);
    cpe_hash_table_remove_by_ins(&source->m_builder->m_sources, source);

    mem_free(source->m_builder->m_alloc, (void *)source->m_name);
}

dr_metalib_source_t
dr_metalib_source_find(dr_metalib_builder_t builder, const char * name) {
    struct dr_metalib_source key;
    key.m_name = name;

    return (dr_metalib_source_t)cpe_hash_table_find(&builder->m_sources, &key);
}

const char * dr_metalib_source_name(dr_metalib_source_t source) {
    return source->m_name;
}

const char * dr_metalib_source_file(dr_metalib_source_t source) {
    return source->m_type == dr_metalib_source_type_file
        ? (const char *)(source + 1)
        : NULL;
}

const void * dr_metalib_source_buf(dr_metalib_source_t source) {
    return source->m_type == dr_metalib_source_type_memory
        ? (const void *)(source + 1)
        : NULL;
}

size_t dr_metalib_source_buf_capacity(dr_metalib_source_t source) {
    return source->m_type == dr_metalib_source_type_memory
        ? source->m_capacity
        : 0;
}

int dr_metalib_source_add_include(dr_metalib_source_t user, dr_metalib_source_t using) {
    assert(user);
    assert(using);
    return dr_metalib_source_relation_create(user, using) == NULL ? -1 : 0;
}

dr_metalib_source_t
dr_metalib_source_add_include_file(dr_metalib_source_t user_source, const char * name, const char * file, dr_metalib_source_from_t from) {
    struct mem_buffer name_buffer;
    dr_metalib_source_t using;
    
    assert(user_source);
    assert(file);
    assert(user_source->m_builder);

    mem_buffer_init(&name_buffer, 0);

    if (name == NULL) {
        name = file_name_base(file, &name_buffer);
        if (name == NULL) {
            mem_buffer_clear(&name_buffer);
            return NULL;
        }
    }
    assert(name);
    
    using = dr_metalib_source_find(user_source->m_builder, name);
    if (using == NULL) {
        using = dr_metalib_builder_add_file(user_source->m_builder, name, file);
        if (using == NULL) {
            mem_buffer_clear(&name_buffer);
            return NULL;
        }
        using->m_from = from;
    }
    assert(using);

    mem_buffer_clear(&name_buffer);

    return dr_metalib_source_add_include(user_source, using) == 0
        ? using
        : NULL;
}

dr_metalib_source_type_t dr_metalib_source_type(dr_metalib_source_t source) {
    return source->m_type;
}

dr_metalib_source_format_t dr_metalib_source_format(dr_metalib_source_t source) {
    return source->m_format;
}

dr_metalib_source_from_t dr_metalib_source_from(dr_metalib_source_t source) {
    return source->m_from;
}

dr_metalib_source_state_t dr_metalib_source_state(dr_metalib_source_t source) {
    return source->m_state;
}

const char * dr_metalib_source_libname(dr_metalib_source_t source) {
    dr_metalib_source_element_t e;

    TAILQ_FOREACH(e, &source->m_elements, m_next) {
        if (e->m_type == dr_metalib_source_element_type_lib) {
            return dr_metalib_source_element_name(e);
        }
    }

    return NULL;
}

static void dr_metalib_source_do_analize(dr_metalib_source_t source, const void * buf, size_t bufSize) {
    if(source->m_format == dr_metalib_source_format_xml) {
        dr_metalib_source_analize_xml(
            source,
            source->m_builder->m_inbuild_lib,
            buf, (int)bufSize,
            source->m_builder->m_em);
    }
    else {
        CPE_ERROR(source->m_builder->m_em, "not support source format %d", source->m_format);
    }
}

void dr_metalib_source_analize(dr_metalib_source_t source) {
    assert(source);
    if (source->m_state != dr_metalib_source_state_not_analize) return;

    source->m_state = dr_metalib_source_state_analizing;

    if(source->m_type == dr_metalib_source_type_file) {
        struct mem_buffer buffer;
        FILE * file;
        const char * file_name;

        file_name = dr_metalib_source_file(source);
        assert(file_name);

        file = file_stream_open(file_name, "r", source->m_builder->m_em);
        if (file) {
            ssize_t file_size;
            mem_buffer_init(&buffer, 0);
            file_size = file_stream_load_to_buffer(&buffer, file, source->m_builder->m_em);
            if (file_size > 0) {
                CPE_ERROR_SET_FILE(source->m_builder->m_em, file_name);
                dr_metalib_source_do_analize(source, mem_buffer_make_continuous(&buffer, 0), file_size);
                CPE_ERROR_SET_FILE(source->m_builder->m_em, 0);
            }
            mem_buffer_clear(&buffer);
            file_stream_close(file, source->m_builder->m_em);
        }
    }
    else if (source->m_type == dr_metalib_source_type_memory) {
        CPE_ERROR_SET_FILE(source->m_builder->m_em, source->m_name);
        dr_metalib_source_do_analize(source, source + 1, source->m_capacity);
        CPE_ERROR_SET_FILE(source->m_builder->m_em, 0);
    }
    else {
        CPE_ERROR(source->m_builder->m_em, "%s: not support type %d", source->m_name, source->m_type);
    }

    source->m_state = dr_metalib_source_state_analized;
}

struct dr_metalib_source_relation *
dr_metalib_source_relation_create(dr_metalib_source_t user, dr_metalib_source_t using) {
    struct dr_metalib_source_relation * relation;
    assert(using);
    assert(user);

    relation = mem_alloc(user->m_builder->m_alloc, sizeof(struct dr_metalib_source_relation));
    if (relation == NULL) return NULL;

    relation->m_user = user;
    relation->m_using = using;

    TAILQ_INSERT_TAIL(&user->m_includes, relation, m_next_for_includes);
    TAILQ_INSERT_TAIL(&using->m_include_by, relation, m_next_for_include_by);

    return relation;
}

void dr_metalib_source_relation_free(struct dr_metalib_source_relation * relation) {
    TAILQ_REMOVE(&relation->m_user->m_includes, relation, m_next_for_includes);
    TAILQ_REMOVE(&relation->m_using->m_include_by, relation, m_next_for_include_by);
    mem_free(relation->m_using->m_builder->m_alloc, relation);
}

static
dr_metalib_source_t
dr_metalib_source_include_next(struct dr_metalib_source_it * it) {
    struct dr_metalib_source_relation ** relation;
    dr_metalib_source_t r;

    relation = (struct dr_metalib_source_relation **)it->m_data;

    if ((*relation) == NULL) return NULL;

    r = (*relation)->m_using;
    *relation = TAILQ_NEXT(*relation, m_next_for_includes);
    return r;
}

void dr_metalib_source_includes(struct dr_metalib_source_it * it, dr_metalib_source_t source) {
    struct dr_metalib_source_relation ** relation;

    relation = (struct dr_metalib_source_relation **)it->m_data;

    it->next = dr_metalib_source_include_next; 
    *relation = TAILQ_EMPTY(&source->m_includes)
        ? NULL
        : TAILQ_FIRST(&source->m_includes);
}

static
dr_metalib_source_t
dr_metalib_source_include_by_next(struct dr_metalib_source_it * it) {
    struct dr_metalib_source_relation ** relation;
    dr_metalib_source_t r;

    relation = (struct dr_metalib_source_relation **)it->m_data;

    if ((*relation) == NULL) return NULL;

    r = (*relation)->m_user;
    *relation = TAILQ_NEXT(*relation, m_next_for_include_by);
    return r;
}


void dr_metalib_source_include_by(struct dr_metalib_source_it * it, dr_metalib_source_t source) {
    struct dr_metalib_source_relation ** relation;

    relation = (struct dr_metalib_source_relation **)it->m_data;

    it->next = dr_metalib_source_include_by_next; 
    *relation = TAILQ_EMPTY(&source->m_include_by)
        ? NULL
        : TAILQ_FIRST(&source->m_include_by);
}
