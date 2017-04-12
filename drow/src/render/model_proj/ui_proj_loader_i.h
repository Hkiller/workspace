#ifndef UI_MODEL_BIN_LOADER_I_H
#define UI_MODEL_BIN_LOADER_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "render/model_proj/ui_proj_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_proj_loader {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    char * m_root;
};

#ifdef __cplusplus
}
#endif

#endif

