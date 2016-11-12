#include <stdlib.h>
#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/tsort.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_yaml.h"
#include "cpe/dr/dr_types.h"
#include "../dr_ctype_ops.h"
#include "dr_metalib_ops.h"

static int dr_inbuild_calc_lib_paras(
    struct DRInBuildMetaLib * inBuildLib, error_monitor_t em);

struct DRInBuildCreateCtx {
    LPDRMETALIB m_metaLib;
    error_monitor_t m_em;
    int m_stringUsedCount;
};

static int dr_inbuild_build_add_string(struct DRInBuildCreateCtx * ctx, const char * data) {
    size_t dataSize;
    void * p;

    if (data == NULL || data[0] == 0) {
        return -1;
    }

    dataSize = strlen(data) + 1;

    p = dr_lib_alloc_in_strbuf(ctx->m_metaLib, dataSize, &ctx->m_stringUsedCount, ctx->m_em);
    if (p == NULL) {
        return -1;
    }

    cpe_str_dup((char*)p, dataSize, data);

    return dr_lib_addr_to_pos(ctx->m_metaLib, p);
}

static void dr_inbuild_build_add_macro(
    struct DRInBuildCreateCtx * ctx,
    struct DRInBuildMacro * macroEle)
{
    macroEle->m_data.m_name_pos = dr_inbuild_build_add_string(ctx, macroEle->m_name);
    macroEle->m_data.m_desc_pos = dr_inbuild_build_add_string(ctx, macroEle->m_desc);
    dr_lib_add_macro(ctx->m_metaLib, &macroEle->m_data, ctx->m_em);
}

static void dr_inbuild_build_calc_entry_defaultvalue(
    struct DRInBuildCreateCtx * ctx, LPDRMETA createdMeta, struct DRInBuildMetaEntry * entryEle)
{
    void * p;

    if (entryEle->m_dft_value == NULL || entryEle->m_data.m_unitsize <= 0) {
        return;
    }

    if (entryEle->m_data.m_type == CPE_DR_TYPE_UNION) return;

    p = dr_lib_alloc_in_strbuf(ctx->m_metaLib, entryEle->m_data.m_unitsize, &ctx->m_stringUsedCount, ctx->m_em);
    if (p == NULL) {
        return;
    }

    if (entryEle->m_data.m_type == CPE_DR_TYPE_STRUCT) {
        bzero(p, entryEle->m_data.m_unitsize);
    }
    else {
        struct DRInBuildMacro * macro = NULL;

        if (entryEle->m_data.m_type != CPE_DR_TYPE_STRING) {
            macro = dr_inbuild_metalib_find_macro(entryEle->m_meta->m_lib, entryEle->m_dft_value);
        }

        if (macro) {
            if (dr_entry_set_from_int32(p, macro->m_data.m_value, &entryEle->m_data, ctx->m_em) != 0) {
                return;
            }
        }
        else {
            if (dr_entry_set_from_string(p, entryEle->m_dft_value, &entryEle->m_data, ctx->m_em) != 0) {
                return;
            }
        }
    }

    entryEle->m_data.m_dft_value_pos = dr_lib_addr_to_pos(ctx->m_metaLib, p);
}

static void dr_inbuild_build_calc_entry_composite_type(
    struct DRInBuildCreateCtx * ctx, LPDRMETA createdMeta, struct DRInBuildMetaEntry * entryEle)
{
    LPDRMETA refMeta;
    if (entryEle->m_data.m_type > CPE_DR_TYPE_COMPOSITE) return;

    refMeta = dr_lib_find_meta_by_name(ctx->m_metaLib, entryEle->m_ref_type_name);
    if (refMeta) {
        char * base = (char*)(createdMeta) - createdMeta->m_self_pos;
        assert(entryEle->m_data.m_type == refMeta->m_type);
        entryEle->m_data.m_ref_type_pos = (int32_t)((char*)refMeta - (char*)base);
    }
    else {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_ENTRY_NO_TYPE, "ref type \"%s\" is unknown!", entryEle->m_ref_type_name);
        entryEle->m_ignore = 1;
    }
}

static void dr_inbuild_build_calc_entry_select(
    struct DRInBuildCreateCtx * ctx, LPDRMETA createdMeta, struct DRInBuildMetaEntry * entryEle)
{
    LPDRMETAENTRY selectEntry;

