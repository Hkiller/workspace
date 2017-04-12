#include "yajl/yajl_tree.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/random.h"
#include "cpe/utils/string_utils.h"
#include "gd/net_trans/net_trans_task.h"
#include "appsvr/account/appsvr_account_adapter.h"
#include "appsvr_weixin_module_i.h"
#include "cpe/pal/pal_limits.h"

static void appsvr_weixin_send_error_response(appsvr_weixin_module_t module, int err, const char * errstr);

static int appsvr_weixin_account_relogin_start(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_RELOGIN const * req) {
    appsvr_weixin_module_t module = *(appsvr_weixin_module_t*)appsvr_account_adapter_data(adapter);
    
	return appsvr_weixin_backend_start_login(module, 1);
}

static int appsvr_weixin_account_login_start(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_LOGIN const * req) {
    appsvr_weixin_module_t module = *(appsvr_weixin_module_t*)appsvr_account_adapter_data(adapter);

    module->m_login_session = cpe_rand_dft(UINT32_MAX);
	return appsvr_weixin_backend_start_login(module, 0);
}

int appsvr_weixin_login_init(appsvr_weixin_module_t module) {
    module->m_account_adapter = 
        appsvr_account_adapter_create(
            module->m_account_module, appsvr_account_weixin, "weixin",
            sizeof(appsvr_weixin_module_t), appsvr_weixin_account_login_start, appsvr_weixin_account_relogin_start);
    if (module->m_account_adapter == NULL) {
        CPE_ERROR(module->m_em, "appsvr_weixin_login_init: create adapter fail!");
        return -1;
    }
    
    *(appsvr_weixin_module_t*)appsvr_account_adapter_data(module->m_account_adapter) = module;
    
    return 0;
}

void appsvr_weixin_login_fini(appsvr_weixin_module_t module) {
    if (module->m_account_adapter) {
        appsvr_account_adapter_free(module->m_account_adapter);
        module->m_account_adapter = NULL;
    }
}

static void appsvr_weixin_login_code_to_token_on_result(net_trans_task_t task, void * ctx) {
    appsvr_weixin_module_t module = ctx;
    const char * json_text;
    char error_buf[128];
    yajl_val data_tree;
    yajl_val val, val2;
    APPSVR_ACCOUNT_LOGIN_RESULT login_result;
    bzero(&login_result, sizeof(login_result));

    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            module->m_em, "weixin: code to token: task execute fail, result=%s, errno=%d!",
            net_trans_task_result_str(net_trans_task_result(task)),
            net_trans_task_errno(task));
        appsvr_weixin_send_error_response(module, -1, "");
        return;
    }

    json_text = net_trans_task_buffer_to_string(task);
    if (json_text == NULL) {
        CPE_ERROR(module->m_em, "weixin: code to token: task execute fail, no result json data!");
        appsvr_weixin_send_error_response(module, -1, "");
        return;
    }

    data_tree  = yajl_tree_parse(json_text, error_buf, sizeof(error_buf));
    if (data_tree == NULL){
        CPE_ERROR(module->m_em, "weixin: parse result fail\n%s", json_text);
        appsvr_weixin_send_error_response(module, -1, "");
        return;
    }

    if ((val = yajl_tree_get_2(data_tree, "errcode", yajl_t_number))) {
        int svr_error = yajl_get_integer(val);
        if (svr_error != 0) {
            if ((val = yajl_tree_get_2(data_tree, "errmsg", yajl_t_string)) == NULL) {
                CPE_ERROR(module->m_em, "weixin: parse result: get error fail\n%s", json_text);
                appsvr_weixin_send_error_response(module, -1, "");
                return;
            }
            else {
                CPE_ERROR(
                    module->m_em, "weixin: parse result: server return error: %d (%s)",
                    svr_error, yajl_get_string(val));
                appsvr_weixin_send_error_response(module, -1, "");
                return;
            }
        }
    }

    bzero(&login_result, sizeof(login_result));

    if ((val = yajl_tree_get_2(data_tree, "openid", yajl_t_string)) == NULL) {
        CPE_ERROR(module->m_em, "weixin: parse result: get openid fail\n%s", json_text);
        appsvr_weixin_send_error_response(module, -1, "");
        return;
    }

    if ((val2 = yajl_tree_get_2(data_tree, "access_token", yajl_t_string)) == NULL) {
        CPE_ERROR(module->m_em, "weixin: parse result: get access_token fail\n%s", json_text);
        appsvr_weixin_send_error_response(module, -1, "");
        return;
    }

    CPE_INFO(module->m_em, "weixin: openid: %s", yajl_get_string(val));
    CPE_INFO(module->m_em, "weixin: access_token: %s", yajl_get_string(val2));
    
    snprintf(login_result.token, sizeof(login_result.token), "%s+%s", yajl_get_string(val), yajl_get_string(val2));

    if ((val = yajl_tree_get_2(data_tree, "expires_in", yajl_t_number)) == NULL) {
        CPE_ERROR(module->m_em, "weixin: parse result: get expires_in fail\n%s", json_text);
        appsvr_weixin_send_error_response(module, -1, "");
        return;
    }
    login_result.expires_in = yajl_get_integer(val);
    
    appsvr_account_adapter_notify_login_result(module->m_account_adapter, &login_result);
}

