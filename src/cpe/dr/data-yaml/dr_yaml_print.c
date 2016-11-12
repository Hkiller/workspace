#include <assert.h>
#include "yaml.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stackbuf.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_yaml.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

struct dr_yaml_print_process_stack {
    LPDRMETA m_meta;
    LPDRMETAENTRY m_entry;
    int m_entry_pos;
    int m_entry_count;
    int m_array_pos;
    const char * m_src_data;
    size_t m_src_capacity;
};

static void dr_yaml_notify_document_error(yaml_document_t * document, error_monitor_t em) {
    CPE_ERROR(em, "Memory error: Not enough memory for creating a document");
}

int dr_yaml_add_struct_i(
    yaml_document_t * document,
    const char * input, uint32_t capacity, LPDRMETA meta,
    error_monitor_t em);

int dr_yaml_add_basic_i(
    yaml_document_t * document,
    const char * input, size_t capacity, LPDRMETAENTRY entry,
    error_monitor_t em)
{
    if (entry->m_type == CPE_DR_TYPE_STRING) {
        return yaml_document_add_scalar( 
                document,
                (yaml_char_t *)YAML_STR_TAG,
                (yaml_char_t *)input,
                -1,
                YAML_SINGLE_QUOTED_SCALAR_STYLE);
    }
    else if (entry->m_type > CPE_DR_TYPE_COMPOSITE) {
        char buf[20 + 1];
        struct write_stream_mem bufS = CPE_WRITE_STREAM_MEM_INITIALIZER(buf, 20);
        int len = dr_ctype_print_to_stream((write_stream_t)&bufS, input, entry->m_type, em);
        if (len > 0) {
            const char * tag = NULL;
            char tag_buf[32];
            
            buf[len] = 0;

            if (entry->m_type != CPE_DR_TYPE_INT32 && entry->m_type != CPE_DR_TYPE_FLOAT) {
                snprintf(tag_buf, sizeof(tag_buf), "!%s", dr_type_name(entry->m_type));
                tag = tag_buf;
            }

            return yaml_document_add_scalar( 
                    document,
                    (yaml_char_t*)tag,
                    (yaml_char_t *)buf,
                    -1,
                    YAML_PLAIN_SCALAR_STYLE);
        }
    }

    return 0;
}

int dr_yaml_add_basic_array_i(
    yaml_document_t * document,
    const char * input, uint32_t count, LPDRMETAENTRY entry, 
    error_monitor_t em)
{
    int root_node;
    uint32_t i;
    uint32_t element_capacity = (uint32_t)dr_entry_element_size(entry);
    
    root_node = yaml_document_add_sequence(document, NULL, YAML_BLOCK_SEQUENCE_STYLE);
    if (root_node == 0) {
        dr_yaml_notify_document_error(document, em);
        return 0;
    }

    for(i = 0; i < count; ++i, input += element_capacity) {
        int value = dr_yaml_add_basic_i(document, input, element_capacity, entry, em);

        if (value == 0) continue;
        
        if (!yaml_document_append_sequence_item(document, root_node, value)) {
            dr_yaml_notify_document_error(document, em);
            continue;
        }
    }
    
    return root_node;
}

int dr_yaml_add_struct_array_i(
    yaml_document_t * document,
    const char * input, uint32_t count, LPDRMETA meta, uint32_t element_size,
    error_monitor_t em)
{
    int root_node;
    uint32_t i;
    
    root_node = yaml_document_add_sequence(document, NULL, YAML_BLOCK_SEQUENCE_STYLE);
    if (root_node == 0) {
        dr_yaml_notify_document_error(document, em);
        return 0;
    }

    for(i = 0; i < count; ++i, input += element_size) {
        int value = dr_yaml_add_struct_i(document, input, element_size, meta, em);
        if (value == 0) {
            dr_yaml_notify_document_error(document, em);
            continue;
        }
        
        if (!yaml_document_append_sequence_item(document, root_node, value)) {
            dr_yaml_notify_document_error(document, em);
            continue;
        }
    }
    
    return root_node;
}

