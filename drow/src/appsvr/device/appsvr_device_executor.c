#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "plugin/app_env/plugin_app_env_executor.h"
#include "appsvr_device_executor_i.h"

int appsvr_device_query_path(
    void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size,
    dr_data_t * r, mem_buffer_t result_alloc)
{
    appsvr_device_module_t module = (appsvr_device_module_t)ctx;
    size_t capacity = sizeof(APPSVR_DEVICE_PATH);
    dr_data_t result;
    APPSVR_DEVICE_QUERY_PATH const * req = (APPSVR_DEVICE_QUERY_PATH const *)req_data;

    mem_buffer_clear_data(result_alloc);
    result = (dr_data_t)mem_buffer_alloc(result_alloc, sizeof(struct dr_data) + capacity);

    result->m_meta = module->m_meta_path_info;
    result->m_data = (void*)(result + 1);
    result->m_size = capacity;

    appsvr_device_backend_set_path_info(module, req, (APPSVR_DEVICE_PATH *)result->m_data);

    if (r) *r = result;
    
    return 0;
}

int appsvr_device_query_info(
    void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size,
    dr_data_t * r, mem_buffer_t result_alloc)
{
    appsvr_device_module_t module = (appsvr_device_module_t)ctx;
    size_t capacity = sizeof(APPSVR_DEVICE_INFO);
    dr_data_t result;

    mem_buffer_clear_data(result_alloc);
    result = (dr_data_t)mem_buffer_alloc(result_alloc, sizeof(struct dr_data) + capacity);

    result->m_meta = module->m_meta_device_info;
    result->m_data = (void*)(result + 1);
    result->m_size = capacity;

    appsvr_device_backend_set_device_info(module, (APPSVR_DEVICE_INFO *)result->m_data);
    
    if (r) *r = result;
    
    return 0;
}

int appsvr_device_query_network_state(
    void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size,
    dr_data_t * r, mem_buffer_t result_alloc)
{
    appsvr_device_module_t module = (appsvr_device_module_t)ctx;
    size_t capacity = sizeof(APPSVR_DEVICE_NETWORK_INFO);
    dr_data_t result;
    
    mem_buffer_clear_data(result_alloc);
    result = (dr_data_t)mem_buffer_alloc(result_alloc, sizeof(struct dr_data) + capacity);

    result->m_meta = module->m_meta_network_info;
    result->m_data = (void*)(result + 1);
    result->m_size = capacity;

    if (appsvr_device_backend_set_network_state(module, (APPSVR_DEVICE_NETWORK_INFO *)result->m_data) != 0) return -1;
    
    if (r) *r = result;
    
    return 0;
}
