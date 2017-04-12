#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stackbuf.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_build.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"
#include "dr_inbuild_types.h"
#include "dr_inbuild_error.h"
#include "dr_builder_ops.h"

enum DRXmlParseState {
    PS_Init
    , PS_InMetaLib
    , PS_InMeta
    , PS_Error
};

#define DR_DO_READ_INT(__t, __d, __e)                                  \
    do {                                                                \
        char * buf = cpe_str_dup_len(                                   \
            ctx->m_data_buf, sizeof(ctx->m_data_buf),                   \
            (const char *)valueBegin, len);                             \
        if (buf == NULL) {                                              \
            if (__e) {                                                  \
                DR_NOTIFY_ERROR(ctx->m_em, (__e));                      \
            }                                                           \
            return;                                                     \
        }                                                               \
        else { char * endptr = 0;                                       \
            (__d) = (__t)strtol(buf, &endptr, 10);                      \
            if ( !endptr || *endptr != 0) {                             \
                if (__e) {                                              \
                    DR_NOTIFY_ERROR(ctx->m_em, (__e));                  \
                }                                                       \
            }                                                           \
        }                                                               \
    } while(0)


#define DR_DO_READ_INT_OR_MACRO(__t, __d, __e)                           \
    do {                                                                \
        char * buf = cpe_str_dup_len(                                   \
            ctx->m_data_buf, sizeof(ctx->m_data_buf),                   \
            (const char *)valueBegin, len);                             \
        if (buf == NULL) {                                              \
            if (__e) { DR_NOTIFY_ERROR(ctx->m_em, (__e)); }             \
            return;                                                     \
        }                                                               \
        else { char * endptr = 0;                                       \
            (__d) = (__t)strtol(buf, &endptr, 10);                      \
            if ( !endptr || *endptr != 0) {                             \
                struct DRInBuildMacro * macro =                         \
                    dr_inbuild_metalib_find_macro(ctx->m_metaLib, buf); \
                if (macro) {                                            \
                    __d = (__t)macro->m_data.m_value;                   \
                }                                                       \
                else {                                                  \
                    DR_NOTIFY_ERROR_EXTRA(ctx->m_em, CPE_DR_ERROR_UNDEFINED_MACRO_NAME, ctx->m_data_buf); \
                }                                                       \
            }                                                           \
        }                                                               \
    } while(0)

#define DR_DO_DUP_STR(buf)                                              \
    buf =  mem_buffer_strdup_len(                                       \
        &ctx->m_metaLib->m_tmp_buf, (char const *)valueBegin, len);

struct DRXmlParseCtx {
    dr_metalib_source_t m_source;
    struct DRInBuildMetaLib * m_metaLib;
    struct DRInBuildMeta * m_curentMeta;
    enum DRXmlParseState m_state;
    error_monitor_t m_em;
    char m_data_buf[256];
};

static void dr_build_xml_process_metalib(
    struct DRXmlParseCtx * ctx,
    int nb_attributes,
    const xmlChar** attributes)
{
    int indexAttribute = 0;
    int index = 0;
    int haveVersion = 0;

    if (ctx->m_state != PS_Init) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_XML_PARSE, "error metalib tag!");
        ctx->m_state = PS_Error;
        return;
    }

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const xmlChar *localname = attributes[index];
        /*const xmlChar *prefix = attributes[index+1];*/
        /*const xmlChar *nsURI = attributes[index+2];*/
        const xmlChar *valueBegin = attributes[index+3];
        const xmlChar *valueEnd = attributes[index+4];

        int len = (int)(valueEnd - valueBegin);

        if (strcmp((char const *)localname, "name") == 0) {
            if (cpe_str_dup_len(ctx->m_metaLib->m_data.szName, CPE_DR_NAME_LEN, (char const *)valueBegin, len) == NULL) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
                return;
            }
        }
        else if (strcmp((char const *)localname, "tagsetversion") == 0) {
            DR_DO_READ_INT(int, ctx->m_metaLib->m_data.iTagSetVersion, CPE_DR_ERROR_INVALID_TAGSET_VERSION);
        }
        else if (strcmp((char const *)localname, "version") == 0) {
            haveVersion = 1;
            DR_DO_READ_INT(int, ctx->m_metaLib->m_data.iVersion, CPE_DR_ERROR_INVALID_VERSION);
        }
        else {
        }
    }

    if (ctx->m_metaLib->m_data.iTagSetVersion != 1) {
        CPE_ERROR_EX(
            ctx->m_em,
            CPE_DR_ERROR_INVALID_TAGSET_VERSION,
            "unknown tagsetversion version, only support 1!");

        ctx->m_state = PS_Error;
    }

    if (!haveVersion || ctx->m_metaLib->m_data.iVersion < 0) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NO_VERSION);
    }

    if (ctx->m_source) {
        dr_metalib_source_element_create(
            ctx->m_source, dr_metalib_source_element_type_lib, ctx->m_metaLib->m_data.szName);
    }

    ctx->m_state = PS_InMetaLib;
}