int dr_yaml_add_union_i(
    yaml_document_t * document,
    const char * input, uint32_t capacity, LPDRMETA meta, LPDRMETAENTRY union_entry,
    error_monitor_t em)
{
    if (union_entry == NULL) {
        return dr_yaml_add_struct_i(document, input, capacity, meta, em);
    }
    else {
        int union_node;
        int value = 0;
        int key;

        union_node = yaml_document_add_mapping(document, NULL, YAML_BLOCK_MAPPING_STYLE);
        if (!union_node) {
            dr_yaml_notify_document_error(document, em);
            return 0;
        }

        if (union_entry->m_type > CPE_DR_TYPE_COMPOSITE) {
            value = dr_yaml_add_basic_i(document, input, capacity, union_entry, em);
        }
        else {
            value = dr_yaml_add_struct_i(document, input, capacity, dr_entry_ref_meta(union_entry), em);
        }

        if (value == 0) {
            CPE_ERROR(em, "add union entry value %s.%s fail!", dr_meta_name(meta), dr_entry_name(union_entry));
            return 0;
        }

        key = yaml_document_add_scalar(
            document, NULL, (yaml_char_t *) dr_entry_name(union_entry), -1, YAML_PLAIN_SCALAR_STYLE);
        if (key == 0) {
            CPE_ERROR(em, "add union entry key %s.%s fail!", dr_meta_name(meta), dr_entry_name(union_entry));
            return 0;
        }

        if (!yaml_document_append_mapping_pair(document, union_node, key, value)) {
            dr_yaml_notify_document_error(document, em);
            return 0;
        }
        
        return union_node;
    }
    
}

int dr_yaml_add_union_array_i(
    yaml_document_t * document,
    const char * input, uint32_t count, LPDRMETA meta, uint32_t element_size, LPDRMETAENTRY union_entry,
    error_monitor_t em)
{
    int root_node;
    uint32_t i;
    
    root_node = yaml_document_add_sequence(document, NULL, YAML_BLOCK_SEQUENCE_STYLE);
    if (root_node == 0) {
        dr_yaml_notify_document_error(document, em);
        return 0;
    }

    for(i = 0; i < count; ++i, input += element_size) {
        int value = dr_yaml_add_union_i(document, input, element_size, meta, union_entry, em);
        if (value == 0) {
            dr_yaml_notify_document_error(document, em);
            continue;
        }
        
        if (!yaml_document_append_sequence_item(document, root_node, value)) {
            dr_yaml_notify_document_error(document, em);
            continue;
        }
    }
    
    return root_node;
}

