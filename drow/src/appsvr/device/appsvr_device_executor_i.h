#ifndef APPSVR_STATISTICS_DEVICE_EXECUTOR_H
#define APPSVR_STATISTICS_DEVICE_EXECUTOR_H
#include "appsvr_device_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int appsvr_device_query_path(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size, dr_data_t * r, mem_buffer_t result_alloc);
int appsvr_device_query_info(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size, dr_data_t * r, mem_buffer_t result_alloc);
int appsvr_device_query_network_state(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size, dr_data_t * r, mem_buffer_t result_alloc);
int appsvr_push_executor_init(appsvr_device_module_t module);
void appsvr_push_executor_fini(appsvr_device_module_t module);

#ifdef __cplusplus
}
#endif

#endif
