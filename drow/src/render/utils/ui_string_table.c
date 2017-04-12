#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_time.h"
#include "cpe/utils/string_utils.h"
#include "cpe/vfs/vfs_file.h"
#include "gd/app/app_context.h"
#include "ui_string_table_i.h"

ui_string_table_t ui_string_table_create(mem_allocrator_t alloc, error_monitor_t em) {
    ui_string_table_t string_table;

    string_table = mem_calloc(alloc, sizeof(struct ui_string_table));
    if (string_table == NULL) {
        CPE_ERROR(em, "ui_string_table_create: alloc fail!");
        return NULL;
    }

    string_table->m_alloc = alloc;
    string_table->m_em = em;

    mem_buffer_init(&string_table->m_format_buf, alloc);
    
    return string_table;
}

void ui_string_table_free(ui_string_table_t string_table) {
    if (string_table->m_data) {
        ui_string_table_unload(string_table);
    }

    mem_buffer_clear(&string_table->m_format_buf);

    mem_free(string_table->m_alloc, string_table);
}

int ui_string_table_load_update_data(ui_string_table_t string_table, const char * path) {
    struct ui_env_strings_head * head;
    uint32_t head_sizde;

    head = string_table->m_data;
    if (head->m_magic[0] != 'S' || head->m_magic[1] != 'T' || head->m_magic[2] != 'B' || head->m_magic[3] != 01) {
        CPE_ERROR(string_table->m_em, "ui_string_table_load: load from %s: head magic check fail!", path);
        return -1;
    }

    CPE_COPY_NTOH32(&string_table->m_record_count, &head->m_record_count);

    head_sizde = sizeof(struct ui_env_strings_head) + sizeof(struct ui_env_strings_item) * string_table->m_record_count * 2;
    if (head_sizde > string_table->m_data_size) {
        CPE_ERROR(
            string_table->m_em, "ui_string_table_load: load from %s: recount count %d, head size %d, total size %d!",
            path, string_table->m_record_count, head_sizde, (int)string_table->m_data_size);
        return -1;
    }

#ifdef CPE_BIG_ENDIAN
    string_table->m_items = (struct ui_env_strings_item *)(head + 1);
#else
    string_table->m_items = ((struct ui_env_strings_item *)(head + 1)) + string_table->m_record_count;
#endif

    string_table->m_strings = ((const char *)(string_table->m_data)) + head_sizde;

    return 0;
}

int ui_string_table_load_file(ui_string_table_t string_table, vfs_mgr_t vfs, const char * path) {
    vfs_file_t fp;
    
    if (string_table->m_data) ui_string_table_unload(string_table);
    assert(string_table->m_data == NULL);

    fp = vfs_file_open(vfs, path, "rb");
    if (fp == NULL) {
        CPE_ERROR(string_table->m_em, "ui_string_table_load: open %s fail!", path);
        return -1;
    }
    
    string_table->m_data_size = vfs_file_size(fp);
    string_table->m_data = mem_alloc(string_table->m_alloc, string_table->m_data_size);
    if (string_table->m_data == NULL) {
        CPE_ERROR(string_table->m_em, "ui_string_table_load: load from %s: alloc buf %d fail!", path, (int)string_table->m_data_size);
        vfs_file_close(fp);
        return -1;
    }
    
    if (vfs_file_read(fp, string_table->m_data, string_table->m_data_size) < (ssize_t)string_table->m_data_size) {
        CPE_ERROR(string_table->m_em, "ui_string_table_load: load from %s fail!", path);
        mem_free(string_table->m_alloc, string_table->m_data);
        string_table->m_data = NULL;
        string_table->m_data_size = 0;
        vfs_file_close(fp);
        return -1;
    }
    vfs_file_close(fp);
    fp = NULL;

    if (ui_string_table_load_update_data(string_table, path) != 0) {
        ui_string_table_unload(string_table);
        return -1;
    }

    return 0;
}

