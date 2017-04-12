#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_calc.h"
#include "gd/app/app_context.h"
#include "plugin/app_env/plugin_app_env_executor.h"
#include "appsvr_damai_module_i.h"

static void appsvr_damai_set_userinfo(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    appsvr_damai_module_t module = (appsvr_damai_module_t)ctx;
    char req_buf[128];
    size_t n;
    struct dr_data_source data_source;
    
    data_source.m_data.m_meta = req_meta;
    data_source.m_data.m_data = (void*)req_data;
    data_source.m_data.m_size = req_size;
    data_source.m_next = NULL;
    
    n = snprintf(req_buf, sizeof(req_buf), "{");
    
    n += snprintf(
        req_buf + n, sizeof(req_buf) - n, "\"service\"=\"%s\"",
        dr_calc_str_with_dft(gd_app_tmp_buffer(module->m_app), module->m_computer, module->m_userinfo_service, &data_source, "0"));

    n += snprintf(
        req_buf + n, sizeof(req_buf) - n, ",\"role\"=\"%s\"",
        dr_calc_str_with_dft(gd_app_tmp_buffer(module->m_app), module->m_computer, module->m_userinfo_role, &data_source, "0"));

    n += snprintf(
        req_buf + n, sizeof(req_buf) - n, ",\"grade\"=\"%s\"",
        dr_calc_str_with_dft(gd_app_tmp_buffer(module->m_app), module->m_computer, module->m_userinfo_grade, &data_source, "1"));
    
    n += snprintf(req_buf + n, sizeof(req_buf) - n, "}");

    CPE_INFO(module->m_em, "appsvr_damai_set_userinfo: req=%s", req_buf);
    
    appsvr_damai_backend_set_userinfo(module, req_buf);
}

int appsvr_damai_set_userinfo_init(appsvr_damai_module_t module) {
    assert(module->m_userinfo_source_meta);
    
    module->m_set_userinfo_executor = 
        plugin_app_env_executor_create_oneway(
            module->m_app_env, module->m_userinfo_source_meta, module, appsvr_damai_set_userinfo);
    if (module->m_set_userinfo_executor == NULL) {
        CPE_ERROR(module->m_em, "appsvr_damai_set_userinfo_init: create executor fail!");
        return -1;
    }
    
    return 0;
}

void appsvr_damai_set_userinfo_fini(appsvr_damai_module_t module) {
    if (module->m_set_userinfo_executor) {
        plugin_app_env_executor_free(module->m_set_userinfo_executor);
        module->m_set_userinfo_executor = NULL;
    }
}
