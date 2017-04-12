#ifndef DROW_UI_STRING_TABLE_H
#define DROW_UI_STRING_TABLE_H
#include "cpe/utils/utils_types.h"
#include "cpe/vfs/vfs_types.h"
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_string_table_t ui_string_table_create(mem_allocrator_t alloc, error_monitor_t em);
void ui_string_table_free(ui_string_table_t string_table);

int ui_string_table_load_file(ui_string_table_t string_table, vfs_mgr_t vfs, const char * path);
int ui_string_table_write_file(ui_string_table_t string_table, vfs_mgr_t vfs, const char * path);
void ui_string_table_unload(ui_string_table_t string_table);

void * ui_string_table_data(ui_string_table_t string_table);
size_t ui_string_table_data_size(ui_string_table_t string_table);
int ui_string_table_load(ui_string_table_t string_table, void const * data, size_t data_size);

uint8_t ui_string_table_message_exist(ui_string_table_t string_table, uint32_t msg_id);

const char * ui_string_table_message(ui_string_table_t string_table, uint32_t msg_id);
const char * ui_string_table_message_format(ui_string_table_t string_table, uint32_t msg_id, char * args);

const char * ui_string_table_message_format_time(
    ui_string_table_t string_table, uint32_t msg_id,
    uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t sec, uint8_t min);

const char * ui_string_table_message_format_time_local(
    ui_string_table_t string_table, uint32_t msg_id, uint32_t t);

const char * ui_string_table_message_format_time_duration(
    ui_string_table_t string_table, uint32_t msg_id_base, int32_t time_diff);

const char * ui_string_table_message_format_time_duration_by_base(
    ui_string_table_t string_table, uint32_t msg_id_base, uint32_t base, uint32_t v);
    
#ifdef __cplusplus
}
#endif

#endif

