#ifndef APPSVR_SHARE_MODULE_I_H
#define APPSVR_SHARE_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "appsvr/share/appsvr_share_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(appsvr_share_adapter_list, appsvr_share_adapter) appsvr_share_adapter_list_t;
typedef TAILQ_HEAD(appsvr_share_request_list, appsvr_share_request) appsvr_share_request_list_t;
typedef TAILQ_HEAD(appsvr_share_request_block_list, appsvr_share_request_block) appsvr_share_request_block_list_t;

struct appsvr_share_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;

    appsvr_share_adapter_list_t m_adapters;

    uint32_t m_request_max_id;
    uint32_t m_request_count;
    appsvr_share_request_list_t m_requests_to_process;
    appsvr_share_request_list_t m_requests;
    appsvr_share_request_list_t m_free_requests;
    appsvr_share_request_block_list_t m_free_request_blocks;
};

#ifdef __cplusplus
}
#endif

#endif