int dr_yaml_add_struct_i(
    yaml_document_t * document,
    const char * input, uint32_t capacity, LPDRMETA meta,
    error_monitor_t em)
{
    uint16_t i = 0;
    uint16_t entry_num = dr_meta_entry_num(meta);
    int struct_node;
    
    struct_node = yaml_document_add_mapping(document, NULL, YAML_BLOCK_MAPPING_STYLE);
    if (!struct_node) {
        dr_yaml_notify_document_error(document, em);
        return 0;
    }

    for(i = 0; i < dr_meta_entry_num(meta); ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(meta, i);
        uint32_t entry_capacity;
        const char * entry_data;
        uint32_t array_count;
        int value = 0;
        int key;
        
        if ((uint32_t)entry->m_data_start_pos >= capacity) continue;

        entry_data = input + entry->m_data_start_pos;
        entry_capacity = capacity - entry->m_data_start_pos;

        if (i + 1 != entry_num && entry_capacity > (uint32_t)entry->m_unitsize) {
            entry_capacity = entry->m_unitsize;
        }

        array_count = entry->m_array_count;
        if (array_count != 1) {
            LPDRMETAENTRY refer = dr_entry_array_refer_entry(entry);
            if (refer) {
                dr_entry_try_read_uint32(
                    &array_count,
                    input + entry->m_array_refer_data_start_pos,
                    refer, em);
            }
        }

        if (entry->m_type > CPE_DR_TYPE_COMPOSITE) {
            if (entry->m_array_count == 1) {
                value = dr_yaml_add_basic_i(document, entry_data, entry_capacity, entry, em);
            }
            else {
                value = dr_yaml_add_basic_array_i(document, entry_data, array_count, entry, em);
            }
        }
        else {
            LPDRMETA entry_meta;
            uint32_t element_size = (uint32_t)dr_entry_element_size(entry);

            entry_meta = dr_entry_ref_meta(entry);
            if (entry_meta == NULL) {
                CPE_ERROR(em, "add value %s.%s: ref meta not exist!", dr_meta_name(meta), dr_entry_name(entry));
                continue;
            }

            if (entry_meta->m_type == CPE_DR_TYPE_UNION) {
                LPDRMETAENTRY select_entry;
                int32_t union_entry_id;
                
                select_entry = dr_entry_select_entry(entry);
                if (select_entry) {
                    LPDRMETAENTRY union_entry;
                    
                    dr_entry_try_read_int32(
                        &union_entry_id,
                        input + entry->m_select_data_start_pos,
                        select_entry,
                        em);

                    union_entry = dr_meta_find_entry_by_id(entry_meta, union_entry_id);
                    if (union_entry == NULL) {
                        CPE_ERROR(
                            em, "add value %s.%s: union meta %s no entry id = %d!",
                            dr_meta_name(meta), dr_entry_name(entry), dr_meta_name(entry_meta), union_entry_id);
                        continue;
                    }

                    if (entry->m_array_count == 1) {
                        value = dr_yaml_add_union_i(document, entry_data, entry_capacity, entry_meta, union_entry, em);
                    }
                    else {
                        value = dr_yaml_add_union_array_i(document, entry_data, array_count, entry_meta, element_size, union_entry, em);
                    }
                }
                else { /*没有select的Union */
                    if (entry->m_array_count == 1) {
                        value = dr_yaml_add_struct_i(document, entry_data, entry_capacity, entry_meta, em);
                    }
                    else {
                        value = dr_yaml_add_struct_array_i(document, entry_data, array_count, entry_meta, element_size, em);
                    }
                }
            }
            else { /*struct */
                if (entry->m_array_count == 1) {
                    value = dr_yaml_add_struct_i(document, entry_data, entry_capacity, entry_meta, em);
                }
                else {
                    value = dr_yaml_add_struct_array_i(document, entry_data, array_count, entry_meta, element_size, em);
                }
            }
        }

        if (value == 0) {
            CPE_ERROR(em, "add value %s.%s fail!", dr_meta_name(meta), dr_entry_name(entry));
            continue;
        }

        key = yaml_document_add_scalar(
            document, NULL, (yaml_char_t *) dr_entry_name(entry), -1, YAML_PLAIN_SCALAR_STYLE);
        if (key == 0) {
            CPE_ERROR(em, "add key %s.%s fail!", dr_meta_name(meta), dr_entry_name(entry));
            continue;
        }

        if (!yaml_document_append_mapping_pair(document, struct_node, key, value)) {
            dr_yaml_notify_document_error(document, em);
            continue;
        }
    }

    return struct_node;
}

struct dr_yaml_print_ctx {
    write_stream_t m_stream;
    int m_len;
};

static int dr_write_to_stream_handler(void *data, unsigned char * buf, size_t size) {
    struct dr_yaml_print_ctx * ctx = (struct dr_yaml_print_ctx *)data;
    int rv;
    
    rv = stream_write(ctx->m_stream, buf, size);

    if (rv >= 0 && ctx->m_len >= 0) ctx->m_len += rv;

    return rv;
}

