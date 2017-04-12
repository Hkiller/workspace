#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "pom_grp_internal_ops.h"

static int pom_grp_meta_build_store_meta_on_entry_normal(
    struct DRInBuildMetaLib * builder,
    struct DRInBuildMeta * meta,
    pom_grp_entry_meta_t entry,
    mem_buffer_t str_buffer,
    error_monitor_t em)
{
    LPDRMETA entry_meta = pom_grp_entry_meta_normal_meta(entry);

    struct DRInBuildMetaEntry * new_entry = dr_inbuild_meta_add_entry(meta);
    if (new_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %s fail!", entry->m_name);
        return -1;
    }

    dr_inbuild_entry_set_name(new_entry, entry->m_name);
    dr_inbuild_entry_set_id(new_entry, -1);
    dr_inbuild_entry_set_type(new_entry, dr_meta_name(entry_meta));
    dr_inbuild_entry_set_array_count(new_entry, 1);

    if (dr_inbuild_metalib_find_meta(builder, dr_meta_name(entry_meta)) == NULL) {
        if (dr_inbuild_metalib_copy_meta(builder, entry_meta) == NULL) {
            CPE_ERROR(em, "pom_grp_meta_build_store_meta: create meta for %s fail!", entry->m_name);
            return -1;
        }
    }

    return 0;
}

static int pom_grp_meta_build_store_meta_on_entry_ba(
    struct DRInBuildMeta * meta,
    pom_grp_entry_meta_t entry,
    mem_buffer_t str_buffer,
    error_monitor_t em)
{
    struct DRInBuildMetaEntry * new_entry = dr_inbuild_meta_add_entry(meta);
    if (new_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %s fail!", entry->m_name);
        return -1;
    }

    dr_inbuild_entry_set_name(new_entry, entry->m_name);
    dr_inbuild_entry_set_id(new_entry, -1);
    dr_inbuild_entry_set_type(new_entry, "uint8");
    dr_inbuild_entry_set_array_count(new_entry, pom_grp_entry_meta_ba_bytes(entry));

    return 0;
}

static int pom_grp_meta_build_store_meta_on_entry_binary(
    struct DRInBuildMeta * meta,
    pom_grp_entry_meta_t entry,
    mem_buffer_t str_buffer,
    error_monitor_t em)
{
    struct DRInBuildMetaEntry * new_entry = dr_inbuild_meta_add_entry(meta);
    if (new_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %s fail!", entry->m_name);
        return -1;
    }

    dr_inbuild_entry_set_name(new_entry, entry->m_name);
    dr_inbuild_entry_set_id(new_entry, -1);
    dr_inbuild_entry_set_type(new_entry, "uint8");
    dr_inbuild_entry_set_array_count(new_entry, pom_grp_entry_meta_binary_capacity(entry));

    return 0;
}

