#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/error.h"
#include "ui_string_table_builder_i.h"
#include "ui_string_table_i.h"

ui_string_table_builder_t
ui_string_table_builder_create(mem_allocrator_t alloc, error_monitor_t em) {
    ui_string_table_builder_t builder;

    builder = mem_alloc(alloc, sizeof(struct ui_string_table_builder));
    if (builder == NULL) {
        CPE_ERROR(builder->m_em, "ui_string_table_builder_create: alloc fail!");
        return NULL;
    }

    builder->m_alloc = alloc;
    builder->m_em = em;
    builder->m_last_id = 0;
    builder->m_record_count = 0;

    if (cpe_hash_table_init(
            &builder->m_msgs_by_id,
            alloc,
            (cpe_hash_fun_t) ui_string_table_builder_id_hash,
            (cpe_hash_eq_t) ui_string_table_builder_id_eq,
            CPE_HASH_OBJ2ENTRY(ui_string_table_builder_msg, m_hh_for_id),
            -1) != 0)
    {
        CPE_ERROR(builder->m_em, "ui_string_table_builder_create: init hash table fail!");
        mem_free(alloc, builder);
        return NULL;
    }

    if (cpe_hash_table_init(
            &builder->m_msgs_by_str,
            alloc,
            (cpe_hash_fun_t) ui_string_table_builder_str_hash,
            (cpe_hash_eq_t) ui_string_table_builder_str_eq,
            CPE_HASH_OBJ2ENTRY(ui_string_table_builder_msg, m_hh_for_str),
            -1) != 0)
    {
        CPE_ERROR(builder->m_em, "ui_string_table_builder_create: init hash table fail!");
        cpe_hash_table_fini(&builder->m_msgs_by_id);
        mem_free(alloc, builder);
        return NULL;
    }

    return builder;
}

void ui_string_table_builder_free(ui_string_table_builder_t builder) {
    struct cpe_hash_it msg_it;
    ui_string_table_builder_msg_t msg;

    cpe_hash_it_init(&msg_it, &builder->m_msgs_by_id);
    msg = cpe_hash_it_next(&msg_it);
    while (msg) {
        ui_string_table_builder_msg_t next = cpe_hash_it_next(&msg_it);

        cpe_hash_table_remove_by_ins(&builder->m_msgs_by_id, msg);
        cpe_hash_table_remove_by_ins(&builder->m_msgs_by_str, msg);
        mem_free(builder->m_alloc, msg);

        assert(builder->m_record_count > 0);
        builder->m_record_count--;
        
        msg = next;
    }

    cpe_hash_table_fini(&builder->m_msgs_by_id);
    cpe_hash_table_fini(&builder->m_msgs_by_str);

    mem_free(builder->m_alloc, builder);
}

uint32_t ui_string_table_builder_msg_alloc(ui_string_table_builder_t builder, const char * str_msg) {
    struct ui_string_table_builder_msg key;
    ui_string_table_builder_msg_t msg;

    if (str_msg == NULL || str_msg[0] == 0) return 0;
    
    key.m_str = str_msg;
    msg = cpe_hash_table_find(&builder->m_msgs_by_str, &key);
    if (msg == NULL) {
        size_t msg_len = strlen(str_msg) + 1;
        
        msg = mem_alloc(builder->m_alloc, sizeof(struct ui_string_table_builder_msg) + msg_len);
        if (msg == NULL) {
            CPE_ERROR(builder->m_em, "ui_string_table_builder_msg_alloc: alloc fail!");
            return 0;
        }

        msg->m_id = ++builder->m_last_id;
        msg->m_str = (void*)(msg + 1);
        memcpy(msg + 1, str_msg, msg_len);
        msg->m_ref_count = 0;

        cpe_hash_entry_init(&msg->m_hh_for_id);
        if (cpe_hash_table_insert_unique(&builder->m_msgs_by_id, msg) != 0) {
            CPE_ERROR(builder->m_em, "ui_string_table_builder_msg_alloc: insert id duplicate!");
            mem_free(builder->m_alloc, msg);
            return 0;
        }
                                     
        cpe_hash_entry_init(&msg->m_hh_for_str);
        if (cpe_hash_table_insert_unique(&builder->m_msgs_by_str, msg) != 0) {
            CPE_ERROR(builder->m_em, "ui_string_table_builder_msg_alloc: insert id duplicate!");
            cpe_hash_table_remove_by_ins(&builder->m_msgs_by_id, msg);
            mem_free(builder->m_alloc, msg);
            return 0;
        }

        builder->m_record_count++;
    }

    msg->m_ref_count++;
    return msg->m_id;
}