    if (entryEle->m_selector_path == NULL) {
        return;
    }

    selectEntry = dr_meta_find_entry_by_path(createdMeta, entryEle->m_selector_path);
    if (selectEntry == NULL) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_ENTRY_INVALID_SELECT_VALUE, "entry at %s not exist!", entryEle->m_selector_path);
    }
    else {
        char * base = (char*)(createdMeta) - createdMeta->m_self_pos;
        entryEle->m_data.m_select_entry_pos = (int32_t)((char *)selectEntry - base);
        entryEle->m_data.m_select_data_start_pos = dr_meta_path_to_off(createdMeta, entryEle->m_selector_path, NULL);
    }
}

static void dr_inbuild_build_calc_array_refer(
    struct DRInBuildCreateCtx * ctx, LPDRMETA createdMeta, struct DRInBuildMetaEntry * entryEle)
{
    LPDRMETAENTRY referEntry;

    if (entryEle->m_refer_path == NULL) {
        if (entryEle->m_data.m_array_count == 0) {
            CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_VARIABLE_ARRAY_NO_REFER, "entry is dynamic array, but no reffer!");
        }
        return;
    }

    if (entryEle->m_data.m_array_count == 1) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_ENTRY_INVALID_REFER_VALUE, "entry is not array, but config a reffer!");
        return;
    }

    referEntry = dr_meta_find_entry_by_path(createdMeta, entryEle->m_refer_path);
    if (referEntry == NULL) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_ENTRY_INVALID_REFER_VALUE, "entry at %s not exist!", entryEle->m_refer_path);
    }
    else {
        char * base = (char*)(createdMeta) - createdMeta->m_self_pos;
        entryEle->m_data.m_array_refer_entry_pos = (int32_t)((char *)referEntry - base);
        entryEle->m_data.m_array_refer_data_start_pos = dr_meta_path_to_off(createdMeta, entryEle->m_refer_path, NULL);
    }
}

static void dr_inbuild_build_add_entry(
    struct DRInBuildCreateCtx * ctx,
    LPDRMETA createdMeta,
    struct DRInBuildMetaEntry * entryEle)
{
    entryEle->m_data.m_name_pos = dr_inbuild_build_add_string(ctx, entryEle->m_name);
    entryEle->m_data.m_desc_pos = dr_inbuild_build_add_string(ctx, entryEle->m_desc);
    entryEle->m_data.m_cname_pos = dr_inbuild_build_add_string(ctx, entryEle->m_cname);

    dr_inbuild_build_calc_entry_defaultvalue(ctx, createdMeta, entryEle);
    dr_inbuild_build_calc_entry_composite_type(ctx, createdMeta, entryEle) ;
    dr_inbuild_build_calc_entry_select(ctx, createdMeta, entryEle);
    dr_inbuild_build_calc_array_refer(ctx, createdMeta, entryEle);

    if (!entryEle->m_ignore) {
        dr_meta_add_entry(createdMeta, &entryEle->m_data, ctx->m_em);
    }
}

static void dr_inbuild_build_add_meta(
    struct DRInBuildCreateCtx * ctx,
    struct DRInBuildMeta * metaEle)
{
    LPDRMETA createdMeta = 0;
    struct DRInBuildMetaEntry * entryEle = 0;
    struct dr_inbuild_key_entry * key_entry;
    struct dr_inbuild_index * index;
    struct dr_inbuild_index_entry * index_entry;

    metaEle->m_data.m_name_pos = dr_inbuild_build_add_string(ctx, metaEle->m_name);
    metaEle->m_data.m_desc_pos =dr_inbuild_build_add_string(ctx, metaEle->m_desc);

    createdMeta = dr_lib_add_meta(ctx->m_metaLib, &metaEle->m_data, ctx->m_em);
    if (createdMeta == NULL) return;

    /*build entries*/
    TAILQ_FOREACH(entryEle, &metaEle->m_entries, m_next) {
        dr_inbuild_build_add_entry(ctx, createdMeta, entryEle);
    }

    TAILQ_FOREACH(key_entry, &metaEle->m_key_entries, m_next) {
        dr_meta_add_key(createdMeta, key_entry->m_entry_name, ctx->m_em);
    }