int ui_string_table_write_file(ui_string_table_t string_table, vfs_mgr_t vfs, const char * path) {
    vfs_file_t fp;
    
    if(string_table->m_data == NULL) {
        CPE_ERROR(string_table->m_em, "ui_string_table_write: not loaded!");
        return -1;
    }
    
    fp = vfs_file_open(vfs, path, "wb");
    if (fp == NULL) {
        CPE_ERROR(string_table->m_em, "ui_string_table_write: open %s fail!", path);
        return -1;
    }

    if (vfs_file_write(fp, string_table->m_data, string_table->m_data_size) < (ssize_t)string_table->m_data_size) {
        CPE_ERROR(string_table->m_em, "ui_string_table_write: write to %s fail!", path);
        vfs_file_close(fp);
        return -1;
    }

    vfs_file_close(fp);
    return 0;
}

void ui_string_table_unload(ui_string_table_t string_table) {
    if (string_table->m_data) {
        mem_free(string_table->m_alloc, string_table->m_data);
    }

    string_table->m_data = NULL;
    string_table->m_data_size = 0;
    string_table->m_record_count = 0;
    string_table->m_items = NULL;
    string_table->m_strings = NULL;
}

void * ui_string_table_data(ui_string_table_t string_table) {
    return string_table->m_data;
}

size_t ui_string_table_data_size(ui_string_table_t string_table) {
    return string_table->m_data_size;
}

int ui_string_table_load(ui_string_table_t string_table, void const * data, size_t data_size) {
    ui_string_table_unload(string_table);

    string_table->m_data = mem_alloc(string_table->m_alloc, data_size);
    if (string_table->m_data == NULL) {
        CPE_ERROR(string_table->m_em, "ui_string_table_load: alloc buffer fail, size=%d!", (int)data_size);
        return -1;
    }
    memcpy(string_table->m_data, data, data_size);
    string_table->m_data_size = data_size;
    
    if (ui_string_table_load_update_data(string_table, "memory") != 0) {
        ui_string_table_unload(string_table);
        return -1;
    }

    return 0;
}

int ui_string_table_item_cmp(void const * l, void const * r) {
    return ((int)((struct ui_env_strings_item const *)l)->m_msg_id) - ((int)((struct ui_env_strings_item const *)r)->m_msg_id);
}

uint8_t ui_string_table_message_exist(ui_string_table_t string_table, uint32_t msg_id) {
    struct ui_env_strings_item key;
    struct ui_env_strings_item * p;
    
    if(string_table->m_data == NULL) return 0;

    key.m_msg_id = msg_id;
    p = bsearch(&key, string_table->m_items, string_table->m_record_count, sizeof(key), ui_string_table_item_cmp);
    return p ? 1 : 0;
}

const char * ui_string_table_message(ui_string_table_t string_table, uint32_t msg_id) {
    struct ui_env_strings_item key;
    struct ui_env_strings_item * p;
    
    if(string_table->m_data == NULL) return "load string table fail";

    key.m_msg_id = msg_id;
    p = bsearch(&key, string_table->m_items, string_table->m_record_count, sizeof(key), ui_string_table_item_cmp);
    if (p == NULL) {
        static char s_buf[64];
        snprintf(s_buf, sizeof(s_buf), "%d", msg_id);
        return s_buf;
    }

    return string_table->m_strings + p->m_msg_pos;
}

