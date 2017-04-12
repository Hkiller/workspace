#ifndef APPSVR_SHARE_REQUEST_H
#define APPSVR_SHARE_REQUEST_H
#include "appsvr_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_share_request_t appsvr_share_request_create(appsvr_share_adapter_t adapter);
void appsvr_share_request_free(appsvr_share_request_t request);

uint32_t appsvr_share_request_id(appsvr_share_request_t request);

appsvr_share_request_t appsvr_share_request_first(appsvr_share_adapter_t adapter);

int appsvr_share_request_append_str(appsvr_share_request_t request, appsvr_share_request_block_category_t category, const char * str);
int appsvr_share_request_append_data(appsvr_share_request_t request, appsvr_share_request_block_category_t category, void const * data, size_t data_size);

typedef void (*appsvr_share_request_result_process_fun_t)(void * ctx, appsvr_share_request_t request, void * arg);
    
int appsvr_share_request_set_result_processor(
    appsvr_share_request_t request, void * ctx,
    appsvr_share_request_result_process_fun_t on_result, void * arg, void (*arg_free)(void *));

uint8_t appsvr_share_request_is_success(appsvr_share_request_t request);
    
int appsvr_share_request_set_done(appsvr_share_request_t request, uint8_t is_success);
    
#ifdef __cplusplus
}
#endif

#endif
