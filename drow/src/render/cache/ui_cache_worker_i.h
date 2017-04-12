#ifndef UI_CACHE_WALKER_I_H
#define UI_CACHE_WALKER_I_H
#include "ui_cache_manager_i.h"
#include "render/cache/ui_cache_worker.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_worker {
    ui_cache_manager_t m_cacme_mgr;
    TAILQ_ENTRY(ui_cache_worker) m_next_for_mgr;
    uint8_t m_is_runing;
    pthread_t m_thread;
};

void ui_cache_worker_free_all(ui_cache_manager_t mgr);
    
#ifdef __cplusplus
}
#endif

#endif