    TAILQ_FOREACH(index, &metaEle->m_indexes, m_next) {
        struct dr_index_info index_data;
        struct dr_index_info * created_index;

        bzero(&index_data, sizeof(index_data));
        index_data.m_name_pos = dr_inbuild_build_add_string(ctx, index->m_index_name);

        created_index = dr_meta_add_index(createdMeta, &index_data, ctx->m_em);
        if (created_index == NULL) continue;

        TAILQ_FOREACH(index_entry, &index->m_entries, m_next) {
            dr_index_add_entry(created_index, index_entry->m_entry_name, ctx->m_em);
        }
    }

    dr_meta_do_complete(createdMeta, ctx->m_em);
}

int dr_inbuild_build_lib(
    mem_buffer_t buffer,
    struct DRInBuildMetaLib * inBuildLib,
    error_monitor_t em)
{
    struct DRInBuildCreateCtx ctx;
    struct DRInBuildMacro * macroEle = 0;
    struct DRInBuildMeta * metaEle = 0;
    int ret = 0;

    ctx.m_stringUsedCount = 0;
    ctx.m_metaLib = NULL;
    ctx.m_em = em;

    ret = dr_inbuild_calc_lib_paras(inBuildLib, em);
    if (CPE_IS_ERROR(ret)) {
        return ret;
    }

    ctx.m_metaLib = (LPDRMETALIB)mem_buffer_alloc(buffer, inBuildLib->m_data.iSize);
    if (ctx.m_metaLib == NULL) {
        return -1;
    }

    ret = dr_lib_init(ctx.m_metaLib, &inBuildLib->m_data);
    if (CPE_IS_ERROR(ret)) {
        return ret;
    }

    /*build macros*/
    TAILQ_FOREACH(macroEle, &inBuildLib->m_macros, m_next) {
        dr_inbuild_build_add_macro(&ctx, macroEle);
    }

    /*build metas*/
    TAILQ_FOREACH(metaEle, &inBuildLib->m_metas, m_next) {
        dr_inbuild_build_add_meta(&ctx, metaEle);
    }

    /*setup composite default values*/
    TAILQ_FOREACH(metaEle, &inBuildLib->m_metas, m_next) {
        struct DRInBuildMetaEntry * entryEle = 0;
        TAILQ_FOREACH(entryEle, &metaEle->m_entries, m_next) {
            if (entryEle->m_ignore) continue;
            if (entryEle->m_dft_value && entryEle->m_data.m_type <= CPE_DR_TYPE_COMPOSITE) {
                LPDRMETA meta = dr_lib_find_meta_by_name(ctx.m_metaLib, metaEle->m_name);
                LPDRMETAENTRY entry = dr_meta_find_entry_by_name(meta, entryEle->m_name);
                void * dv = (void*)dr_entry_dft_value(entry); 

                assert(dv);

                if (dr_yaml_read(dv, entryEle->m_data.m_unitsize, entryEle->m_dft_value, dr_entry_ref_meta(entry), ctx.m_em) < 0) {
                    CPE_ERROR(
                        ctx.m_em, "%s.%s(%s) set default value from %s fail!",
                        metaEle->m_name, entryEle->m_name, dr_meta_name(dr_entry_ref_meta(entry)), entryEle->m_dft_value);
                }
            }
        }
    }
    
    return CPE_SUCCESS;
}

static int dr_inbuild_build_calc_defaultvalue_size(
    struct DRInBuildMetaLib * inBuildLib, error_monitor_t em)
{
    struct DRInBuildMeta * metaEle = 0;

    int totalSize = 0;

    TAILQ_FOREACH(metaEle, &inBuildLib->m_metas, m_next) {
        struct DRInBuildMetaEntry * entryEle = 0;
        TAILQ_FOREACH(entryEle, &metaEle->m_entries, m_next) {
            if (entryEle->m_ignore) {
                continue;
            }

            if (entryEle->m_dft_value) {
                if (entryEle->m_data.m_unitsize <= 0) {
                    entryEle->m_dft_value = NULL;
                    CPE_WARNING(em,
                        "ignore default value of %s.%s!", metaEle->m_name, entryEle->m_name);
                }
                else {
                    totalSize += entryEle->m_data.m_unitsize;
                }
            }
        }
    }

