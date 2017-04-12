#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/xcalc/xcalc_token.h"
#include "ui_data_evt_collector_i.h"
#include "ui_data_src_i.h"
#include "ui_data_src_res_i.h"

ui_data_evt_collector_t
ui_data_evt_collector_create(ui_data_mgr_t mgr, const char * name, const char * value) {
    ui_data_evt_collector_t evt_collector;
    size_t name_len = strlen(name) + 1;
    size_t value_len = strlen(value) + 1;

    evt_collector = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_evt_collector) + name_len + value_len);
    if (evt_collector == NULL) {
        CPE_ERROR(mgr->m_em, "ui_data_evt_collector: alloc fail!");
        return NULL;
    }

    evt_collector->m_mgr = mgr;
    evt_collector->m_name = (void*)(evt_collector + 1);
    evt_collector->m_value = evt_collector->m_name + name_len;

    memcpy((void*)evt_collector->m_name, name, name_len);
    memcpy((void*)evt_collector->m_value, value, value_len);

    TAILQ_INSERT_TAIL(&mgr->m_evt_collectors, evt_collector, m_next);

    return evt_collector;
}

void ui_data_evt_collector_free(ui_data_evt_collector_t collector) {
    TAILQ_REMOVE(&collector->m_mgr->m_evt_collectors, collector, m_next);
    mem_free(collector->m_mgr->m_alloc, collector);
}

int ui_data_src_collect_res_from_event(ui_data_src_t user, const char * event_name, const char * args) {
    ui_data_evt_collector_t evt_collector;
    struct xcomputer_find_value_from_str_ctx calc_ctx = { args, ',', '=' };
    struct xcomputer_args calc_args = { (void*)&calc_ctx, xcomputer_find_value_from_str };
    xcomputer_t computer = user->m_mgr->m_computer;
    int rv = 0;

    TAILQ_FOREACH(evt_collector, &user->m_mgr->m_evt_collectors, m_next) {
        xtoken_t value;
        char value_buf[256];
        const char * res;
        
        if (strcmp(evt_collector->m_name, event_name) != 0) continue;

        value = xcomputer_compute(computer, evt_collector->m_value, &calc_args);
        if (value == NULL) {
            rv = -1;
            continue;
        }

        res = xtoken_to_str(value, value_buf, sizeof(value_buf));

        if (res && res[0]) {
            if (ui_data_src_res_create_by_path(user, res) == NULL) {
                rv = -1;
            }
        }

        xcomputer_free_token(computer, value);
    }

    return rv;
}
