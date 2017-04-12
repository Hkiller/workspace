#ifndef GD_APP_ATTR_REQUEST_H
#define GD_APP_ATTR_REQUEST_H
#include "app_attr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

app_attr_request_t app_attr_request_create(app_attr_module_t module);
void app_attr_request_free(app_attr_request_t request);

app_attr_module_t app_attr_request_module(app_attr_request_t request);
uint32_t app_attr_request_id(app_attr_request_t request);

int app_attr_request_remove_by_id(app_attr_module_t module, uint32_t request_id);
int app_attr_request_remove_by_ctx(app_attr_module_t module, void * ctx);
    
typedef void (*app_attr_request_result_process_fun_t)(void * ctx, app_attr_request_t request, void * arg);
int app_attr_request_set_result_processor(
    app_attr_request_t request, void * ctx,
    app_attr_request_result_process_fun_t on_result, void * arg, void (*arg_free)(void *));

uint8_t app_attr_request_is_success(app_attr_request_t request);
    
int app_attr_request_set_done(app_attr_request_t request, uint8_t is_success);

#ifdef __cplusplus
}
#endif

#endif