static void dr_build_xml_process_macro(
    struct DRXmlParseCtx * ctx,
    int nb_attributes,
    const xmlChar** attributes)
{
    int indexAttribute = 0;
    int index = 0;
    int haveValue = 0;
    struct DRInBuildMacro * newMacro;
    int haveError = 0;

    if (ctx->m_state != PS_InMetaLib) {
        return;
    }

    newMacro = dr_inbuild_metalib_add_macro(ctx->m_metaLib);
    if (newMacro == NULL) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NO_MEMORY);
        return;
    }

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes && !haveError;
        ++indexAttribute, index += 5)
    {
        const xmlChar *localname = attributes[index];
        /*const xmlChar *prefix = attributes[index+1];*/
        /*const xmlChar *nsURI = attributes[index+2];*/
        const xmlChar *valueBegin = attributes[index+3];
        const xmlChar *valueEnd = attributes[index+4];

        int len = (int)(valueEnd - valueBegin);

        if (strcmp((char const *)localname, "name") == 0) {
            if (len > CPE_DR_MACRO_LEN) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
                return;
            }

            DR_DO_DUP_STR(newMacro->m_name);

            if (newMacro->m_name) {
                if (dr_inbuild_metalib_add_macro_to_index(ctx->m_metaLib, newMacro) != 0) {
                    CPE_ERROR(ctx->m_em, "macro %s name duplicate!", newMacro->m_name);
                }
            }
        }
        else if (strcmp((char const *)localname, "desc") == 0) {
            if (len > CPE_DR_DESC_LEN) len = CPE_DR_DESC_LEN;
            DR_DO_DUP_STR(newMacro->m_desc);
        }
        else if (strcmp((char const *)localname, "value") == 0) {
            if (len >= CPE_INTEGER_BUF_LEN) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
                return;
            }
            haveValue = 1;

            DR_DO_READ_INT_OR_MACRO(int32_t, newMacro->m_data.m_value, 0);
        }
        else {
        }
    }

    if (newMacro->m_name == NULL) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_MACRO_NO_NAME_ATTR);
        haveError = 1;
    }

    if (!haveValue) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_MACRO_NO_VALUE);
        haveError = 1;
    }

    if (haveError) {
        dr_inbuild_metalib_remove_macro(ctx->m_metaLib, newMacro);
    }
    else {
        if (ctx->m_source) {
            dr_metalib_source_element_create(ctx->m_source, dr_metalib_source_element_type_macro, newMacro->m_name);
        }
    }
}