static int pom_grp_meta_build_store_meta_on_entry_list(
    struct DRInBuildMetaLib * builder,
    struct DRInBuildMeta * main_meta,
    LPDRMETA src_main_meta,
    pom_grp_entry_meta_t entry,
    mem_buffer_t str_buffer,
    error_monitor_t em)
{
    LPDRMETA entry_meta = pom_grp_entry_meta_list_meta(entry);
    assert(entry_meta);

    if (entry->m_data.m_list.m_standalone) {
        struct DRInBuildMeta * meta;

        if (dr_inbuild_metalib_find_meta(builder, dr_meta_name(entry_meta))) return 0;

        meta = dr_inbuild_metalib_add_meta(builder);
        if (meta == NULL) {
            CPE_ERROR(em, "pom_grp_meta_build_store_meta: create meta for %s fail!", entry->m_name);
            return -1;
        }

        if (dr_inbuild_meta_init(meta, entry_meta) != 0
            || dr_inbuild_meta_copy_key_entrys(meta, src_main_meta) != 0
            || dr_inbuild_meta_copy_entrys(meta, entry_meta) != 0
            || dr_inbuild_meta_copy_keys(meta, src_main_meta) != 0
            )
        {
            CPE_ERROR(em, "pom_grp_meta_build_store_meta: setup meta for %s fail!", entry->m_name);
            return -1;
        }
    }
    else {
        struct DRInBuildMetaEntry * new_entry;
        size_t countNameLen;
        char * countNameBuf;
        /*count*/
        new_entry = dr_inbuild_meta_add_entry(main_meta);
        if (new_entry == NULL) {
            CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %sCount fail!", entry->m_name);
            return -1;
        }

        countNameLen = strlen(entry->m_name) + 1 + strlen("Count");
        countNameBuf = mem_buffer_alloc(str_buffer, countNameLen);
        if (countNameBuf == NULL) {
            CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %sCount fail!", entry->m_name);
            return -1;
        }
        snprintf(countNameBuf, countNameLen, "%sCount", entry->m_name);
        dr_inbuild_entry_set_name(new_entry, countNameBuf);
        dr_inbuild_entry_set_id(new_entry, -1);
        dr_inbuild_entry_set_type(new_entry, "uint32");
        dr_inbuild_entry_set_array_count(new_entry, 1);


        /*entry*/
        if (dr_inbuild_metalib_find_meta(builder, dr_meta_name(entry_meta)) == NULL) {
            if (dr_inbuild_metalib_copy_meta(builder, entry_meta) == NULL) {
                CPE_ERROR(em, "pom_grp_meta_build_store_meta: create meta for %s fail!", entry->m_name);
                return -1;
            }
        }


        new_entry = dr_inbuild_meta_add_entry(main_meta);
        if (new_entry == NULL) {
            CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %s fail!", entry->m_name);
            return -1;
        }

        dr_inbuild_entry_set_name(new_entry, entry->m_name);
        dr_inbuild_entry_set_id(new_entry, -1);
        dr_inbuild_entry_set_type(new_entry, dr_meta_name(entry_meta));
        dr_inbuild_entry_set_array_count(new_entry, pom_grp_entry_meta_list_capacity(entry));
        dr_inbuild_entry_set_array_refer(new_entry, countNameBuf);
    }

    return 0;
}

static int pom_grp_meta_build_calc_version(
    struct DRInBuildMetaLib * builder)
{
    return 1;
}

static int pom_grp_meta_build_store_meta_list(
    struct DRInBuildMetaLib * builder,
    pom_grp_meta_t meta,
    LPDRMETA src_main_meta,
    error_monitor_t em)
{
    struct DRInBuildMeta * list_meta;
    struct DRInBuildMetaEntry * count_entry;
    struct DRInBuildMetaEntry * data_entry;
    char name_buf[128];

    list_meta = dr_inbuild_metalib_add_meta(builder);
    if (list_meta == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create list meta fail!");
        return -1;
    }

    dr_inbuild_meta_set_current_version(list_meta, dr_meta_current_version(src_main_meta));

    snprintf(name_buf, sizeof(name_buf), "%sList", pom_grp_meta_name(meta));
    dr_inbuild_meta_set_name(list_meta, name_buf);
    dr_inbuild_meta_set_current_version(list_meta, dr_meta_current_version(src_main_meta));
    dr_inbuild_meta_set_align(list_meta, dr_meta_align(src_main_meta));
    dr_inbuild_meta_set_type(list_meta, CPE_DR_TYPE_STRUCT);

    count_entry = dr_inbuild_meta_add_entry(list_meta);
    if (count_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create list meta count entry fail!");
        return -1;
    }
    dr_inbuild_entry_set_name(count_entry, "count");
    dr_inbuild_entry_set_version(count_entry, 1);
    dr_inbuild_entry_set_type(count_entry, "uint32");
    dr_inbuild_entry_set_array_count(count_entry, 1);

    data_entry = dr_inbuild_meta_add_entry(list_meta);
    if (data_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create list meta data entry fail!");
        return -1;
    }
    dr_inbuild_entry_set_name(data_entry, "data");
    dr_inbuild_entry_set_version(data_entry, 1);

    snprintf(name_buf, sizeof(name_buf), "%s", pom_grp_meta_name(meta));
    dr_inbuild_entry_set_type(data_entry, name_buf);
    dr_inbuild_entry_set_array_count(data_entry, 0);
    dr_inbuild_entry_set_array_refer(data_entry, "count");

    return 0;
}

