#ifndef APP_ATTR_REQUEST_I_H
#define APP_ATTR_REQUEST_I_H
#include "gd/app_attr/app_attr_request.h"
#include "app_attr_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum app_attr_request_state {
    app_attr_request_init, /*刚刚创建 */
    app_attr_request_waiting, /*正在提交 */
    app_attr_request_check, /*等待检查结果 */
    app_attr_request_done, /*提交结束 */
} app_attr_request_state_t;

struct app_attr_request {
    app_attr_module_t m_module;
    TAILQ_ENTRY(app_attr_request) m_next_for_module;
    app_attr_request_state_t m_state;
    TAILQ_ENTRY(app_attr_request) m_next_for_state;
    uint32_t m_id;
    app_attr_formula_list_t m_formulas;
    app_attr_attr_binding_list_t m_attrs;

    /*结果回调 */
    void * m_result_ctx;
    app_attr_request_result_process_fun_t m_result_processor;
    void * m_result_arg;
    void (*m_result_arg_free)(void *);
    
    uint8_t m_is_success;
};

void app_attr_request_real_free(app_attr_request_t request);    
void app_attr_request_tick(app_attr_module_t module);
void app_attr_request_set_state(app_attr_request_t request, app_attr_request_state_t state);
    
#ifdef __cplusplus
}
#endif

#endif
