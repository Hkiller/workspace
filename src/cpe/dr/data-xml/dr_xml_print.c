#include <assert.h>
#include "libxml/xmlwriter.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stackbuf.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_xml.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

struct DrXmlPrintProcessStack {
    LPDRMETA m_meta;
    LPDRMETAENTRY m_entry;
    int m_entry_pos;
    int m_entry_count;
    int m_array_pos;
    const char * m_src_data;
    size_t m_src_capacity;
};

/* static const char * yajl_errno_to_string(yajl_gen_status s) { */
/*     static const char * s_errorMsgs[] = { */
/*         "keys_must_be_strings" */
/*         , "max_depth_exceeded" */
/*         , "in_error_state" */
/*         , "generation_complete" */
/*         , "invalid_number" */
/*         , "no_buf" */
/*         , "invalid_string" }; */

/*     if (s < 1/\*yajl_gen_keys_must_be_strings*\/ || s > yajl_gen_invalid_string) { */
/*         return "invalid yajl error"; */
/*     } */
/*     else { */
/*         return s_errorMsgs[s - 1]; */
/*     } */
/* } */

/* #define XML_PRINT_CHECK_GEN_RESULT(s)                          \ */
/*     { yajl_gen_status __s = (s);                                \ */
/*     if (__s != yajl_gen_status_ok) {                            \ */
/*     CPE_ERROR(em, "yajl error: %s", yajl_errno_to_string(__s)); \ */
/*     return;                                                     \ */
/*     }                                                           \ */
/*     } */

/* #define XML_PRINT_GEN_STRING(g, str)               \ */
/*     do {                                            \ */
/*         const char * __p = (str);                   \ */
/*         XML_PRINT_CHECK_GEN_RESULT(                \ */
/*             yajl_gen_string(g, (const unsigned char *)__p, strlen(__p))); \ */
/*     } while(0) */

/* static void dr_print_print_numeric(yajl_gen g, int typeId, const void * data, error_monitor_t em) { */
/*     else { */
/*         yajl_gen_null(g); */
/*     } */
/* } */

static void dr_print_print_basic_data(xmlTextWriterPtr xml_writer, LPDRMETAENTRY entry, const void * data, error_monitor_t em) {
    switch(entry->m_type) {
    case CPE_DR_TYPE_INT8:
    case CPE_DR_TYPE_UINT8:
    case CPE_DR_TYPE_INT16:
    case CPE_DR_TYPE_UINT16:
    case CPE_DR_TYPE_INT32:
    case CPE_DR_TYPE_UINT32:
    case CPE_DR_TYPE_INT64:
    case CPE_DR_TYPE_UINT64:
    case CPE_DR_TYPE_FLOAT:
    case CPE_DR_TYPE_DOUBLE:
    case CPE_DR_TYPE_CHAR:
    case CPE_DR_TYPE_UCHAR: {
        char buf[20 + 1];
        struct write_stream_mem bufS = CPE_WRITE_STREAM_MEM_INITIALIZER(buf, 20);
        int len = dr_ctype_print_to_stream((write_stream_t)&bufS, data, entry->m_type, em);
        if (len > 0) {
            buf[len] = 0;
            if (xmlTextWriterWriteString(xml_writer, (xmlChar const *)buf) < 0) {
                CPE_ERROR(em, "dr_xml_print: write value %s of %s fail", buf, dr_entry_name(entry));
            }
        }
        else {
            CPE_ERROR(em, "dr_xml_print: write value of %s fail, no data", dr_entry_name(entry));
        }
        break;
    }
    case CPE_DR_TYPE_STRING:
    case (CPE_DR_TYPE_STRING + 1):
        if (xmlTextWriterWriteString(xml_writer, (xmlChar const *)data) < 0) {
            CPE_ERROR(em, "dr_xml_print: write string %s of %s fail", (const char *)data, dr_entry_name(entry));
        }
        break;
    default:
        CPE_ERROR(
            em, "dr_xml_print: write data of %s fail, supported type "FMT_INT32_T"!",
            dr_entry_name(entry), entry->m_type);
        break;
    }
}

