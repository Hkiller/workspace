#include "libxml/encoding.h"
#include "libxml/xmlwriter.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

#define DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE(__attrname, __attrvalue) \
    if (xmlTextWriterWriteAttribute(writer, BAD_CAST __attrname, BAD_CAST __attrvalue) < 0) { \
        CPE_ERROR(em, "dr_save_lib_to_xml_file: Error write attribute %s, value=%s", (__attrname), (__attrvalue)); \
        return -1;                                                      \
    }

#ifdef _MSC_VER
#define DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT(__attrname, __fmt, ...)      \
    if (xmlTextWriterWriteFormatAttribute(writer, BAD_CAST __attrname, __fmt, __VA_ARGS__) < 0) { \
        CPE_ERROR(em, "dr_save_lib_to_xml_file: Error write attribute %s, "__fmt, (__attrname), __VA_ARGS__); \
        return -1;                                                      \
    }
#else
#define DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT(__attrname, __fmt, __args...)      \
    if (xmlTextWriterWriteFormatAttribute(writer, BAD_CAST __attrname, __fmt, ##__args) < 0) { \
        CPE_ERROR(em, "dr_save_lib_to_xml_file: Error write attribute %s, "__fmt, (__attrname), ##__args); \
        return -1;                                                      \
    }
#endif

#define DR_SAVE_LIB_TO_XML_WRITE_START_ELEMENT(__element_name)          \
    if (xmlTextWriterStartElement(writer, BAD_CAST __element_name) < 0) {   \
        CPE_ERROR(em, "dr_save_lib_to_xml_file: start element %s fail!", __element_name); \
        return -1;                                                      \
    }

#define DR_SAVE_LIB_TO_XML_WRITE_END_ELEMENT()          \
    if (xmlTextWriterEndElement(writer) < 0) {   \
        CPE_ERROR(em, "dr_save_lib_to_xml_file: end element fail!"); \
        return -1;                                                      \
    }

static int dr_save_lib_to_xml_entries(LPDRMETA meta, xmlTextWriterPtr writer, error_monitor_t em) {
    int i;
    int count;
    char buf[128];

    count = dr_meta_entry_num(meta);
    for(i = 0; i < count; ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(meta, i);
        LPDRMETA ref_meta = dr_entry_ref_meta(entry);
        LPDRMETAENTRY array_refer_entry = dr_entry_array_refer_entry(entry);
        LPDRMETAENTRY selector_entry = dr_entry_select_entry(entry);

        DR_SAVE_LIB_TO_XML_WRITE_START_ELEMENT("entry");

        DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("name", dr_entry_name(entry));
        DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE(
            "type", (ref_meta ? dr_meta_name(ref_meta) : dr_find_ctype_info_by_type(dr_entry_type(entry))->m_name) );

        if (dr_entry_id(entry) != -1) DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("id", "%d", dr_entry_id(entry));

        if (dr_entry_type(entry) == CPE_DR_TYPE_STRING) {
            DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("size", "%d", (int)dr_entry_size(entry));
        }

        if (dr_entry_array_count(entry) != 1) {
            DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("count", "%d", (int)dr_entry_array_count(entry));
        }

        if (array_refer_entry) {
            DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("refer", dr_meta_off_to_path(meta, dr_entry_data_start_pos(array_refer_entry, 0), buf, sizeof(buf)));
        }

        if (selector_entry) {
            DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("select", dr_meta_off_to_path(meta, dr_entry_data_start_pos(selector_entry, 0), buf, sizeof(buf)));
        }

        if (strcmp(dr_entry_cname(entry), "") != 0) DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("cname", dr_entry_cname(entry));
        if (strcmp(dr_entry_desc(entry), "") != 0) DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("desc", dr_entry_desc(entry));

        DR_SAVE_LIB_TO_XML_WRITE_END_ELEMENT();
    }
    
    return 0;
}

static int dr_save_lib_to_xml_indexies(LPDRMETA meta, xmlTextWriterPtr writer, error_monitor_t em) {
    int i, j;
    int count;
    char buf[128];
    size_t size;

    count = dr_meta_index_num(meta);
    for(i = 0; i < count; ++i) {
        dr_index_info_t index = dr_meta_index_at(meta, i);

        DR_SAVE_LIB_TO_XML_WRITE_START_ELEMENT("index");

        DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("name", dr_index_name(index));

        buf[0] = 0;
        size = 0;
        if (dr_index_entry_num(index) > 0) {
            size += snprintf(buf + size, sizeof(buf) - size, "%s", dr_entry_name(dr_index_entry_at(index, 0)));
            for(j = 1; j < dr_index_entry_num(index); ++j) {
                size += snprintf(buf + size, sizeof(buf) - size, ",%s", dr_entry_name(dr_index_entry_at(index, j)));
            }

            DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("column", "%s", buf);
        }


        DR_SAVE_LIB_TO_XML_WRITE_END_ELEMENT();
    }
    
    return 0;
}

static int dr_save_lib_to_xml_metas(LPDRMETALIB metaLib, xmlTextWriterPtr writer, error_monitor_t em) {
    int i;
    int count;

    count = dr_lib_meta_num(metaLib);
    for(i = 0; i < count; ++i) {
        LPDRMETA meta = dr_lib_meta_at(metaLib, i);

        if (meta->m_type == CPE_DR_TYPE_STRUCT) {
            DR_SAVE_LIB_TO_XML_WRITE_START_ELEMENT("struct");
        }
        else {
            DR_SAVE_LIB_TO_XML_WRITE_START_ELEMENT("union");
        }
    
        DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("name", dr_meta_name(meta));
        DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("version", "%d", dr_meta_current_version(meta));
        if (strcmp(dr_meta_desc(meta), "") != 0) DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("desc", dr_meta_desc(meta));

        if (dr_meta_key_entry_num(meta) > 0) {
            char buf[128];
            size_t size = 0;
            int i;
            size += snprintf(buf + size, sizeof(buf) - size, "%s", dr_entry_name(dr_meta_key_entry_at(meta, 0)));
            for(i = 1; i < dr_meta_key_entry_num(meta); ++i) {
                size += snprintf(buf + size, sizeof(buf) - size, ",%s", dr_entry_name(dr_meta_key_entry_at(meta, i)));
            }

            DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("primarykey", "%s", buf);
        }

        DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("align", "%d", dr_meta_align(meta));

        if (dr_save_lib_to_xml_entries(meta, writer, em) != 0) return -1;
        if (dr_save_lib_to_xml_indexies(meta, writer, em) != 0) return -1;

        DR_SAVE_LIB_TO_XML_WRITE_END_ELEMENT();
    }

    return 0;
}

static int dr_save_lib_to_xml_macros(LPDRMETALIB metaLib, xmlTextWriterPtr writer, error_monitor_t em) {
    int i;
    int count;

    count = dr_lib_macro_num(metaLib);
    for(i = 0; i < count; ++i) {
        LPDRMACRO macro = dr_lib_macro_at(metaLib, i);

        DR_SAVE_LIB_TO_XML_WRITE_START_ELEMENT("macro");
    
        DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("name", dr_macro_name(metaLib, macro));
        DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("value", "%d", dr_macro_value(macro));

        DR_SAVE_LIB_TO_XML_WRITE_END_ELEMENT();
    }

    return 0;
}

static int dr_save_lib_to_xml_i(LPDRMETALIB metaLib, xmlTextWriterPtr writer, error_monitor_t em) {
    xmlTextWriterSetIndent(writer, 1);
    xmlTextWriterSetIndentString(writer, BAD_CAST "    ");

    if (xmlTextWriterStartDocument(writer, NULL, "utf-8", NULL) < 0) {
        CPE_ERROR(em, "dr_save_lib_to_xml_file: Error at xmlTextWriterStartDocument");
        return -1;
    }

    DR_SAVE_LIB_TO_XML_WRITE_START_ELEMENT("metalib");

    DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("tagsetversion", "%d", metaLib->m_tag_set_version);
    DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE("name", dr_lib_name(metaLib));
    DR_SAVE_LIB_TO_XML_WRITE_ATTRIBUTE_FMT("version", "%d", metaLib->m_version);

    if (dr_save_lib_to_xml_macros(metaLib, writer, em) != 0) return -1;
    if (dr_save_lib_to_xml_metas(metaLib, writer, em) != 0) return -1;

    DR_SAVE_LIB_TO_XML_WRITE_END_ELEMENT();

    if (xmlTextWriterEndDocument(writer) < 0) {
        CPE_ERROR(em, "dr_save_lib_to_xml_file: Error at xmlTextWriterEndDocument\n");
        return -1;
    }

    return 0;
}

int dr_save_lib_to_xml_file(LPDRMETALIB metaLib, const char * fileName, error_monitor_t em) {
    int rv;
    xmlTextWriterPtr writer;

    writer = xmlNewTextWriterFilename(fileName, 0);
    if (writer == NULL) {
        CPE_ERROR(em, "dr_save_lib_to_xml_file: Error creating the xml writer");
        return -1;
    }

    rv = dr_save_lib_to_xml_i(metaLib, writer, em);

    xmlFreeTextWriter(writer);

    return rv;
}

char* dr_save_lib_to_xml_buf(
    mem_buffer_t buffer,
    LPDRMETALIB metaLib,
    error_monitor_t em)
{
    int rv;
    xmlBufferPtr buf;
    xmlTextWriterPtr writer;

    mem_buffer_clear_data(buffer);

    buf = xmlBufferCreate();
    if (buf == NULL) {
        CPE_ERROR(em, "dr_save_lib_to_xml_buf: Error creating the xml buffer\n");
        return NULL;
    }

    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL) {
        CPE_ERROR(em, "dr_save_lib_to_xml_buf: Error creating the xml writer\n");
        xmlBufferFree(buf);
        return NULL;
    }

    rv = dr_save_lib_to_xml_i(metaLib, writer, em);
    (void)rv;

    mem_buffer_append(buffer, buf->content, buf->use);
    mem_buffer_append_char(buffer, 0);

    xmlFreeTextWriter(writer);
    xmlBufferFree(buf);

    return (char*)mem_buffer_make_continuous(buffer, 0);
}