static void dr_build_xml_process_meta(
    struct DRXmlParseCtx * ctx,
    int nb_attributes,
    const xmlChar** attributes,
    int32_t metaType)
{
    int indexAttribute = 0;
    int index = 0;
    int version = -1;
    struct DRInBuildMeta * newMeta = NULL;
    int haveError = 0;

    if (ctx->m_state != PS_InMetaLib) {
        return;
    }

    newMeta = dr_inbuild_metalib_add_meta(ctx->m_metaLib);
    if (newMeta == NULL) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NO_MEMORY);
        return;
    }

    newMeta->m_data.m_type = metaType;
    newMeta->m_data.m_align = ctx->m_metaLib->m_dft_align;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes && !haveError;
        ++indexAttribute, index += 5)
    {
        const xmlChar *localname = attributes[index];
        /*const xmlChar *prefix = attributes[index+1];*/
        /*const xmlChar *nsURI = attributes[index+2];*/
        const xmlChar *valueBegin = attributes[index+3];
        const xmlChar *valueEnd = attributes[index+4];

        int len = (int)(valueEnd - valueBegin);

        if (strcmp((char const *)localname, "name") == 0) {
            char * name;
            if (len > CPE_DR_MACRO_LEN) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
                return;
            }

            DR_DO_DUP_STR(name);

            if (dr_inbuild_meta_set_name(newMeta, name) != 0) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_META_NAME_CONFLICT);
            }
        }
        else if (strcmp((char const *)localname, "desc") == 0) {
            if (len > CPE_DR_DESC_LEN) len = CPE_DR_DESC_LEN;
            DR_DO_DUP_STR(newMeta->m_desc);
        }
        else if (strcmp((char const *)localname, "primarykey") == 0) {
            char * names;
            DR_DO_DUP_STR(names);
             dr_inbuild_meta_add_key_entries(newMeta, names);
        }
        else if (strcmp((char const *)localname, "id") == 0) {
            DR_DO_READ_INT_OR_MACRO(int32_t, newMeta->m_data.m_id, CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE);
        }
        else if (strcmp((char const *)localname, "version") == 0) {
            DR_DO_READ_INT_OR_MACRO(int, version, CPE_DR_ERROR_INVALID_TAGSET_VERSION);
        }
        else if (strcmp((char const *)localname, "align") == 0) {
            DR_DO_READ_INT_OR_MACRO(int32_t, newMeta->m_data.m_align, CPE_DR_ERROR_META_INVALID_ALIGN_VALUE);
        }
        else {
        }
    }


    if (version < 0) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NO_VERSION);
        haveError = 1;
    }
    else if (version > ctx->m_metaLib->m_data.iVersion) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_INVALID_VERSION);
        haveError = 1;
    }
    else {
        newMeta->m_data.m_based_version = version;
        newMeta->m_data.m_current_version = version;
    }

    if (haveError) {
        dr_inbuild_metalib_remove_meta(ctx->m_metaLib, newMeta);
        return;
    }

    ctx->m_state = PS_InMeta;
    ctx->m_curentMeta = newMeta;

    if (ctx->m_source) {
        dr_metalib_source_element_create(ctx->m_source, dr_metalib_source_element_type_meta, newMeta->m_name);
    }
}