const char * ui_string_table_message_format(ui_string_table_t string_table, uint32_t msg_id, char * args) {
    const char * msg;
    uint8_t have_arg = 0;
    const char * arg_begin;
    
    msg = ui_string_table_message(string_table, msg_id);

    while((arg_begin = strstr(msg, "$("))) {
        char arg_buf[64];
        const char * arg_end;
        uint32_t arg_len;
        uint8_t found;
        char * find_at;
        
        if (!have_arg) {
            mem_buffer_clear_data(&string_table->m_format_buf);
            have_arg = 1;
        }

        mem_buffer_append(&string_table->m_format_buf, msg, arg_begin - msg);

        /*读取参数名字 */
        arg_end = strchr(arg_begin, ')');
        if (arg_end == NULL) return msg;
        
        arg_len = arg_end - arg_begin - 2;
        if (arg_len + 1 > CPE_ARRAY_SIZE(arg_buf)) return msg;

        memcpy(arg_buf, arg_begin + 2, arg_len);
        arg_buf[arg_len] = 0;

        /*填写参数 */
        found = 0;
        find_at = args;
        while(find_at) {
            char * sep;
            char * eq;
            
            sep = strchr(find_at, ',');
            if (sep) *sep = 0;

            if ((eq = strchr(find_at, '='))) {
                char * h;
                char * e;
                char s;

                *eq = 0;

                h = cpe_str_trim_head(find_at);
                e = cpe_str_trim_tail(eq, h);
                s = *e;

                *e = 0;

                if (strcmp(h, arg_buf) == 0) {
                    char * v = cpe_str_trim_head(eq + 1);
                    char * ve = cpe_str_trim_tail(v + strlen(v), v);
                    mem_buffer_append(&string_table->m_format_buf, v, ve - v);
                    found = 1;
                }
                
                *e = s;

                *eq = '=';
            }

            if (sep) *sep = ',';

            if (found) break;
            
            if (sep) {
                find_at = sep + 1;
            }
            else {
                break;
            }
        }

        if (!found) return msg;

        msg = arg_end + 1;
    }

    if (have_arg) {
        mem_buffer_append(&string_table->m_format_buf, msg, strlen(msg));
        mem_buffer_append_char(&string_table->m_format_buf, 0);
        return (const char *)mem_buffer_make_continuous(&string_table->m_format_buf, 0);
    }
    else {
        return msg;
    }
}


const char * ui_string_table_message_format_time(
    ui_string_table_t string_table, uint32_t msg_id,
    uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t sec, uint8_t min)
{
    const char * msg;
    uint8_t have_arg = 0;
    const char * arg_begin;
    char value_buf[32];
    
    msg = ui_string_table_message(string_table, msg_id);

    while((arg_begin = strstr(msg, "$("))) {
        char arg_buf[64];
        const char * arg_end;
        uint32_t arg_len;
        
        if (!have_arg) {
            mem_buffer_clear_data(&string_table->m_format_buf);
            have_arg = 1;
        }

        mem_buffer_append(&string_table->m_format_buf, msg, arg_begin - msg);

        /*读取参数名字 */
        arg_end = strchr(arg_begin, ')');
        if (arg_end == NULL) return msg;
        
        arg_len = arg_end - arg_begin - 2;
        if (arg_len + 1 > CPE_ARRAY_SIZE(arg_buf)) return msg;

        memcpy(arg_buf, arg_begin + 2, arg_len);
        arg_buf[arg_len] = 0;

        if (strcmp(arg_buf, "year") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", year);
        }
        else if (strcmp(arg_buf, "month") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", mon);
        }
        else if (strcmp(arg_buf, "day") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", day);
        }
        else if (strcmp(arg_buf, "hour") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%2d", hour);
        }
        else if (strcmp(arg_buf, "sec") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%2d", sec);
        }
        else if (strcmp(arg_buf, "min") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%2d", min);
        }
        else {
            return msg;
        }

        mem_buffer_append(&string_table->m_format_buf, value_buf, strlen(value_buf));
        msg = arg_end + 1;
    }

    if (have_arg) {
        mem_buffer_append(&string_table->m_format_buf, msg, strlen(msg));
        mem_buffer_append_char(&string_table->m_format_buf, 0);
        return (const char *)mem_buffer_make_continuous(&string_table->m_format_buf, 0);
    }
    else {
        return msg;
    }
}

const char * ui_string_table_message_format_time_local(
    ui_string_table_t string_table, uint32_t msg_id, uint32_t t)
{
    struct tm b;
    time_t tt = t;
    localtime_r(&tt, &b);
    return ui_string_table_message_format_time(
        string_table, msg_id,
        b.tm_year + 1900, b.tm_mon + 1, b.tm_mday, b.tm_hour, b.tm_min, b.tm_sec);
}