void ui_string_table_builder_msg_free(ui_string_table_builder_t builder, uint32_t msg_id) {
    struct ui_string_table_builder_msg key;
    ui_string_table_builder_msg_t msg;

    assert(msg_id);
    
    key.m_id = msg_id;
    msg = cpe_hash_table_find(&builder->m_msgs_by_id, &key);

    if (msg == NULL) {
        CPE_ERROR(builder->m_em, "ui_string_table_builder_msg_free: msg %d not exist!", msg_id);
        assert(0);
        return;
    }

    assert(msg->m_ref_count > 0);

    msg->m_ref_count--;
    if (msg->m_ref_count == 0) {
        cpe_hash_table_remove_by_ins(&builder->m_msgs_by_id, msg);
        cpe_hash_table_remove_by_ins(&builder->m_msgs_by_str, msg);
        mem_free(builder->m_alloc, msg);
        assert(builder->m_record_count > 0);
        builder->m_record_count--;
    }
}

const char * ui_string_table_builder_msg_get(ui_string_table_builder_t builder, uint32_t msg_id) {
    struct ui_string_table_builder_msg key;
    ui_string_table_builder_msg_t msg;
    
    key.m_id = msg_id;
    msg = cpe_hash_table_find(&builder->m_msgs_by_id, &key);

    return msg ? msg->m_str : NULL;
}

int ui_string_table_builder_build(ui_string_table_builder_t builder, ui_string_table_t strings) {
    struct ui_env_strings_head * head;
    uint32_t head_size;
    struct cpe_hash_it msg_it;
    ui_string_table_builder_msg_t msg;
    char * data;
    uint32_t data_size;
    struct ui_env_strings_item * items;
    struct ui_env_strings_item * items_swap;
    uint32_t i;
    uint32_t write_pos;

    head_size = sizeof(struct ui_env_strings_head) + sizeof(struct ui_env_strings_item) * builder->m_record_count * 2;

    data_size = head_size;
    cpe_hash_it_init(&msg_it, &builder->m_msgs_by_id);
    while((msg = cpe_hash_it_next(&msg_it))) {
        data_size += strlen(msg->m_str) + 1;
    }

    data = mem_alloc(strings->m_alloc, data_size);
    if (data == NULL) {
        CPE_ERROR(builder->m_em, "ui_string_table_builder_build: alloc data fail, size=%d", (int)data_size);
        return -1;
    }
    
    head = (struct ui_env_strings_head *)data;
    head->m_magic[0] = 'S';
    head->m_magic[1] = 'T';
    head->m_magic[2] = 'B';
    head->m_magic[3] = 01;
    CPE_COPY_HTON32(&head->m_record_count, &builder->m_record_count);

#ifdef CPE_BIG_ENDIAN
    items = (struct ui_env_strings_item *)(head + 1);
    items_swap = items + builder->m_record_count;
#else
    items_swap = (struct ui_env_strings_item *)(head + 1);
    items = items_swap + builder->m_record_count;
#endif

    /*构造数据 */
    i = 0;
    write_pos = 0;
    cpe_hash_it_init(&msg_it, &builder->m_msgs_by_id);
    while((msg = cpe_hash_it_next(&msg_it))) {
        uint32_t len = strlen(msg->m_str) + 1;
        
        items[i].m_msg_id = msg->m_id;
        items[i].m_msg_pos = write_pos;

        memcpy(data + head_size + write_pos, msg->m_str, len);
        write_pos += len;
        i++;
    }

    qsort(items, builder->m_record_count, sizeof(items[0]), ui_string_table_item_cmp);
    
    assert(write_pos + head_size == data_size);
    
    for(i = 0; i < builder->m_record_count; ++i) {
        CPE_COPY_ENDIAN32(&items_swap[i].m_msg_id, &items[i].m_msg_id);
        CPE_COPY_ENDIAN32(&items_swap[i].m_msg_pos, &items[i].m_msg_pos);
    }
    
    /*提交数据 */
    ui_string_table_unload(strings);
    strings->m_data = data;
    strings->m_data_size = data_size;
    if (ui_string_table_load_update_data(strings, "builder") != 0) {
        ui_string_table_unload(strings);
        return -1;
    }
    
    return 0;
}

uint32_t ui_string_table_builder_str_hash(ui_string_table_builder_msg_t msg) {
    return cpe_hash_str(msg->m_str, strlen(msg->m_str));
}

int ui_string_table_builder_str_eq(ui_string_table_builder_msg_t l, ui_string_table_builder_msg_t r) {
    return strcmp(l->m_str, r->m_str) == 0;
}

uint32_t ui_string_table_builder_id_hash(ui_string_table_builder_msg_t msg) {
    return msg->m_id;
}

int ui_string_table_builder_id_eq(ui_string_table_builder_msg_t l, ui_string_table_builder_msg_t r) {
    return l->m_id == r->m_id;
}