    return totalSize;
}

static void dr_inbuild_build_calc_type_size_align_meta(struct DRInBuildMeta * metaEle, error_monitor_t em) {
    struct DRInBuildMetaEntry * entryEle = 0;
    
    if (metaEle->m_processed) return;
    metaEle->m_processed = 1;
    
    metaEle->m_data.m_require_align = 1;

    assert(metaEle->m_data.m_real_data_size == 0);
    assert(metaEle->m_data.m_align > 0);

    TAILQ_FOREACH(entryEle, &metaEle->m_entries, m_next) {
        const struct tagDRCTypeInfo * ctypeInfo = NULL;
        uint16_t element_align;

        ctypeInfo = dr_find_ctype_info_by_name(entryEle->m_ref_type_name);
        if (ctypeInfo) {
            entryEle->m_data.m_type = ctypeInfo->m_id;
            element_align = ((int)ctypeInfo->m_size) > 0 ? ctypeInfo->m_size : 1;

            if ((int)ctypeInfo->m_size > 0) {
                entryEle->m_data.m_unitsize =
                    (int32_t)(ctypeInfo->m_size * ( entryEle->m_data.m_array_count < 1 ? 1 : entryEle->m_data.m_array_count));
            }
            else if (entryEle->m_data.m_size > 0) {
                entryEle->m_data.m_unitsize =
                    entryEle->m_data.m_size * ( entryEle->m_data.m_array_count < 1 ? 1 : entryEle->m_data.m_array_count);
            }
            else {
                CPE_ERROR_EX(
                    em, CPE_DR_ERROR_ENTRY_INVALID_SIZE_VALUE,
                    "%s.%s: no size of  type \"%s\" configured!",
                    metaEle->m_name, entryEle->m_name, entryEle->m_ref_type_name);
                entryEle->m_ignore = 1;
                continue;
            }
        }
        else {
            struct DRInBuildMeta * ref_meta = dr_inbuild_metalib_find_meta(metaEle->m_lib, entryEle->m_ref_type_name);
            if (ref_meta == NULL) {
                CPE_ERROR_EX(
                    em, CPE_DR_ERROR_ENTRY_INVALID_SIZE_VALUE,
                    "%s.%s: ref type \"%s\" not exist!",
                    metaEle->m_name, entryEle->m_name, entryEle->m_ref_type_name);
                entryEle->m_ignore = 1;
                continue;
            }

            if (!ref_meta->m_processed) {
                CPE_ERROR_EX(
                    em, CPE_DR_ERROR_ENTRY_INVALID_SIZE_VALUE,
                    "%s.%s: ref type \"%s\" not processed!",
                    metaEle->m_name, entryEle->m_name, entryEle->m_ref_type_name);
                entryEle->m_ignore = 1;
                continue;
            }

            element_align = ref_meta->m_data.m_require_align;
            
            if (ref_meta->m_data.m_real_data_size == 0) {
                entryEle->m_ignore = 1;
                continue;
            }

            entryEle->m_data.m_type = ref_meta->m_data.m_type;
            entryEle->m_data.m_unitsize = ref_meta->m_data.m_real_data_size;
            if (entryEle->m_data.m_unitsize % element_align) {
                entryEle->m_data.m_unitsize = ((entryEle->m_data.m_unitsize / element_align) + 1) * element_align;
            }

            if (entryEle->m_data.m_array_count > 1) {
                entryEle->m_data.m_unitsize *= entryEle->m_data.m_array_count;
            }

            assert(ref_meta->m_data.m_align > 0);
        }

        assert(!entryEle->m_ignore);

        if (element_align > metaEle->m_lib->m_dft_align) element_align = metaEle->m_lib->m_dft_align;
        
        if (element_align <= metaEle->m_data.m_align && element_align > metaEle->m_data.m_require_align) {
            metaEle->m_data.m_require_align = element_align;
        }
               
        if (metaEle->m_data.m_type == CPE_DR_TYPE_STRUCT) {
            uint16_t align = element_align < metaEle->m_data.m_align ? element_align : metaEle->m_data.m_align;
            uint16_t left = ((uint32_t)metaEle->m_data.m_real_data_size) % align;
            uint16_t panding = 0;

            if (left) panding = element_align - left;

            entryEle->m_data.m_data_start_pos = metaEle->m_data.m_real_data_size + panding;
            metaEle->m_data.m_real_data_size += panding + entryEle->m_data.m_unitsize;
        }
        else {
            assert(metaEle->m_data.m_type == CPE_DR_TYPE_UNION);
            
            entryEle->m_data.m_data_start_pos = 0;
            if (metaEle->m_data.m_real_data_size < entryEle->m_data.m_unitsize) {
                metaEle->m_data.m_real_data_size = entryEle->m_data.m_unitsize;
            }
        }
    }

    if (metaEle->m_data.m_real_data_size % metaEle->m_data.m_require_align) {
        metaEle->m_data.m_real_data_size = ((metaEle->m_data.m_real_data_size / metaEle->m_data.m_require_align) + 1) * metaEle->m_data.m_require_align;
    }
}

