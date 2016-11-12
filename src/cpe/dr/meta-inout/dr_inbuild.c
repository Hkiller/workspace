#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "dr_inbuild_types.h"

static uint32_t dr_inbuild_macro_hash(const struct DRInBuildMacro * data) {
    return data->m_name ? cpe_hash_str(data->m_name, strlen(data->m_name)) : 1;
}

static int dr_inbuild_macro_cmp(const struct DRInBuildMacro * l, const struct DRInBuildMacro * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

static uint32_t dr_inbuild_meta_hash(const struct DRInBuildMeta * data) {
    return data->m_name ? cpe_hash_str(data->m_name, strlen(data->m_name)) : 1;
}

static int dr_inbuild_meta_cmp(const struct DRInBuildMeta * l, const struct DRInBuildMeta * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

struct DRInBuildMetaLib * dr_inbuild_create_lib(void) {
    struct DRInBuildMetaLib * inBuildMetaLib = (struct DRInBuildMetaLib *)malloc(sizeof(struct DRInBuildMetaLib));
    if (inBuildMetaLib == NULL) {
        return NULL;
    }

    bzero(inBuildMetaLib, sizeof(*inBuildMetaLib));

    mem_buffer_init(&inBuildMetaLib->m_tmp_buf, NULL);

    if (cpe_hash_table_init(
            &inBuildMetaLib->m_index_macros, 
            NULL,
            (cpe_hash_fun_t) dr_inbuild_macro_hash,
            (cpe_hash_eq_t) dr_inbuild_macro_cmp,
            CPE_HASH_OBJ2ENTRY(DRInBuildMacro, m_hh),
            -1) != 0)
    {
        mem_buffer_clear(&inBuildMetaLib->m_tmp_buf);
        free(inBuildMetaLib);
        return NULL;
    }

    if (cpe_hash_table_init(
            &inBuildMetaLib->m_index_metas, 
            NULL,
            (cpe_hash_fun_t) dr_inbuild_meta_hash,
            (cpe_hash_eq_t) dr_inbuild_meta_cmp,
            CPE_HASH_OBJ2ENTRY(DRInBuildMeta, m_hh),
            -1) != 0)
    {
        mem_buffer_clear(&inBuildMetaLib->m_tmp_buf);
        cpe_hash_table_fini(&inBuildMetaLib->m_index_macros);
        free(inBuildMetaLib);
        return NULL;
    }


    inBuildMetaLib->m_data.iID = -1;
    inBuildMetaLib->m_dft_align = CPE_DEFAULT_ALIGN;
    
    TAILQ_INIT(&inBuildMetaLib->m_macros);

    TAILQ_INIT(&inBuildMetaLib->m_metas);

    return inBuildMetaLib;
}

void dr_inbuild_free_lib(struct DRInBuildMetaLib * inBuildMetaLib) {
    if (inBuildMetaLib == NULL) {
        return;
    }

    /*free macro list*/
    while(! TAILQ_EMPTY(&inBuildMetaLib->m_macros)) {
        dr_inbuild_metalib_remove_macro(inBuildMetaLib, TAILQ_FIRST(&inBuildMetaLib->m_macros));
    }

    /*free meta list*/
    while(! TAILQ_EMPTY(&inBuildMetaLib->m_metas)) {
        dr_inbuild_metalib_remove_meta(inBuildMetaLib, TAILQ_FIRST(&inBuildMetaLib->m_metas));
    }


    mem_buffer_clear(&inBuildMetaLib->m_tmp_buf);
    cpe_hash_table_fini(&inBuildMetaLib->m_index_macros);
    cpe_hash_table_fini(&inBuildMetaLib->m_index_metas);

    free(inBuildMetaLib);
}

void dr_inbuild_meta_clear(struct DRInBuildMeta * meta) {

    /*free meta list*/
    while(! TAILQ_EMPTY(&meta->m_entries)) {
        dr_inbuild_meta_remove_entry(meta, TAILQ_FIRST(&meta->m_entries));
    }

    while(! TAILQ_EMPTY(&meta->m_key_entries)) {
        struct dr_inbuild_key_entry * key_entry = TAILQ_FIRST(&meta->m_key_entries);
        TAILQ_REMOVE(&meta->m_key_entries, key_entry, m_next);
        free(key_entry);
    }

    while(! TAILQ_EMPTY(&meta->m_indexes)) {
        struct dr_inbuild_index * index = TAILQ_FIRST(&meta->m_indexes);
        TAILQ_REMOVE(&meta->m_indexes, index, m_next);

        while(! TAILQ_EMPTY(&index->m_entries)) {
            struct dr_inbuild_index_entry * index_entry = TAILQ_FIRST(&index->m_entries);
            TAILQ_REMOVE(&index->m_entries, index_entry, m_next);
            free(index_entry);
        }

        free(index);
    }
}

int dr_inbuild_metalib_version(struct DRInBuildMetaLib * inBuildMetaLib) {
    return inBuildMetaLib->m_data.iVersion;
}

void dr_inbuild_metalib_set_version(struct DRInBuildMetaLib * inBuildMetaLib, int version) {
    inBuildMetaLib->m_data.iVersion = version;
}

void dr_inbuild_metalib_set_tagsetversion(struct DRInBuildMetaLib * inBuildMetaLib, int version) {
    inBuildMetaLib->m_data.iTagSetVersion = version;
}

void dr_inbuild_metalib_set_name(struct DRInBuildMetaLib * inBuildMetaLib, const char * name) {
    cpe_str_dup(inBuildMetaLib->m_data.szName, sizeof(inBuildMetaLib->m_data.szName), name);
}

struct DRInBuildMacro * dr_inbuild_metalib_add_macro(struct DRInBuildMetaLib * inBuildMetaLib) {
    struct DRInBuildMacro * newMacro = (struct DRInBuildMacro *)malloc(sizeof(struct DRInBuildMacro));
    if (newMacro == NULL) {
        return NULL;
    }

    bzero(newMacro, sizeof(struct DRInBuildMacro));
    cpe_hash_entry_init(&newMacro->m_hh);

    TAILQ_INSERT_TAIL(&inBuildMetaLib->m_macros, newMacro, m_next);

    return newMacro;
}

int dr_inbuild_metalib_add_macro_to_index(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMacro * macro) {
    return cpe_hash_table_insert_unique(&inBuildMetaLib->m_index_macros, macro);
}

void dr_inbuild_metalib_remove_macro(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMacro * macro) {
    if (macro == NULL) return;

    if (cpe_hash_table_find(&inBuildMetaLib->m_index_macros, macro) == macro) {
        cpe_hash_table_remove_by_ins(&inBuildMetaLib->m_index_macros, macro);
    }

    TAILQ_REMOVE(&inBuildMetaLib->m_macros, macro, m_next);

    free(macro);
}

struct DRInBuildMacro *
dr_inbuild_metalib_find_macro(
    struct DRInBuildMetaLib * inBuildMetaLib, const char * macro_name)
{
    struct DRInBuildMacro key;

    key.m_name = (char *)macro_name;

    return (struct DRInBuildMacro *)cpe_hash_table_find(&inBuildMetaLib->m_index_macros, &key);
}

struct DRInBuildMeta *
dr_inbuild_metalib_add_meta(struct DRInBuildMetaLib * inBuildMetaLib) {
    struct DRInBuildMeta * newMeta = (struct DRInBuildMeta *)malloc(sizeof(struct DRInBuildMeta));
    if (newMeta == NULL) {
        return NULL;
    }

    bzero(newMeta, sizeof(struct DRInBuildMeta));
    newMeta->m_lib = inBuildMetaLib;
    newMeta->m_data.m_id = -1;
    newMeta->m_data.m_align = inBuildMetaLib->m_dft_align;

    newMeta->m_entries_count = 0;
    TAILQ_INIT(&newMeta->m_entries);

    newMeta->m_key_entrie_count = 0;
    TAILQ_INIT(&newMeta->m_key_entries);

    newMeta->m_index_count = 0;
    TAILQ_INIT(&newMeta->m_indexes);

    TAILQ_INSERT_TAIL(&inBuildMetaLib->m_metas, newMeta, m_next);

    return newMeta;
}

void dr_inbuild_metalib_remove_meta(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMeta * meta) {
    assert(meta);

    dr_inbuild_meta_clear(meta);

    if (meta->m_name) {
        cpe_hash_table_remove_by_ins(&meta->m_lib->m_index_metas, meta);
    }

    TAILQ_REMOVE(&inBuildMetaLib->m_metas, meta, m_next);

    free(meta);
}

struct DRInBuildMeta *
dr_inbuild_metalib_find_meta(struct DRInBuildMetaLib * inBuildMetaLib, const char * meta_name) {
    struct DRInBuildMeta key;
    key.m_name = meta_name;

    return (struct DRInBuildMeta *)cpe_hash_table_find(&inBuildMetaLib->m_index_metas, &key);
}

void dr_inbuild_meta_set_type(struct DRInBuildMeta * meta, int type) {
    meta->m_data.m_type = type;
}

void dr_inbuild_meta_set_id(struct DRInBuildMeta * meta, int id) {
    meta->m_data.m_id = id;
}

void dr_inbuild_meta_set_align(struct DRInBuildMeta * meta, int align) {
    meta->m_data.m_align = align ? align : CPE_DEFAULT_ALIGN;
}

void dr_inbuild_meta_set_base_version(struct DRInBuildMeta * meta, int version) {
    meta->m_data.m_based_version = version;
}

int dr_inbuild_meta_current_version(struct DRInBuildMeta * meta) {
    return meta->m_data.m_current_version;
}

void dr_inbuild_meta_set_current_version(struct DRInBuildMeta * meta, int version) {
    meta->m_data.m_current_version = version;
}

int dr_inbuild_meta_set_name(struct DRInBuildMeta * meta, const char * name) {
    if (meta->m_name) {
        cpe_hash_table_remove_by_ins(&meta->m_lib->m_index_metas, meta);
    }

    meta->m_name = mem_buffer_strdup(&meta->m_lib->m_tmp_buf, name);
    if (cpe_hash_table_insert_unique(&meta->m_lib->m_index_metas, meta) != 0) {
        meta->m_name = NULL;
        return -1;
    }

    return 0;
}

void dr_inbuild_meta_set_desc(struct DRInBuildMeta * meta, const char * desc) {
    meta->m_desc = mem_buffer_strdup(&meta->m_lib->m_tmp_buf, desc);
}

int dr_inbuild_meta_add_key_entries(struct DRInBuildMeta * meta, const char * names) {
    const char * token_begin = names;
    const char * token_end = strchr(token_begin, ',');
    char name_buf[128];
    struct dr_inbuild_key_entry * key_entry;
    char * entry_name;

    for(; token_begin; token_begin = token_end ? token_end + 1 : NULL, token_end = token_begin ? strchr(token_begin, ',') : NULL) {
        if (token_end) {
            size_t len = token_end - token_begin;
            if ((len + 1) > sizeof(name_buf)) {
                return -1;
            }

            memcpy(name_buf, token_begin, len);
            name_buf[len] = 0;
        }
        else {
            cpe_str_dup(name_buf, sizeof(name_buf), token_begin);
        }

        entry_name = mem_buffer_strdup(&meta->m_lib->m_tmp_buf, name_buf);
        if (entry_name == NULL) return -1;

        key_entry = malloc(sizeof(struct dr_inbuild_key_entry));
        if (key_entry == NULL) return -1;

        key_entry->m_entry_name = entry_name;

        meta->m_key_entrie_count++;
        TAILQ_INSERT_TAIL(&meta->m_key_entries, key_entry, m_next);
    }

    return 0;
}

struct dr_inbuild_index * dr_inbuild_meta_add_index(struct DRInBuildMeta * meta, const char * name) {
    struct dr_inbuild_index * index = (struct dr_inbuild_index *)malloc(sizeof(struct dr_inbuild_index));
    if (index == NULL) return NULL;

    index->m_meta = meta;
    index->m_index_name = mem_buffer_strdup(&meta->m_lib->m_tmp_buf, name);
    index->m_entry_count = 0;
    TAILQ_INIT(&index->m_entries);

    ++meta->m_index_count;
    TAILQ_INSERT_TAIL(&meta->m_indexes, index, m_next);

    return index;
}

int dr_inbuild_index_add_entries(struct dr_inbuild_index * index, const char * names) {
    const char * token_begin = names;
    const char * token_end = strchr(token_begin, ',');
    char name_buf[128];
    struct dr_inbuild_index_entry * index_entry;
    char * entry_name;

    for(; token_begin; token_begin = token_end ? token_end + 1 : NULL, token_end = token_begin ? strchr(token_begin, ',') : NULL) {
        if (token_end) {
            size_t len = token_end - token_begin;
            if ((len + 1) > sizeof(name_buf)) {
                return -1;
            }

            memcpy(name_buf, token_begin, len);
            name_buf[len] = 0;
        }
        else {
            cpe_str_dup(name_buf, sizeof(name_buf), token_begin);
        }

        entry_name = mem_buffer_strdup(&index->m_meta->m_lib->m_tmp_buf, name_buf);
        if (entry_name == NULL) return -1;

        index_entry = malloc(sizeof(struct dr_inbuild_index_entry));
        if (index_entry == NULL) return -1;

        index_entry->m_entry_name = entry_name;

        index->m_entry_count++;
        TAILQ_INSERT_TAIL(&index->m_entries, index_entry, m_next);
    }

    return 0;
}

struct DRInBuildMetaEntry *
dr_inbuild_meta_add_entry(struct DRInBuildMeta * meta) {
    struct DRInBuildMetaEntry * newEntry = (struct DRInBuildMetaEntry *)malloc(sizeof(struct DRInBuildMetaEntry));
    if (newEntry == NULL) {
        return NULL;
    }

    bzero(newEntry, sizeof(struct DRInBuildMetaEntry));
    newEntry->m_meta = meta;
    newEntry->m_data.m_id = -1;
    newEntry->m_data.m_array_count = 1;
    newEntry->m_data.m_array_refer_data_start_pos = -1;
    newEntry->m_data.m_array_refer_entry_pos = -1;
    newEntry->m_data.m_type = CPE_DR_TYPE_UNKOWN;
    newEntry->m_data.m_select_range_min = 1;
    newEntry->m_data.m_select_data_start_pos = -1;
    newEntry->m_data.m_select_entry_pos = -1;
    newEntry->m_data.m_dft_value_pos = -1;

    TAILQ_INSERT_TAIL(&meta->m_entries, newEntry, m_next);
    ++meta->m_entries_count;

    return newEntry;
}

void dr_inbuild_meta_remove_entry(struct DRInBuildMeta * meta, struct DRInBuildMetaEntry * entry) {
    if (entry == NULL) {
        return;
    }

    TAILQ_REMOVE(&meta->m_entries, entry, m_next);
    --meta->m_entries_count;

    free(entry);
}

int dr_inbuild_entry_version(struct DRInBuildMetaEntry * entry) {
    return entry->m_data.m_version;
}

void dr_inbuild_entry_set_type(struct DRInBuildMetaEntry * entry, const char * type_name) {
    entry->m_ref_type_name = mem_buffer_strdup(&entry->m_meta->m_lib->m_tmp_buf, type_name);
}

void dr_inbuild_entry_set_id(struct DRInBuildMetaEntry * entry, int id) {
    entry->m_data.m_id = id;
}

void dr_inbuild_entry_set_version(struct DRInBuildMetaEntry * entry, int version) {
    entry->m_data.m_version = version;
}

void dr_inbuild_entry_set_array_count(struct DRInBuildMetaEntry * entry, int array_count) {
    entry->m_data.m_array_count = array_count;
}

void dr_inbuild_entry_set_array_refer(struct DRInBuildMetaEntry * entry, const char * refer) {
    entry->m_refer_path = mem_buffer_strdup(&entry->m_meta->m_lib->m_tmp_buf, refer);
}

void dr_inbuild_entry_set_selector(struct DRInBuildMetaEntry * entry, const char * selector) {
    entry->m_selector_path = mem_buffer_strdup(&entry->m_meta->m_lib->m_tmp_buf, selector);
}

void dr_inbuild_entry_set_name(struct DRInBuildMetaEntry * entry, const char * name) {
    entry->m_name = mem_buffer_strdup(&entry->m_meta->m_lib->m_tmp_buf, name);
}

void dr_inbuild_entry_set_desc(struct DRInBuildMetaEntry * entry, const char * desc) {
    entry->m_desc = mem_buffer_strdup(&entry->m_meta->m_lib->m_tmp_buf, desc);
}

void dr_inbuild_entry_set_size(struct DRInBuildMetaEntry * entry, int size) {
    entry->m_data.m_size = size;
}

struct DRInBuildMeta *
dr_inbuild_metalib_copy_meta(struct DRInBuildMetaLib * inBuildMetaLib, LPDRMETA meta) {
    struct DRInBuildMeta * new_meta;

    new_meta = dr_inbuild_metalib_find_meta(inBuildMetaLib, dr_meta_name(meta));
    if (new_meta != NULL) return new_meta;

    new_meta = dr_inbuild_metalib_add_meta(inBuildMetaLib);
    if (new_meta == NULL) return NULL;

    if (dr_inbuild_meta_init(new_meta, meta) != 0
        || dr_inbuild_meta_copy_entrys(new_meta, meta) != 0
        || dr_inbuild_meta_copy_keys(new_meta, meta) != 0
        || dr_inbuild_meta_copy_indexes(new_meta, meta) != 0)
    {
        dr_inbuild_metalib_remove_meta(inBuildMetaLib, new_meta);
        return NULL;
    }

    return new_meta;
}

struct DRInBuildMeta *
dr_inbuild_metalib_copy_meta_r(struct DRInBuildMetaLib * inBuildMetaLib, LPDRMETA meta) {
    struct DRInBuildMeta * new_meta;

    new_meta = dr_inbuild_metalib_find_meta(inBuildMetaLib, dr_meta_name(meta));
    if (new_meta != NULL) return new_meta;

    new_meta = dr_inbuild_metalib_add_meta(inBuildMetaLib);
    if (new_meta == NULL) return NULL;

    if (dr_inbuild_meta_init(new_meta, meta) != 0) {
        dr_inbuild_metalib_remove_meta(inBuildMetaLib, new_meta);
        return NULL;
    }

    if (dr_inbuild_metalib_copy_meta_ref_metas(inBuildMetaLib, meta) != 0
        || dr_inbuild_meta_copy_entrys(new_meta, meta) != 0
        || dr_inbuild_meta_copy_keys(new_meta, meta) != 0
        || dr_inbuild_meta_copy_indexes(new_meta, meta) != 0)
    {
        dr_inbuild_metalib_remove_meta(inBuildMetaLib, new_meta);
        return NULL;
    }

    TAILQ_REMOVE(&inBuildMetaLib->m_metas, new_meta, m_next);
    TAILQ_INSERT_TAIL(&inBuildMetaLib->m_metas, new_meta, m_next);
    
    return new_meta;
}

int dr_inbuild_metalib_copy_meta_ref_metas(struct DRInBuildMetaLib * inBuildMetaLib, LPDRMETA meta) {
    int i, count;
    
    count = dr_meta_entry_num(meta);
    for(i = 0; i < count; ++i) {
        LPDRMETA ref_meta = dr_entry_ref_meta(dr_meta_entry_at(meta, i));
        if (ref_meta) {
            if (dr_inbuild_metalib_copy_meta_r(inBuildMetaLib, ref_meta) == NULL) {
                return -1;
            }
        }
    }

    return 0;
}

int dr_inbuild_meta_init(struct DRInBuildMeta * new_meta, LPDRMETA meta) {
    dr_inbuild_meta_set_name(new_meta, dr_meta_name(meta));
    dr_inbuild_meta_set_current_version(new_meta, dr_meta_current_version(meta));
    dr_inbuild_meta_set_align(new_meta, dr_meta_align(meta));
    dr_inbuild_meta_set_id(new_meta, dr_meta_id(meta));
    dr_inbuild_meta_set_desc(new_meta, dr_meta_desc(meta));
    dr_inbuild_meta_set_type(new_meta, dr_meta_type(meta));
    return 0;
}


struct DRInBuildMetaEntry *
dr_inbuild_meta_copy_entry(struct DRInBuildMeta * meta, LPDRMETAENTRY entry) {
    char buf[256];
    struct DRInBuildMetaEntry * new_entry;
    LPDRMETA ref_meta;
    LPDRMETA self_meta;
    LPDRMETAENTRY select_entry;

    self_meta = dr_entry_self_meta(entry);
    assert(self_meta);

    new_entry = dr_inbuild_meta_add_entry(meta);
    if (new_entry == NULL) return NULL;

    dr_inbuild_entry_set_name(new_entry, mem_buffer_strdup(&meta->m_lib->m_tmp_buf, dr_entry_name(entry)));
    dr_inbuild_entry_set_version(new_entry, dr_entry_version(entry));
    dr_inbuild_entry_set_id(new_entry, dr_entry_id(entry));
    dr_inbuild_entry_set_type(new_entry, dr_entry_type_name(entry));
    dr_inbuild_entry_set_size(new_entry, entry->m_size);
    dr_inbuild_entry_set_array_count(new_entry, dr_entry_array_count(entry));

    if ((ref_meta = dr_entry_ref_meta(entry))) {
        if (dr_inbuild_metalib_find_meta(meta->m_lib, dr_meta_name(ref_meta)) == NULL) {
            dr_inbuild_metalib_copy_meta_r(meta->m_lib, ref_meta);
        }
    }

    if (dr_entry_array_count(entry) != 1) {
        const char * refer_path = dr_meta_off_to_path(self_meta, entry->m_array_refer_data_start_pos, buf, sizeof(buf));
        dr_inbuild_entry_set_array_refer(
            new_entry, 
            mem_buffer_strdup(&meta->m_lib->m_tmp_buf, refer_path));
    }

    if ((select_entry = dr_entry_select_entry(entry))) {
        const char * select_path = dr_meta_off_to_path(self_meta, entry->m_select_data_start_pos, buf, sizeof(buf));
        dr_inbuild_entry_set_selector(
            new_entry, 
            mem_buffer_strdup(&meta->m_lib->m_tmp_buf, select_path));
    }

    return new_entry;
}

int dr_inbuild_meta_copy_keys(struct DRInBuildMeta * new_meta, LPDRMETA src_meta) {
    int i, count;
    int rv;

    rv = 0;

    count = dr_meta_key_entry_num(src_meta);
    for(i = 0; i < count; ++i) {
        if (dr_inbuild_meta_add_key_entries(new_meta, dr_entry_name(dr_meta_key_entry_at(src_meta, i))) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int dr_inbuild_meta_copy_index(struct DRInBuildMeta * new_meta, dr_index_info_t src_index) {
    int i, count;
    int rv;
    struct dr_inbuild_index * new_index;

    rv = 0;

    new_index = dr_inbuild_meta_add_index(new_meta, dr_index_name(src_index));

    count = dr_index_entry_num(src_index);
    for(i = 0; i < count; ++i) {
        if (dr_inbuild_index_add_entries(new_index, dr_entry_name(dr_index_entry_at(src_index, i))) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int dr_inbuild_meta_copy_indexes(struct DRInBuildMeta * new_meta, LPDRMETA src_meta) {
    int i, count;
    int rv;

    rv = 0;

    count = dr_meta_index_num(src_meta);
    for(i = 0; i < count; ++i) {
        if (dr_inbuild_meta_copy_index(new_meta, dr_meta_index_at(src_meta, i)) != 0) {
                rv = -1;
        }
    }

    return rv;
}

int dr_inbuild_meta_copy_entrys(struct DRInBuildMeta * new_meta, LPDRMETA src_meta) {
    int i, count;

    count = dr_meta_entry_num(src_meta);
    for(i = 0; i < count; ++i) {
        if (dr_inbuild_meta_copy_entry(new_meta, dr_meta_entry_at(src_meta, i)) == NULL) {
            return -1;
        }
    }

    return 0;
}

int dr_inbuild_meta_copy_key_entrys(struct DRInBuildMeta * new_meta, LPDRMETA src_meta) {
    int i, count;
    int rv;

    rv = 0;

    count = dr_meta_key_entry_num(src_meta);
    for(i = 0; i < count; ++i) {
        LPDRMETAENTRY entry = dr_meta_key_entry_at(src_meta, i);
        if (dr_entry_is_key(entry)) {
            if (dr_inbuild_meta_copy_entry(new_meta, entry) == NULL) {
                rv = -1;
            }
        }
    }

    return rv;
}

struct DRInBuildMetaEntry *
dr_inbuild_meta_find_entry(struct DRInBuildMeta * meta, const char * name) {
    struct DRInBuildMetaEntry * entry;

    TAILQ_FOREACH(entry, &meta->m_entries, m_next) {
        if (strcmp(entry->m_name, name) == 0) return entry;
    }

    return NULL;
}
