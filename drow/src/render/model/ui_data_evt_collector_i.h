#ifndef UI_MODEL_COLLECTOR_EVT_I_H
#define UI_MODEL_COLLECTOR_EVT_I_H
#include "render/model/ui_data_evt_collector.h"
#include "ui_data_mgr_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_evt_collector {
    ui_data_mgr_t m_mgr;
    TAILQ_ENTRY(ui_data_evt_collector) m_next;
    const char * m_name;
    const char * m_value;
};

#ifdef __cplusplus
}
#endif

#endif