const char * ui_string_table_message_format_time_duration(
    ui_string_table_t string_table, uint32_t msg_id, int32_t i_time_diff)
{
    const char * msg;
    uint8_t have_arg = 0;
    const char * arg_begin;
    char value_buf[32];
    uint32_t time_diff;
    uint16_t year = 0;
    uint8_t mon = 0;
    uint8_t day = 0;
    uint8_t hour = 0;
    uint8_t sec = 0;
    uint8_t min = 0;
    uint8_t msg_id_ok = 0;
    
    if (i_time_diff > 0) {
        msg_id += 6;
        time_diff = (uint32_t)i_time_diff;
    }
    else {
        time_diff = (uint32_t)(- i_time_diff);
    }

    
    if (time_diff > (365 * 30 * 24 * 60 * 60) ) { /*年 */
        year = time_diff / (365 * 30 * 24 * 60 * 60);
        time_diff -= year * (365 * 30 * 24 * 60 * 60);
        msg_id += 5;
        msg_id_ok = 1;
    }

    if (time_diff > (30 * 24 * 60 * 60) ) { /*月 */
        mon = time_diff / (30 * 24 * 60 * 60);
        time_diff -= mon * (30 * 24 * 60 * 60);
        if (!msg_id_ok) {
            msg_id += 4;
            msg_id_ok = 1;
        }
    }
    
    if (time_diff > (24 * 60 * 60) ) { /*天 */
        day = time_diff / (24 * 60 * 60);
        time_diff -= day * (24 * 60 * 60);
        if (!msg_id_ok) {
            msg_id += 3;
            msg_id_ok = 1;
        }
    }

    if (time_diff > 60 * 60 ) { /*时 */
        hour = time_diff / (60 * 60);
        time_diff -= hour * (60 * 60);
        
        if (!msg_id_ok) {
            msg_id += 2;
            msg_id_ok = 1;
        }
    }

    if (time_diff > 60 ) { /*分 */
        sec = time_diff / 60;
        time_diff -= sec * 60;
        
        if (!msg_id_ok) {
            msg_id += 1;
            msg_id_ok = 1;
        }
    }

    min = time_diff;

    msg = ui_string_table_message(string_table, msg_id);

    while((arg_begin = strstr(msg, "$("))) {
        char arg_buf[64];
        const char * arg_end;
        uint32_t arg_len;
        
        if (!have_arg) {
            mem_buffer_clear_data(&string_table->m_format_buf);
            have_arg = 1;
        }

        mem_buffer_append(&string_table->m_format_buf, msg, arg_begin - msg);

        /*读取参数名字 */
        arg_end = strchr(arg_begin, ')');
        if (arg_end == NULL) return msg;
        
        arg_len = arg_end - arg_begin - 2;
        if (arg_len + 1 > CPE_ARRAY_SIZE(arg_buf)) return msg;

        memcpy(arg_buf, arg_begin + 2, arg_len);
        arg_buf[arg_len] = 0;

        if (strcmp(arg_buf, "d-year") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", year);
        }
        else if (strcmp(arg_buf, "d-month") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", mon);
        }
        else if (strcmp(arg_buf, "d-day") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", day);
        }
        else if (strcmp(arg_buf, "d-hour") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", hour);
        }
        else if (strcmp(arg_buf, "d-sec") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", sec);
        }
        else if (strcmp(arg_buf, "d-min") == 0) {
            snprintf(value_buf, sizeof(value_buf), "%d", min);
        }
        else {
            return msg;
        }

        mem_buffer_append(&string_table->m_format_buf, value_buf, strlen(value_buf));
        msg = arg_end + 1;
    }

    if (have_arg) {
        mem_buffer_append(&string_table->m_format_buf, msg, strlen(msg));
        mem_buffer_append_char(&string_table->m_format_buf, 0);
        return (const char *)mem_buffer_make_continuous(&string_table->m_format_buf, 0);
    }
    else {
        return msg;
    }
    
}

const char * ui_string_table_message_format_time_duration_by_base(
    ui_string_table_t string_table, uint32_t msg_id, uint32_t base, uint32_t v)
{
    return ui_string_table_message_format_time_duration(
        string_table, msg_id,
        base > v ? - (int32_t)(base - v) : (int32_t)(v - base));
}
