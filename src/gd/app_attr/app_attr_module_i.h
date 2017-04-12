#ifndef APP_ATTR_MODULE_I_H
#define APP_ATTR_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/xcalc/xcalc_types.h"
#include "gd/app/app_types.h"
#include "gd/app_attr/app_attr_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct app_attr_attr_binding * app_attr_attr_binding_t;

typedef TAILQ_HEAD(app_attr_provider_list, app_attr_provider) app_attr_provider_list_t;
typedef TAILQ_HEAD(app_attr_synchronizer_list, app_attr_synchronizer) app_attr_synchronizer_list_t;
typedef TAILQ_HEAD(app_attr_attr_list, app_attr_attr) app_attr_attr_list_t;
typedef TAILQ_HEAD(app_attr_attr_binding_list, app_attr_attr_binding) app_attr_attr_binding_list_t;
typedef TAILQ_HEAD(app_attr_request_list, app_attr_request) app_attr_request_list_t;
typedef TAILQ_HEAD(app_attr_formula_list, app_attr_formula) app_attr_formula_list_t;

struct app_attr_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    xcomputer_t m_computer;
    app_attr_provider_list_t m_providers;
    struct cpe_hash_table m_attrs;

    uint32_t m_request_max_id;
    uint32_t m_request_count;
    app_attr_request_list_t m_requests_to_process;
    app_attr_request_list_t m_requests;
    
    app_attr_synchronizer_list_t m_synchronizer_to_process;
    
    app_attr_request_list_t m_free_requests;
    app_attr_formula_list_t m_free_formulas;
    app_attr_attr_binding_list_t m_free_attr_bindings;
};

#ifdef __cplusplus
}
#endif

#endif