static void dr_build_xml_process_entry(
    struct DRXmlParseCtx * ctx,
    int nb_attributes,
    const xmlChar** attributes)
{
    int indexAttribute = 0;
    int index = 0;
    int version = -1;
    int haveError = 0;
    int haveMin = 0;
    int haveMax = 0;
    struct DRInBuildMetaEntry * newEntry;

    if (ctx->m_state != PS_InMeta || ctx->m_curentMeta == NULL) {
        return;
    }

    newEntry = dr_inbuild_meta_add_entry(ctx->m_curentMeta);
    if (newEntry == NULL) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NO_MEMORY);
        return;
    }

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes && !haveError;
        ++indexAttribute, index += 5)
    {
        const xmlChar *localname = attributes[index];
        /*const xmlChar *prefix = attributes[index+1];*/
        /*const xmlChar *nsURI = attributes[index+2];*/
        const xmlChar *valueBegin = attributes[index+3];
        const xmlChar *valueEnd = attributes[index+4];

        int len = (int)(valueEnd - valueBegin);

        if (strcmp((char const *)localname, "name") == 0) {
            if (len >= CPE_DR_NAME_LEN) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
                return;
            }

            DR_DO_DUP_STR(newEntry->m_name);
        }
        else if (strcmp((char const *)localname, "cname") == 0) {
            if (len >= CPE_DR_CHNAME_LEN) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
                len = CPE_DR_CHNAME_LEN - 1;
            }

            DR_DO_DUP_STR(newEntry->m_cname);
        }
        else if (strcmp((char const *)localname, "desc") == 0) {
            if (len >= CPE_DR_DESC_LEN) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
                len = CPE_DR_CHNAME_LEN - 1;
            }

            DR_DO_DUP_STR(newEntry->m_desc);
        }
        else if (strcmp((char const *)localname, "type") == 0) {
            if (len >= CPE_DR_NAME_LEN) {
                DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
                return;
            }

            DR_DO_DUP_STR(newEntry->m_ref_type_name);
        }
        else if (strcmp((char const *)localname, "version") == 0) {
            DR_DO_READ_INT(int, version, CPE_DR_ERROR_INVALID_TAGSET_VERSION);
        }
        else if (strcmp((char const *)localname, "id") == 0) {
            DR_DO_READ_INT_OR_MACRO(int32_t, newEntry->m_data.m_id, CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE);
            newEntry->m_data.m_select_range_max = newEntry->m_data.m_id;
            newEntry->m_data.m_select_range_min = newEntry->m_data.m_id;
        }
        else if (strcmp((char const *)localname, "maxid") == 0) {
            haveMax = 1;
            DR_DO_READ_INT_OR_MACRO(int32_t, newEntry->m_data.m_select_range_max, CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE);
        }
        else if (strcmp((char const *)localname, "minid") == 0) {
            haveMin = 1;
            DR_DO_READ_INT_OR_MACRO(int32_t, newEntry->m_data.m_select_range_min, CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE);
        }
        else if (strcmp((char const *)localname, "select") == 0) {
            DR_DO_DUP_STR(newEntry->m_selector_path);
        }
        else if (strcmp((char const *)localname, "size") == 0) {
            DR_DO_READ_INT_OR_MACRO(int32_t, newEntry->m_data.m_size, CPE_DR_ERROR_ENTRY_INVALID_SIZE_VALUE);
        }
        else if (strcmp((char const *)localname, "defaultvalue") == 0) {
            DR_DO_DUP_STR(newEntry->m_dft_value);
        }
        else if (strcmp((char const *)localname, "count") == 0) {
            DR_DO_READ_INT_OR_MACRO(int32_t, newEntry->m_data.m_array_count, CPE_DR_ERROR_ENTRY_INVALID_COUNT_VALUE);
            if(newEntry->m_data.m_array_count < 0) {
                CPE_ERROR_EX(
                    ctx->m_em, CPE_DR_ERROR_ENTRY_INVALID_COUNT_VALUE,
                    "invalid ount value %d!",
                    (int)newEntry->m_data.m_array_count);
            }
        }
        else if (strcmp((char const *)localname, "refer") == 0) {
            DR_DO_DUP_STR(newEntry->m_refer_path);
        }
        else {
        }
    }

    if (haveMin && haveMax && newEntry->m_data.m_select_range_min > newEntry->m_data.m_select_range_max) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE, "id min(%d) bigger than max(%d)!",
                     newEntry->m_data.m_select_range_min, newEntry->m_data.m_select_range_max);
    }

    if (haveMin && !haveMax) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE, "id maxid not setted!");
    }

    if (haveMax && !haveMin) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE, "id minid not setted!");
    }

    if (version < 0) {
        version = ctx->m_curentMeta->m_data.m_based_version;
    }
    else if (version > ctx->m_metaLib->m_data.iVersion) {
        DR_NOTIFY_ERROR(ctx->m_em, CPE_DR_ERROR_INVALID_VERSION);
        haveError = 1;
    }

    newEntry->m_data.m_version = version;

    if (newEntry->m_name == NULL) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_META_NO_NAME, "entry no name");
        haveError = 1;
    }

    if (newEntry->m_ref_type_name == NULL) {
        CPE_ERROR_EX(ctx->m_em, CPE_DR_ERROR_ENTRY_NO_TYPE, "%s's type is null!", newEntry->m_name);
        haveError = 1;
    }

    if (newEntry->m_data.m_id != -1) {
        struct DRInBuildMetaEntry * check_entry;

        TAILQ_FOREACH(check_entry, &ctx->m_curentMeta->m_entries, m_next) {
            if (check_entry == newEntry) continue;

            if (newEntry->m_data.m_id <= check_entry->m_data.m_select_range_max
                && newEntry->m_data.m_id >= check_entry->m_data.m_select_range_min)
            {
                CPE_ERROR(
                    ctx->m_em, "meta %s id %d duplicate, %s and %s!",
                    ctx->m_curentMeta->m_name, newEntry->m_data.m_id, check_entry->m_name, newEntry->m_name);
                haveError = 1;
            }
        }
    }

    if (haveError) {
        dr_inbuild_meta_remove_entry(ctx->m_curentMeta, newEntry);
        return;
    }
}