void appsvr_weixin_notify_login_result(
    appsvr_weixin_module_t module, const char * code, uint32_t session, int err, const char * errstr)
{
    char req_buf[256];
    net_trans_task_t task;
    
    if (err != 0) {
        appsvr_weixin_send_error_response(module, err, errstr);
        return;
    }

    /*构造请求 */
    snprintf(req_buf, sizeof(req_buf),
             "https://api.weixin.qq.com/sns/oauth2/access_token?appid=%s&secret=%s&code=%s&grant_type=authorization_code",
             module->m_appid, module->m_secret, code);
    if (module->m_debug) {
        CPE_INFO(module->m_em, "weixin: code to token: request is %s!", req_buf);
    }

    /*创建http请求 */
    task = net_trans_task_create(module->m_trans_group, 0);
    if (task == NULL) {
        CPE_ERROR(module->m_em, "weixin: code to token: create task fail!");
        goto SEND_REQ_FAIL;
    }

    net_trans_task_set_commit_op(task, appsvr_weixin_login_code_to_token_on_result, module, NULL);

    /*发送请求 */
    if (net_trans_task_set_get(task, req_buf) != 0) {
        CPE_ERROR(module->m_em, "weixin: code to token: set request get %s fail!", req_buf);
        goto SEND_REQ_FAIL;
    }

    if (net_trans_task_start(task) != 0) {
        CPE_ERROR(module->m_em, "weixin: code to token: start request get %s fail!", req_buf);
        goto SEND_REQ_FAIL;
    }

    return;

SEND_REQ_FAIL:
    appsvr_weixin_send_error_response(module, -1, "");
    return;
}

static void appsvr_weixin_send_error_response(appsvr_weixin_module_t module, int err, const char * errstr) {
    APPSVR_ACCOUNT_LOGIN_RESULT login_result;
    bzero(&login_result, sizeof(login_result));
    
    if (err == 9999) {
        login_result.result = appsvr_account_login_not_install;
    }
    else if(err == -2) {
        login_result.result = appsvr_account_login_canceled;
    }
    else if(err == -4) {
        login_result.result = appsvr_account_login_not_authorization;
    }
    else {
        login_result.result = err;
    }

    login_result.error_code = err;
    cpe_str_dup(login_result.error_msg, sizeof(login_result.error_msg), errstr);
    
    CPE_ERROR(module->m_em, "weixin: cappsvr_weixin_send_error_response login_result.result=%d,ogin_result.error_msg=%s",(int)login_result.result,login_result.error_msg);
    appsvr_account_adapter_notify_login_result(module->m_account_adapter, &login_result);
}
