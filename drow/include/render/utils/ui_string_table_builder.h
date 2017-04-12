#ifndef DROW_UI_STRING_TABLE_BUILDER_H
#define DROW_UI_STRING_TABLE_BUILDER_H
#include "cpe/utils/utils_types.h"
#include "cpe/vfs/vfs_types.h"
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_string_table_builder_t ui_string_table_builder_create(mem_allocrator_t alloc, error_monitor_t em);
void ui_string_table_builder_free(ui_string_table_builder_t builder);

uint32_t ui_string_table_builder_msg_alloc(ui_string_table_builder_t builder, const char * msg);
void ui_string_table_builder_msg_free(ui_string_table_builder_t builder, uint32_t msg_id);
const char * ui_string_table_builder_msg_get(ui_string_table_builder_t builder, uint32_t msg_id);

int ui_string_table_builder_build(ui_string_table_builder_t builder, ui_string_table_t strings);

#ifdef __cplusplus
}
#endif

#endif

