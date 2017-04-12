#ifndef UI_MODEL_EVT_COLLECTOR_H
#define UI_MODEL_EVT_COLLECTOR_H
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_data_evt_collector_t ui_data_evt_collector_create(ui_data_mgr_t mgr, const char * name, const char * value); 
void ui_data_evt_collector_free(ui_data_evt_collector_t collector);

#ifdef __cplusplus
}
#endif

#endif

