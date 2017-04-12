#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_group.h"
#include "gd/net_trans/net_trans_task.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "account_svr_backend_damai.h"
#include "yajl/yajl_tree.h"

int account_svr_backend_damai_init(account_svr_backend_t backend, cfg_t cfg) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_damai_t damai = account_svr_backend_data(backend);
    const char * str_value;

    str_value = cfg_get_string(cfg, "app-id", NULL);
    if (str_value == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_damai_init: app-id not configured!");
        return -1;
    }
    damai->m_app_id = cpe_str_mem_dup(svr->m_alloc, str_value);
    if (damai->m_app_id == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_damai_init: dup app-id %s fail!", str_value);
        return -1;
    }
    
    damai->m_trans_group = net_trans_group_create(svr->m_trans_mgr, "damai");
    if (damai->m_trans_group == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_damai_init: crate trans group fail!");
        mem_free(svr->m_alloc, damai->m_app_id);
        damai->m_app_id = NULL;
        return -1;
    }

    return 0;
}

void account_svr_backend_damai_fini(account_svr_backend_t backend) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_damai_t damai = account_svr_backend_data(backend);

    mem_free(svr->m_alloc, damai->m_app_id);
    damai->m_app_id = NULL;

    net_trans_group_free(damai->m_trans_group);
    damai->m_trans_group = NULL;
}

static int account_svr_backend_damai_parse_result(account_svr_t svr, SVR_ACCOUNT_LOGIN_INFO * login_info, const char * json_text) {
    char error_buf[128];
    yajl_val data_tree;
    yajl_val val;

    data_tree  = yajl_tree_parse(json_text, error_buf, sizeof(error_buf));
    if(data_tree == NULL){
        CPE_ERROR(svr->m_em, "%s: damai: parse result fail\n%s", account_svr_name(svr), json_text);
        return -1;
    }

    if ((val = yajl_tree_get_2(data_tree, "status", yajl_t_number))) {
        int svr_error = yajl_get_integer(val);
        switch(svr_error) {
        case 1:
            return 0;
        case 11:
            return SVR_ACCOUNT_ERROR_VENDOR_NOT_LOGIN;
        default:
            if ((val = yajl_tree_get_2(data_tree, "data", yajl_t_string)) == NULL) {
                CPE_ERROR(svr->m_em, "%s: damai: parse result: get error fail\n%s", account_svr_name(svr), json_text);
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: damai: parse result: server return error: %d (%s)", 
                    account_svr_name(svr), svr_error, yajl_get_string(val));
            }
            return -1;
        }
    }
    else {
        CPE_ERROR(svr->m_em, "%s: damai: parse result: get status fail\n%s", account_svr_name(svr), json_text);
        return -1;
    }
}

static void account_svr_backend_damai_token_to_id_on_result(net_trans_task_t task, void * ctx) {
    account_svr_backend_t backend = ctx;
    account_svr_t svr = backend->m_svr;
    logic_require_t require = * (logic_require_t *)net_trans_task_data(task);
    const char * json_text;
    SVR_ACCOUNT_LOGIN_INFO * login_info;
    int rv;
    
    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            svr->m_em, "%s: damai: total_to_id: task execute fail, result=%s, errno=%d!",
            account_svr_name(svr),
            net_trans_task_result_str(net_trans_task_result(task)),
            net_trans_task_errno(task));
        logic_require_set_error(require);
        return;
    }

    json_text = net_trans_task_buffer_to_string(task);
    if (json_text == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: damai: total_to_id: task execute fail, no result json data!",
            account_svr_name(svr));
        logic_require_set_error(require);
        return;
    }

    login_info = logic_data_data(logic_require_data_find(require, dr_meta_name(svr->m_meta_login_info)));
    if ((rv = account_svr_backend_damai_parse_result(svr, login_info, json_text) != 0)) {
        logic_require_set_error_ex(require, rv);
        return;
    }

    logic_require_set_done(require);
}

int account_svr_backend_damai_token_to_id(account_svr_backend_t backend, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_require_t require) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_damai_t damai = account_svr_backend_data(backend);
    struct write_stream_buffer request_stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_dump_buffer);
    const char * request_data;
    size_t request_data_len;
    logic_data_t result_data;
    SVR_ACCOUNT_LOGIN_INFO * login_info;
    net_trans_task_t task;
    char * sep;
    char * name_sep;
    char header_len_buf[64];
    
    result_data = logic_require_data_get_or_create(require, svr->m_meta_login_info, 0);
    if (result_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: damai: total_to_id: create result data fail!", account_svr_name(svr));
        logic_require_set_error(require);
        return -1;
    }
    login_info = logic_data_data(result_data);
    
    task = net_trans_task_create(damai->m_trans_group, sizeof(require));
    if (task == NULL) {
        CPE_ERROR(svr->m_em, "%s: damai: total_to_id: create task fail!", account_svr_name(svr));
        logic_require_set_error(require);
        return -1;
    }
    * (logic_require_t *)net_trans_task_data(task) = require;

    net_trans_task_set_commit_op(task, account_svr_backend_damai_token_to_id_on_result, backend, NULL);

    sep = strchr(logic_id->account, '+');
    if (sep == NULL) {
        CPE_ERROR(svr->m_em, "%s: damai: total_to_id: account %s format error!", account_svr_name(svr), logic_id->account);
        net_trans_task_free(task);
        logic_require_set_error(require);
        return -1;
    }

    *sep = 0;
    
    /*构造请求 */
    mem_buffer_clear_data(&svr->m_dump_buffer);
    stream_printf(
        (write_stream_t)&request_stream,
        "{\"id\": \"%d\", \"appid\": \"%s\", \"username\": \"%s\", \"token\": \"%s\" }",
        net_trans_task_id(task), damai->m_app_id, sep+1, logic_id->account);
    stream_putc((write_stream_t)&request_stream, 0);

    login_info->logic_id.account_type = SVR_ACCOUNT_DAMAI;
    cpe_str_dup(login_info->logic_id.account, sizeof(login_info->logic_id.account), sep + 1);
    cpe_str_dup(login_info->token, sizeof(login_info->token), logic_id->account);
    name_sep = strchr(sep + 1, '_');
    cpe_str_dup(login_info->name, sizeof(login_info->name), name_sep ? name_sep + 1 : sep + 1);

    request_data = (char*)mem_buffer_make_continuous(&svr->m_dump_buffer, 0);
    request_data_len = mem_buffer_size(&svr->m_dump_buffer) - 1;
    
    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: damai: total_to_id: request is %s!",
            account_svr_name(svr), request_data);
    }

    /*发送请求 */
    if (net_trans_task_set_post_to(
            task,
            "http://sdk.93damai.com/cpVerify.php", request_data, request_data_len) != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: damai: total_to_id: set request post %s fail!",
            account_svr_name(svr), request_data);
        net_trans_task_free(task);
        logic_require_set_error(require);
        return -1;
    }

    snprintf(header_len_buf, sizeof(header_len_buf), "Content-Length: %d", (int)request_data_len);
    if (net_trans_task_set_useragent(task, "libcurl-agent/1.0") != 0
        || net_trans_task_append_header(task, "Content-Type: application/json; charset=utf-8") != 0
        || net_trans_task_append_header(task, header_len_buf) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: damai: query result: setup curl fail!", account_svr_name(svr));
        net_trans_task_free(task);
        logic_require_set_error(require);
        return -1;
    }
    
    if (net_trans_task_start(task) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: damai: total_to_id: start request get %s fail!",
            account_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
        net_trans_task_free(task);
        logic_require_set_error(require);
        return -1;
    }

    return 0;
}