void dr_xml_print_i(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    xmlTextWriterPtr xml_writer,
    error_monitor_t em)
{
    struct DrXmlPrintProcessStack processStack[CPE_DR_MAX_LEVEL];
    int stackPos;

    processStack[0].m_meta = meta;
    processStack[0].m_entry = dr_meta_entry_at(meta, 0);
    processStack[0].m_entry_pos = 0;
    processStack[0].m_entry_count = meta->m_entry_count;
    processStack[0].m_array_pos = 0;
    processStack[0].m_src_data = (const char *)input;
    processStack[0].m_src_capacity = capacity;

    if (xmlTextWriterStartDocument(xml_writer, NULL, "UTF-8", NULL) < 0) {
        CPE_ERROR(em, "dr_xml_print: xmlTextWriterStartDocument error");
        return;
    }

    if (xmlTextWriterStartElement(xml_writer, (xmlChar const *)dr_meta_name(meta)) < 0) {
        CPE_ERROR(em, "dr_xml_print: xmlTextWriterStartElement %s error", dr_meta_name(meta));
        return;
    }

    for(stackPos = 0; stackPos >= 0;) {
        struct DrXmlPrintProcessStack * curStack;

        assert(stackPos < CPE_DR_MAX_LEVEL);

        curStack = &processStack[stackPos];
        if (curStack->m_meta == NULL) {
            --stackPos;
            continue;
        }

        for(; curStack->m_entry_pos < curStack->m_entry_count
                && curStack->m_entry
                ;
            ++curStack->m_entry_pos
                , curStack->m_array_pos = 0
                , curStack->m_entry = dr_meta_entry_at(curStack->m_meta, curStack->m_entry_pos)
            )
        {
            size_t elementSize;
            int32_t array_count;
            LPDRMETAENTRY refer;

        LOOPENTRY:
            elementSize = dr_entry_element_size(curStack->m_entry);
            if (elementSize == 0) continue;

            refer = NULL;
            if (curStack->m_entry->m_array_count != 1) {
                refer = dr_entry_array_refer_entry(curStack->m_entry);
            }

            array_count = curStack->m_entry->m_array_count;
            if (refer) {
                dr_entry_try_read_int32(
                    &array_count,
                    curStack->m_src_data + curStack->m_entry->m_array_refer_data_start_pos,
                    refer,
                    em);
            }

            for(; curStack->m_array_pos < array_count; ++curStack->m_array_pos) {
                const char * entryData = curStack->m_src_data + dr_entry_data_start_pos(curStack->m_entry, curStack->m_array_pos);
                if ((size_t)(entryData - curStack->m_src_data) > curStack->m_src_capacity) {
                    CPE_ERROR(
                        em, "%s.%s[%d]: read size overflow, capacity=%d",
                        dr_meta_name(dr_entry_self_meta(curStack->m_entry)), dr_entry_name(curStack->m_entry),
                        curStack->m_array_pos, (int)curStack->m_src_capacity);
                    break;
                }

                if (curStack->m_entry->m_type <= CPE_DR_TYPE_COMPOSITE) {
                    if (stackPos + 1 < CPE_DR_MAX_LEVEL) {
                        struct DrXmlPrintProcessStack * nextStack;
                        nextStack = &processStack[stackPos + 1];

                        nextStack->m_meta = dr_entry_ref_meta(curStack->m_entry);
                        if (nextStack->m_meta == 0) {
                            //yajl_gen_map_close(g);
                            break;
                        }

                        nextStack->m_src_data = entryData;
                        if (curStack->m_entry_pos + 1 == curStack->m_entry_count && curStack->m_array_pos + 1 == array_count) {
                            nextStack->m_src_capacity = curStack->m_src_capacity - (entryData - curStack->m_src_data);
                        }
                        else {
                            nextStack->m_src_capacity = curStack->m_src_capacity - (entryData - curStack->m_src_data);
                            if (nextStack->m_src_capacity > elementSize) nextStack->m_src_capacity = elementSize;
                        }

                        nextStack->m_entry_pos = 0;
                        nextStack->m_entry_count = nextStack->m_meta->m_entry_count;

                        if (curStack->m_entry->m_type == CPE_DR_TYPE_UNION) {
                            LPDRMETAENTRY select_entry;
                            select_entry = dr_entry_select_entry(curStack->m_entry);
                            if (select_entry) {
                                int32_t union_entry_id;
                                dr_entry_try_read_int32(
                                    &union_entry_id,
                                    curStack->m_src_data + curStack->m_entry->m_select_data_start_pos,
                                    select_entry,
                                    em);
                                nextStack->m_entry_pos =
                                    dr_meta_find_entry_idx_by_id(nextStack->m_meta, union_entry_id);
                                if (nextStack->m_entry_pos < 0) {
                                    //yajl_gen_map_close(g);
                                    continue;
                                }

                                nextStack->m_entry_count = nextStack->m_entry_pos + 1;
                            }
                        }

                        nextStack->m_entry = dr_meta_entry_at(nextStack->m_meta, nextStack->m_entry_pos);

                        nextStack->m_array_pos = 0;

                        if (xmlTextWriterStartElement(xml_writer, (xmlChar const *)dr_entry_name(curStack->m_entry)) < 0) {
                            CPE_ERROR(em, "dr_xml_print: xmlTextWriterStartElement %s error", dr_entry_name(curStack->m_entry));
                        }

                        ++curStack->m_array_pos;
                        ++stackPos;
                        curStack = nextStack;
                        goto LOOPENTRY;
                    }
                }
                else {
                    if ((size_t)(entryData + elementSize - curStack->m_src_data) > curStack->m_src_capacity) {
                        CPE_ERROR(
                            em, "%s.%s[%d]: read size overflow, element-size=%d, capacity=%d",
                            dr_meta_name(dr_entry_self_meta(curStack->m_entry)), dr_entry_name(curStack->m_entry),
                            curStack->m_array_pos, (int)elementSize, (int)curStack->m_src_capacity);
                        break;
                    }

                    if (xmlTextWriterStartElement(xml_writer, (xmlChar const *)dr_entry_name(curStack->m_entry)) < 0) {
                        CPE_ERROR(em, "dr_xml_print: xmlTextWriterStartElement %s error", dr_entry_name(curStack->m_entry));
                    }

                    dr_print_print_basic_data(
                        xml_writer,
                        curStack->m_entry,
                        curStack->m_src_data + dr_entry_data_start_pos(curStack->m_entry, curStack->m_array_pos),
                        em);

                    if (xmlTextWriterEndElement(xml_writer) < 0) {
                        CPE_ERROR(em, "dr_xml_print: xmlTextWriterEndElement %s error", dr_entry_name(curStack->m_entry));
                    }
                }
            }
        }

        if (stackPos > 0) {
            if (xmlTextWriterEndElement(xml_writer) < 0) {
                CPE_ERROR(em, "dr_xml_print: xmlTextWriterEndElement %s error", dr_entry_name(curStack->m_entry));
            }
        }

        --stackPos;
    }

    if (xmlTextWriterEndElement(xml_writer) < 0) {
        CPE_ERROR(em, "dr_xml_print: xmlTextWriterEndElement %s error", dr_meta_name(meta));
        return;
    }

    if (xmlTextWriterEndDocument(xml_writer) < 0) {
        CPE_ERROR(em, "dr_xml_print: xmlTextWriterEndDocument error");
        return;
    }
}