static void dr_build_xml_process_index(
    struct DRXmlParseCtx * ctx,
    int nb_attributes,
    const xmlChar** attributes)
{
    int indexAttribute = 0;
    int index = 0;
    const char * name = NULL;
    const char * columns = NULL;

    if (ctx->m_state != PS_InMeta || ctx->m_curentMeta == NULL) {
        return;
    }

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const xmlChar *localname = attributes[index];
        /*const xmlChar *prefix = attributes[index+1];*/
        /*const xmlChar *nsURI = attributes[index+2];*/
        const xmlChar *valueBegin = attributes[index+3];
        const xmlChar *valueEnd = attributes[index+4];

        int len = (int)(valueEnd - valueBegin);

        if (strcmp((char const *)localname, "name") == 0) {
            DR_DO_DUP_STR(name);
        }
        else if (strcmp((char const *)localname, "column") == 0) {
            DR_DO_DUP_STR(columns);
        }
        else {
        }
    }

    if (name == NULL) {
        CPE_ERROR(ctx->m_em, "build index for meta %s, name not configured!", ctx->m_curentMeta->m_name);
    }

    if (columns == NULL) {
        CPE_ERROR(ctx->m_em, "build index for meta %s, column not configured!", ctx->m_curentMeta->m_name);
    }

    if (name && columns) {
        struct dr_inbuild_index * inbuild_index = dr_inbuild_meta_add_index(ctx->m_curentMeta, name);
        if (inbuild_index == NULL) {
            CPE_ERROR(ctx->m_em, "build index for meta %s, add index %s fail!", ctx->m_curentMeta->m_name, name);
        }
        else if (dr_inbuild_index_add_entries(inbuild_index, columns) != 0) {
            CPE_ERROR(ctx->m_em, "build index for meta %s, add columns %s to index %s fail!", ctx->m_curentMeta->m_name, columns, name);
        }
    }
}

static void dr_build_xml_process_include(
    struct DRXmlParseCtx * ctx,
    int nb_attributes,
    const xmlChar** attributes)
{
    int indexAttribute = 0;
    int index = 0;
    const char * include_name = NULL;
    const char * include_file = NULL;
    dr_metalib_source_t include_source;

    if (ctx->m_source == NULL) return;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const xmlChar *localname = attributes[index];
        /*const xmlChar *prefix = attributes[index+1];*/
        /*const xmlChar *nsURI = attributes[index+2];*/
        const xmlChar *valueBegin = attributes[index+3];
        const xmlChar *valueEnd = attributes[index+4];

        int len = (int)(valueEnd - valueBegin);

        if (strcmp((char const *)localname, "name") == 0) {
            DR_DO_DUP_STR(include_name);
        }
        if (strcmp((char const *)localname, "file") == 0) {
            DR_DO_DUP_STR(include_file);
            (void)include_file;
        }
        else {
        }
    }

    if (include_name) {
        include_source = dr_metalib_source_find(ctx->m_source->m_builder, include_name);
        if (include_source == NULL) {
            CPE_ERROR_EX(
                ctx->m_em, CPE_DR_ERROR_ENTRY_NO_TYPE, "include %s not exist!", include_name);
        }
        else {
            if (dr_metalib_source_add_include(ctx->m_source, include_source) != 0) {
                CPE_ERROR_EX(
                    ctx->m_em, CPE_DR_ERROR_ENTRY_NO_TYPE, "add include %s fail!", include_name);
            }
        }
    }
}


