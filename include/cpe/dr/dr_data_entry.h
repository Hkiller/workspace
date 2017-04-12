#ifndef CPE_DR_DATA_ENTRY_H
#define CPE_DR_DATA_ENTRY_H
#include "cpe/utils/error.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_data_entry_t dr_data_entry_find(dr_data_entry_t buff, dr_data_t data, const char * attr_name);
dr_data_entry_t dr_data_entry_search_in_source(dr_data_entry_t buff, dr_data_source_t data_source, const char * attr_name);

int dr_data_entry_set_from_entry(dr_data_entry_t to, dr_data_entry_t from, error_monitor_t em);
int dr_data_entry_set_from_string(dr_data_entry_t to, const char * value, error_monitor_t em);
int dr_data_entry_set_from_value(dr_data_entry_t to, dr_value_t from, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