static void dr_inbuild_build_calc_type_size_align(struct DRInBuildMetaLib * inBuildLib, error_monitor_t em) {
    struct DRInBuildMeta * metaEle = 0;

    TAILQ_FOREACH(metaEle, &inBuildLib->m_metas, m_next) {
        dr_inbuild_build_calc_type_size_align_meta(metaEle, em);
    }
}

static int dr_inbuild_calc_string_size(const char * data) {
    return data
        ? (int)strlen(data) + 1
        : 0;
}

static int dr_inbuild_calc_strbuf_size(struct DRInBuildMetaLib * inBuildLib) {
    struct DRInBuildMacro * macroEle = 0;
    struct DRInBuildMeta * metaEle = 0;
    struct DRInBuildMetaEntry * entryEle = 0;
    struct dr_inbuild_index * index;

    int strBufSize = 0;

    TAILQ_FOREACH(macroEle, &inBuildLib->m_macros, m_next) {
        strBufSize += dr_inbuild_calc_string_size(macroEle->m_name);
        strBufSize += dr_inbuild_calc_string_size(macroEle->m_desc);
    }

    TAILQ_FOREACH(metaEle, &inBuildLib->m_metas, m_next) {
        strBufSize += dr_inbuild_calc_string_size(metaEle->m_name);
        strBufSize += dr_inbuild_calc_string_size(metaEle->m_desc);

        TAILQ_FOREACH(entryEle, &metaEle->m_entries, m_next) {
            strBufSize += dr_inbuild_calc_string_size(entryEle->m_name);
            strBufSize += dr_inbuild_calc_string_size(entryEle->m_desc);
            strBufSize += dr_inbuild_calc_string_size(entryEle->m_cname);
        }

        TAILQ_FOREACH(index, &metaEle->m_indexes, m_next) {
            strBufSize += dr_inbuild_calc_string_size(index->m_index_name);
        }
    }

    return strBufSize;
}

int dr_inbuild_calc_lib_paras(
    struct DRInBuildMetaLib * inBuildLib, error_monitor_t em)
{
    struct DRInBuildMacro * macroEle = 0;
    struct DRInBuildMeta * metaEle = 0;
    struct dr_inbuild_index * index;

    dr_inbuild_build_calc_type_size_align(inBuildLib, em);

    inBuildLib->m_data.iMaxMacros = 0;
    TAILQ_FOREACH(macroEle, &inBuildLib->m_macros, m_next) {
        ++inBuildLib->m_data.iMaxMacros;
    }

    inBuildLib->m_data.iMetaSize = 0;
    inBuildLib->m_data.iMaxMetas = 0;
    TAILQ_FOREACH(metaEle, &inBuildLib->m_metas, m_next) {
        int index_entry_use_size;

        metaEle->m_data.m_entry_count = metaEle->m_entries_count;

        metaEle->m_data.m_key_start_from_meta = 
            sizeof(struct tagDRMeta)
            + sizeof(struct tagDRMetaEntry) * metaEle->m_entries_count;

        metaEle->m_data.m_index_pos_from_meta =
            metaEle->m_data.m_key_start_from_meta
            + sizeof(struct dr_idx_entry_info) * metaEle->m_key_entrie_count;

        metaEle->m_data.m_index_entry_pos_from_meta =
            metaEle->m_data.m_index_pos_from_meta
            + sizeof(struct dr_index_info) * metaEle->m_index_count;
            
        index_entry_use_size = 0;
        TAILQ_FOREACH(index, &metaEle->m_indexes, m_next) {
            index_entry_use_size += sizeof(struct dr_index_entry_info) * index->m_entry_count;
        }

        metaEle->m_data.m_meta_size = 
            metaEle->m_data.m_index_entry_pos_from_meta
            + index_entry_use_size;

        inBuildLib->m_data.iMaxMetas++;
        inBuildLib->m_data.iMetaSize += metaEle->m_data.m_meta_size;
    }

