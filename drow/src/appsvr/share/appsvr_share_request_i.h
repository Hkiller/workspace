#ifndef APPSVR_SHARE_REQUEST_I_H
#define APPSVR_SHARE_REQUEST_I_H
#include "appsvr/share/appsvr_share_request.h"
#include "appsvr_share_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum appsvr_share_request_state {
    appsvr_share_request_init, /*刚刚创建 */
    appsvr_share_request_working, /*正在提交 */
    appsvr_share_request_done, /*提交结束 */
} appsvr_share_request_state_t;

struct appsvr_share_request {
    appsvr_share_module_t m_module;
    TAILQ_ENTRY(appsvr_share_request) m_next_for_module;
    appsvr_share_adapter_t m_adapter;
    TAILQ_ENTRY(appsvr_share_request) m_next_for_adapter;
    appsvr_share_request_state_t m_state;
    TAILQ_ENTRY(appsvr_share_request) m_next_for_state;
    uint32_t m_id;

    /*结果回调 */
    void * m_result_ctx;
    appsvr_share_request_result_process_fun_t m_result_processor;
    void * m_result_arg;
    void (*m_result_arg_free)(void *);
    
    uint8_t m_is_success;
    appsvr_share_request_block_list_t m_blocks;
};

void appsvr_share_request_real_free(appsvr_share_request_t request);    
void appsvr_share_request_tick(appsvr_share_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