static void dr_build_xml_startElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI,
        int nb_namespaces,
        const xmlChar** namespaces,
        int nb_attributes,
        int nb_defaulted,
        const xmlChar** attributes)
{
    struct DRXmlParseCtx * ctx = (struct DRXmlParseCtx *)(inputCtx);

    if (strcmp((const char *)localname, "entry") == 0) {
        dr_build_xml_process_entry(ctx, nb_attributes, attributes);
    }
    else if (strcmp((const char *)localname, "index") == 0) {
        dr_build_xml_process_index(ctx, nb_attributes, attributes);
    }
    else if (strcmp((const char *)localname, "struct") == 0) {
        dr_build_xml_process_meta(ctx, nb_attributes, attributes, CPE_DR_TYPE_STRUCT);
    }
    else if (strcmp((const char *)localname, "union") == 0) {
        dr_build_xml_process_meta(ctx, nb_attributes, attributes, CPE_DR_TYPE_UNION);
    }
    else if (strcmp((const char *)localname, "macro") == 0) {
        dr_build_xml_process_macro(ctx, nb_attributes, attributes);
    }
    else if (strcmp((const char *)localname, "metalib") == 0) {
        dr_build_xml_process_metalib(ctx, nb_attributes, attributes);
    }
    else if (strcmp((const char *)localname, "include") == 0) {
        dr_build_xml_process_include(ctx, nb_attributes, attributes);
    }
    else {
        
    }
}

static void dr_build_xml_endElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct DRXmlParseCtx * ctx = (struct DRXmlParseCtx *)(inputCtx);

    if (strcmp((const char *)localname, "struct") == 0) {
        if (ctx->m_curentMeta) {
            struct dr_inbuild_key_entry * key_entry;

            TAILQ_FOREACH(key_entry, &ctx->m_curentMeta->m_key_entries, m_next) {
                if (dr_inbuild_meta_find_entry(ctx->m_curentMeta, key_entry->m_entry_name) == NULL) {
                    CPE_ERROR(
                        ctx->m_em, "meta %s key entry %s not exist!",
                        ctx->m_curentMeta->m_name, key_entry->m_entry_name);
                }
            }

            ctx->m_curentMeta = NULL;
        }

        if (ctx->m_state == PS_InMeta) {
            ctx->m_state = PS_InMetaLib;
        }
    }
    else if (strcmp((const char *)localname, "union") == 0) {
        ctx->m_curentMeta = NULL;
        if (ctx->m_state == PS_InMeta) {
            ctx->m_state = PS_InMetaLib;
        }
    }
    else {
    }
}

static void dr_build_xml_warningSAX(void * inputCtx, const char *msg, ...) {
    struct DRXmlParseCtx * ctx = (struct DRXmlParseCtx *)(inputCtx);
    va_list ap;

    va_start(ap, msg);

    CPE_ERROR_SET_LEVEL(ctx->m_em, CPE_EL_WARNING);
    CPE_ERROR_SET_ERRNO(ctx->m_em, CPE_DR_ERROR_XML_PARSE);
    cpe_error_do_notify_var(ctx->m_em, msg, ap);

    va_end(ap);
}

static void dr_build_xml_errorSAX(void * inputCtx, const char *msg, ...) {
    struct DRXmlParseCtx * ctx = (struct DRXmlParseCtx *)(inputCtx);
    va_list ap;

    va_start(ap, msg);

    CPE_ERROR_SET_LEVEL(ctx->m_em, CPE_EL_ERROR);
    CPE_ERROR_SET_ERRNO(ctx->m_em, CPE_DR_ERROR_XML_PARSE);
    cpe_error_do_notify_var(ctx->m_em, msg, ap);

    va_end(ap);
}