static void dr_yaml_notify_emitter_error(yaml_emitter_t * emitter, error_monitor_t em) {
    switch (emitter->error) {
    case YAML_MEMORY_ERROR:
        CPE_ERROR(em, "Memory error: Not enough memory for emitting\n");
        break;
    case YAML_WRITER_ERROR:
        CPE_ERROR(em, "Writer error: %s\n", emitter->problem);
        break;
    case YAML_EMITTER_ERROR:
        CPE_ERROR(em, "Emitter error: %s\n", emitter->problem);
        break;
    default:
        /* Couldn't happen. */
        CPE_ERROR(em, "Internal error\n");
        break;
    }
}

static int dr_yaml_print_document(write_stream_t output, yaml_document_t * document, error_monitor_t em) {
    yaml_emitter_t emitter;
    struct dr_yaml_print_ctx ctx = { output, 0 };
    
    bzero(&emitter, sizeof(emitter));

    if (!yaml_emitter_initialize(&emitter)) {
        CPE_ERROR(em, "yaml emmit initialize fail!");
        return -1;
    }

    yaml_emitter_set_output(&emitter, dr_write_to_stream_handler, &ctx);
    /* yaml_emitter_set_canonical(&emitter, canonical); */
    yaml_emitter_set_unicode(&emitter, YAML_UTF8_ENCODING);
    yaml_emitter_set_indent(&emitter, 4);

    if (!yaml_emitter_open(&emitter)
        || !yaml_emitter_dump(&emitter, document)
        || !yaml_emitter_close(&emitter))
    {
        dr_yaml_notify_emitter_error(&emitter, em);
    }

    yaml_emitter_delete(&emitter);

    return ctx.m_len;
}
    
int dr_yaml_print(
    write_stream_t output,
    const void * input, size_t capacity, LPDRMETA meta,
    error_monitor_t em)
{
    int ret = 0;
    yaml_document_t document;
    
    /*初始化文档 */
    if (!yaml_document_initialize(&document, NULL, NULL, NULL, 0, 0)) {
        CPE_ERROR(em, "yaml document initialize fail!");
        return -1;
    }

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        dr_yaml_add_struct_i(&document, input, (uint32_t)capacity, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        dr_yaml_add_struct_i(&document, input, (uint32_t)capacity, meta, &logError);
    }

    if (ret == 0) {
        ret = dr_yaml_print_document(output, &document, em);
    }

    yaml_document_delete(&document);

    return ret;
}

int dr_yaml_print_array(
    write_stream_t output,
    const void * input, size_t capacity, LPDRMETA meta,
    error_monitor_t em)
{
    int ret = 0;
    yaml_document_t document;
    uint16_t count;
    
    if (output == NULL || input == NULL || meta == NULL) {
        CPE_ERROR(em, "dr_yaml_print_array: bad para!");
        return -1;
    }

    if (!yaml_document_initialize(&document, NULL, NULL, NULL, 0, 0)) {
        CPE_ERROR(em, "yaml document initialize fail!");
        return -1;
    }

    count = (uint16_t)(capacity / dr_meta_size(meta));
    
    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        dr_yaml_add_struct_array_i(&document, input, count, meta, dr_meta_size(meta), em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        dr_yaml_add_struct_array_i(&document, input, count, meta,dr_meta_size(meta),  &logError);
    }

    if (ret == 0) {
        ret = dr_yaml_print_document(output, &document, em);
    }

    yaml_document_delete(&document);

    return ret;
}

const char * dr_yaml_dump(mem_buffer_t buffer, const void * input, size_t capacity, LPDRMETA meta) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    dr_yaml_print((write_stream_t)&stream, input, capacity, meta, NULL);

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

const char * dr_yaml_dump_inline(mem_buffer_t buffer, const void * input, size_t capacity, LPDRMETA meta) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    dr_yaml_print((write_stream_t)&stream, input, capacity, meta, NULL);

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}