    inBuildLib->m_data.iMaxMacrosGroupNum = 0;
    inBuildLib->m_data.iMacrosGroupSize = 0;

    inBuildLib->m_data.iStrBufSize =
        dr_inbuild_calc_strbuf_size(inBuildLib)
        + dr_inbuild_build_calc_defaultvalue_size(inBuildLib, em);

    inBuildLib->m_data.iSize
        = sizeof(struct tagDRMetaLib)                                    /*head*/
        + sizeof(struct tagDRMacro) * inBuildLib->m_data.iMaxMacros /*macros*/
        + (sizeof(struct idx_meta_by_id)                               /*meta indexes*/
           + sizeof(struct idx_meta_by_name)
           + sizeof(struct idx_meta_by_orig)
            ) * inBuildLib->m_data.iMaxMetas
        + inBuildLib->m_data.iMetaSize                              /*metas*/
        + inBuildLib->m_data.iMacrosGroupSize                       /*macro group*/
        + inBuildLib->m_data.iStrBufSize                            /*strings*/
        ;

    return CPE_SUCCESS;
}

void dr_inbuild_set_dft_align(struct DRInBuildMetaLib * inBuildMetaLib, uint8_t align) {
    inBuildMetaLib->m_dft_align = align ? align : CPE_DEFAULT_ALIGN;
}

int dr_inbuild_tsort(
    struct DRInBuildMetaLib * inBuildLib,
    error_monitor_t er)
{
    tsorter_str_t tsorter;
    struct tsorter_str_it tsorter_it;
    const char * meta_name;

    tsorter = tsorter_str_create(NULL, 0);
    if (tsorter == NULL) {
        CPE_ERROR(er, "dr_inbuild_tsort: alloc tsorter fail!");
        return -1;
    }

    while(!TAILQ_EMPTY(&inBuildLib->m_metas)) {
        struct DRInBuildMeta * metaEle = TAILQ_FIRST(&inBuildLib->m_metas);
        struct DRInBuildMetaEntry * entryEle;

        if (metaEle->m_name == NULL) {
            dr_inbuild_metalib_remove_meta(inBuildLib, metaEle);
            continue;
        }

        TAILQ_REMOVE(&inBuildLib->m_metas, metaEle, m_next);

        if (tsorter_str_add_element(tsorter, metaEle->m_name) != 0) {
            CPE_ERROR(er, "dr_inbuild_tsort: add element %s fail!", metaEle->m_name);
        }

        TAILQ_FOREACH(entryEle, &metaEle->m_entries, m_next) {
            if (dr_find_ctype_info_by_name(entryEle->m_ref_type_name) == NULL) {
                if (tsorter_str_add_dep(tsorter, metaEle->m_name, entryEle->m_ref_type_name) != 0) {
                    CPE_ERROR(
                        er, "dr_inbuild_tsort: add depend %s ==> %s fail!",
                        metaEle->m_name, entryEle->m_ref_type_name);
                }
            }
        }
    }

    if (tsorter_str_sort(&tsorter_it, tsorter) != 0) {
        CPE_ERROR(
            er, "dr_inbuild_tsort: sort fail, maby have circle!");
    }

    while((meta_name = tsorter_str_next(&tsorter_it))) {
        struct DRInBuildMeta * metaEle = dr_inbuild_metalib_find_meta(inBuildLib, meta_name);
        if (metaEle == NULL) {
            CPE_ERROR(
                er, "dr_inbuild_tsort: meta %s not exist!", meta_name);
            continue;
        }

        TAILQ_INSERT_TAIL(&inBuildLib->m_metas, metaEle, m_next);
    }

    tsorter_str_free(tsorter);
    return 0;
}