static xmlSAXHandler g_dr_xml_handler = {
    NULL /* internalSubsetSAXFunc internalSubset */
    , NULL /* isStandaloneSAXFunc isStandalone */
    , NULL /* hasInternalSubsetSAXFunc hasInternalSubset */
    , NULL /* hasExternalSubsetSAXFunc hasExternalSubset */
    , NULL /* resolveEntitySAXFunc resolveEntity */
    , NULL /* getEntitySAXFunc getEntity */
    , NULL /* entityDeclSAXFunc entityDecl */
    , NULL /* notationDeclSAXFunc notationDecl */
    , NULL /* attributeDeclSAXFunc attributeDecl */
    , NULL /* elementDeclSAXFunc elementDecl */
    , NULL /* unparsedEntityDeclSAXFunc unparsedEntityDecl */
    , NULL /* setDocumentLocatorSAXFunc setDocumentLocator */
    , NULL /* startDocumentSAXFunc startDocument */
    , NULL /* endDocumentSAXFunc endDocument */
    , NULL /* startElementSAXFunc startElement */
    , NULL /* endElementSAXFunc endElement */
    , NULL /* referenceSAXFunc reference */
    , NULL /* charactersSAXFunc characters */
    , NULL /* ignorableWhitespaceSAXFunc ignorableWhitespace */
    , NULL /* processingInstructionSAXFunc processingInstruction */
    , NULL /* commentSAXFunc comment */
    , dr_build_xml_warningSAX /* warningSAXFunc warning */
    , dr_build_xml_errorSAX /* errorSAXFunc error */
    , NULL /* fatalErrorSAXFunc fatalError; unused error() get all the errors * */
    , NULL /* getParameterEntitySAXFunc getParameterEntity */
    , NULL /* cdataBlockSAXFunc cdataBlock */
    , NULL /* externalSubsetSAXFunc externalSubset */
    , XML_SAX2_MAGIC /* unsigned int initialized */
    , NULL /* void *_private */
    , dr_build_xml_startElement /* startElementNsSAX2Func startElementNs */
    , dr_build_xml_endElement /* endElementNsSAX2Func endElementNs */
    , NULL /* xmlStructuredErrorFunc serror */
};

void dr_build_xml_parse_ctx_init(struct DRXmlParseCtx * ctx) {
    bzero(ctx, sizeof(struct DRXmlParseCtx));

    ctx->m_state = PS_Init;
}

void dr_build_xml_parse_ctx_clear(struct DRXmlParseCtx * ctx) {
}

int  dr_create_lib_from_xml(
    mem_buffer_t buffer,
    const char* buf,
    int bufSize,
    uint8_t dft_align,
    FILE * errorFp)
{
    CPE_DEF_ERROR_MONITOR(em, cpe_error_log_to_file, errorFp);

    return dr_create_lib_from_xml_ex(buffer, buf, bufSize, dft_align, &em);
}

void dr_metalib_source_analize_xml(
    dr_metalib_source_t source,
    struct DRInBuildMetaLib * inbuild_lib,
    const void * buf,
    int bufSize,
    error_monitor_t em)
{
    xmlParserCtxtPtr parseCtx = NULL;
    struct DRXmlParseCtx ctx;

    dr_build_xml_parse_ctx_init(&ctx);

    ctx.m_metaLib = inbuild_lib;
    ctx.m_em = em;
    ctx.m_source = source;

    parseCtx = xmlCreatePushParserCtxt(&g_dr_xml_handler, &ctx, buf, bufSize, NULL);

    xmlParseChunk(parseCtx, NULL, 0, 1);
    xmlFreeParserCtxt(parseCtx);

    dr_build_xml_parse_ctx_clear(&ctx);
}

void dr_create_lib_from_xml_ex_i(
    mem_buffer_t buffer,
    const char* buf,
    int bufSize,
    int * ret,
    uint8_t dft_align,
    error_monitor_t em)
{
    struct DRInBuildMetaLib * inbuild_lib;

    inbuild_lib = dr_inbuild_create_lib();
    if (inbuild_lib == NULL) {
        *ret = -1;
        return;
    }

    dr_inbuild_set_dft_align(inbuild_lib, dft_align);

    dr_metalib_source_analize_xml(NULL, inbuild_lib, buf, bufSize, em);

    dr_inbuild_build_lib(buffer, inbuild_lib, em);

    dr_inbuild_free_lib(inbuild_lib);
}

int dr_create_lib_from_xml_ex(
    mem_buffer_t buffer,
    const char* buf,
    int bufSize,
    uint8_t dft_align,
    error_monitor_t em)
{
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        dr_create_lib_from_xml_ex_i(buffer, buf, bufSize, &ret, dft_align, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        dr_create_lib_from_xml_ex_i(buffer, buf, bufSize, &ret, dft_align, &logError);
    }

    return ret;
}