static int pom_grp_meta_build_store_meta_i(
    struct DRInBuildMetaLib * builder,
    pom_grp_meta_t meta,
    mem_buffer_t str_buffer,
    error_monitor_t em)
{
    struct DRInBuildMeta * main_meta;
    LPDRMETALIB src_metalib;
    LPDRMETA src_main_meta;
    uint16_t i, count;

    if (meta->m_main_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: no main entry!");
        return -1;
    }

    if (meta->m_main_entry->m_type != pom_grp_entry_type_normal) {
        CPE_ERROR(
            em, "pom_grp_meta_build_store_meta: main entry %s is not type normal!",
            meta->m_main_entry->m_name);
        return -1;
    }

    src_main_meta = meta->m_main_entry->m_data.m_normal.m_data_meta;
    src_metalib = dr_meta_owner_lib(src_main_meta);

    main_meta = dr_inbuild_metalib_add_meta(builder);
    if (main_meta == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create main meta fail!");
        return -1;
    }
    dr_inbuild_meta_set_current_version(
        main_meta, dr_meta_current_version(src_main_meta));

    dr_inbuild_metalib_set_tagsetversion(builder, dr_lib_tag_set_version(src_metalib));
    dr_inbuild_metalib_set_name(builder, pom_grp_meta_name(meta));

    dr_inbuild_meta_set_name(main_meta, pom_grp_meta_name(meta));
    dr_inbuild_meta_set_current_version(main_meta, dr_meta_current_version(src_main_meta));
    dr_inbuild_meta_set_align(main_meta, dr_meta_align(src_main_meta));
    dr_inbuild_meta_set_id(main_meta, dr_meta_id(src_main_meta));
    dr_inbuild_meta_set_desc(main_meta, dr_meta_desc(src_main_meta));
    dr_inbuild_meta_set_type(main_meta, dr_meta_type(src_main_meta));

    if (dr_inbuild_meta_copy_key_entrys(main_meta, src_main_meta) != 0
        || dr_inbuild_meta_copy_keys(main_meta, src_main_meta) != 0)
    {
        return -1;
    }

    count = pom_grp_meta_entry_count(meta);
    for(i = 0; i < count; ++i) {
        pom_grp_entry_meta_t entry_meta = pom_grp_meta_entry_at(meta, i);
        switch(entry_meta->m_type) {
        case pom_grp_entry_type_normal:
            if (pom_grp_meta_build_store_meta_on_entry_normal(builder, main_meta, entry_meta, str_buffer, em) != 0) return -1;
            break;
        case pom_grp_entry_type_list:
            if (pom_grp_meta_build_store_meta_on_entry_list(builder, main_meta, src_main_meta, entry_meta, str_buffer, em) != 0) return -1;
            break;
        case pom_grp_entry_type_ba:
            if (pom_grp_meta_build_store_meta_on_entry_ba(main_meta, entry_meta, str_buffer, em) != 0) return -1;
            break;
        case pom_grp_entry_type_binary:
            if (pom_grp_meta_build_store_meta_on_entry_binary(main_meta, entry_meta, str_buffer, em) != 0) return -1;
            break;
        }
    }

    if (pom_grp_meta_build_store_meta_list(builder, meta, src_main_meta, em) != 0) {
        return -1;
    }

    dr_inbuild_metalib_set_version(builder, pom_grp_meta_build_calc_version(builder));

    return 0;
}

int pom_grp_meta_build_store_meta(mem_buffer_t buffer, pom_grp_meta_t meta, error_monitor_t em) {
    struct mem_buffer str_buffer;

    struct DRInBuildMetaLib * builder = dr_inbuild_create_lib();
    if (builder == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create metalib builder fail!");
        return -1;
    }
    mem_buffer_init(&str_buffer, NULL);

    if (pom_grp_meta_build_store_meta_i(builder, meta, &str_buffer, em) != 0) {
        goto BUILD_STORE_META_ERROR;
    }
    
    if (dr_inbuild_tsort(builder, em) != 0) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: sort lib fail!");
        goto BUILD_STORE_META_ERROR;
    }

    if (dr_inbuild_build_lib(buffer, builder, em) != 0) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: build lib to buffer fail!");
        goto BUILD_STORE_META_ERROR;
    }

    dr_inbuild_free_lib(builder);
    mem_buffer_clear(&str_buffer);
    return 0;

BUILD_STORE_META_ERROR:
    dr_inbuild_free_lib(builder);
    mem_buffer_clear(&str_buffer);
    return -1;
}