struct dr_xml_print_ctx {
    write_stream_t m_output;
    int m_total_size;
};

static int dr_xml_do_print(struct dr_xml_print_ctx * ctx, const void * buf, size_t size) {
    int r = stream_write(ctx->m_output, buf, size);
    if (r > 0) {
        ctx->m_total_size += r;
    }
    return r;
}

int dr_xml_print(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    int flag,
    error_monitor_t em)
{
    int ret = 0;
    struct dr_xml_print_ctx print_ctx = { output, 0 };
    xmlOutputBufferPtr xml_output;
    xmlTextWriterPtr xml_writer;

    if (output == NULL || input == NULL || meta == NULL) {
        CPE_ERROR(em, "dr_xml_print: bad para!");
        return -1;
    }

    xml_output = xmlOutputBufferCreateIO(
        (xmlOutputWriteCallback)dr_xml_do_print,
        (xmlOutputCloseCallback)NULL,
        (void *)&print_ctx, NULL);
    if (xml_output == NULL) {
        CPE_ERROR_EX(em, CPE_DR_ERROR_NO_MEMORY, "alloc xml_output fail!");
        return -1;
    }

    xml_writer = xmlNewTextWriter(xml_output);
    if (xml_writer == NULL) {
        CPE_ERROR_EX(em, CPE_DR_ERROR_NO_MEMORY, "alloc xml_writer fail!");
        xmlOutputBufferClose(xml_output);
        return -1;
    }

    if (flag & DR_XML_PRINT_BEAUTIFY) {
        xmlTextWriterSetIndent(xml_writer, 4);
    }

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        dr_xml_print_i(output, input, capacity, meta, xml_writer, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        dr_xml_print_i(output, input, capacity, meta, xml_writer, &logError);
    }

    xmlFreeTextWriter(xml_writer);

    return ret == 0 ? print_ctx.m_total_size : ret;
}
