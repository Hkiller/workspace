#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_group.h"
#include "gd/net_trans/net_trans_task.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "account_svr_backend_weixin.h"
#include "yajl/yajl_tree.h"

int account_svr_backend_weixin_init(account_svr_backend_t backend, cfg_t cfg) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_weixin_t weixin = account_svr_backend_data(backend);

    weixin->m_trans_group = net_trans_group_create(svr->m_trans_mgr, "weixin");
    if (weixin->m_trans_group == NULL) {
        CPE_ERROR(svr->m_em, "account_svr_backend_weixin_init: crate trans group fail!");
        return -1;
    }

    return 0;
}

void account_svr_backend_weixin_fini(account_svr_backend_t backend) {
    account_svr_backend_weixin_t weixin = account_svr_backend_data(backend);

    net_trans_group_free(weixin->m_trans_group);
    weixin->m_trans_group = NULL;
}

static int account_svr_backend_weixin_parse_result(account_svr_t svr, SVR_ACCOUNT_LOGIN_INFO * login_info, const char * json_text) {
    char error_buf[128];
    yajl_val data_tree;
    yajl_val val;

    data_tree  = yajl_tree_parse(json_text, error_buf, sizeof(error_buf));
    if(data_tree == NULL){
        CPE_ERROR(svr->m_em, "%s: weixin: parse result fail\n%s", account_svr_name(svr), json_text);
        return -1;
    }

    if ((val = yajl_tree_get_2(data_tree, "errcode", yajl_t_number))) {
        int svr_error = yajl_get_integer(val);
        if (svr_error != 0) {
            if ((val = yajl_tree_get_2(data_tree, "errmsg", yajl_t_string)) == NULL) {
                CPE_ERROR(svr->m_em, "%s: weixin: parse result: get error fail\n%s", account_svr_name(svr), json_text);
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: weixin: parse result: server return error: %d (%s)", 
                    account_svr_name(svr), svr_error, yajl_get_string(val));
            }
            return svr_error;
        }
    }

    if ((val = yajl_tree_get_2(data_tree, "unionid", yajl_t_string)) == NULL) {
        CPE_ERROR(svr->m_em, "%s: weixin: parse result: get id fail\n%s", account_svr_name(svr), json_text);
        return -1;
    }
    cpe_str_dup(login_info->logic_id.account, sizeof(login_info->logic_id.account), yajl_get_string(val));

    if ((val = yajl_tree_get_2(data_tree, "nickname", yajl_t_string)) == NULL) {
        CPE_ERROR(svr->m_em, "%s: weixin: parse result: get name fail\n%s", account_svr_name(svr), json_text);
        return -1;
    }
    cpe_str_dup(login_info->name, sizeof(login_info->name), yajl_get_string(val));

    if ((val = yajl_tree_get_2(data_tree, "headimgurl", yajl_t_string)) == NULL) {
        CPE_ERROR(svr->m_em, "%s: weixin: parse result: get avatar fail\n%s", account_svr_name(svr), json_text);
        return -1;
    }
    cpe_str_dup(login_info->avatar, sizeof(login_info->avatar), yajl_get_string(val));

    CPE_ERROR(svr->m_em, "weixin: yajl get name = %s!", login_info->name);
    CPE_ERROR(svr->m_em, "weixin: yajl get id = %s!", login_info->logic_id.account);
    CPE_ERROR(svr->m_em, "weixin: yajl get avatar = %s!", login_info->avatar);

    return 0;
}

static void account_svr_backend_weixin_token_to_id_on_result(net_trans_task_t task, void * ctx) {
    account_svr_backend_t backend = ctx;
    account_svr_t svr = backend->m_svr;
    logic_require_t require = * (logic_require_t *)net_trans_task_data(task);
    const char * json_text;
    logic_data_t result_data;
    SVR_ACCOUNT_LOGIN_INFO * login_info;

    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            svr->m_em, "%s: weixin: total_to_id: task execute fail, result=%s, errno=%d!",
            account_svr_name(svr),
            net_trans_task_result_str(net_trans_task_result(task)),
            net_trans_task_errno(task));
        logic_require_set_error(require);
        return;
    }

    json_text = net_trans_task_buffer_to_string(task);
    if (json_text == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: weixin: total_to_id: task execute fail, no result json data!",
            account_svr_name(svr));
        logic_require_set_error(require);
        return;
    }

    result_data = logic_require_data_get_or_create(require, svr->m_meta_login_info, 0);
    if (result_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: weixin: total_to_id: create result data fail!", account_svr_name(svr));
        logic_require_set_error(require);
        return;
    }

    login_info = logic_data_data(result_data);

    if (account_svr_backend_weixin_parse_result(svr, login_info, json_text) != 0) {
        logic_require_set_error(require);
        return;
    }
    login_info->logic_id.account_type =SVR_ACCOUNT_WEIXIN;

    logic_require_set_done(require);
}

int account_svr_backend_weixin_token_to_id(account_svr_backend_t backend, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_require_t require) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_weixin_t weixin = account_svr_backend_data(backend);
    struct write_stream_buffer request_stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_dump_buffer);
    net_trans_task_t task;
    char * sep;

    task = net_trans_task_create(weixin->m_trans_group, sizeof(require));
    if (task == NULL) {
        CPE_ERROR(svr->m_em, "%s: weixin: total_to_id: create task fail!", account_svr_name(svr));
        logic_require_set_error(require);
        return -1;
    }

    * (logic_require_t *)net_trans_task_data(task) = require;

    net_trans_task_set_commit_op(task, account_svr_backend_weixin_token_to_id_on_result, backend, NULL);

    sep = strchr(logic_id->account, '+');
    if (sep == NULL) {
        CPE_ERROR(svr->m_em, "%s: weixin: total_to_id: account %s format error!", account_svr_name(svr), logic_id->account);
        logic_require_set_error(require);
        net_trans_task_free(task);
        return -1;
    }

    *sep = 0;
    
    /*构造请求 */
    mem_buffer_clear_data(&svr->m_dump_buffer);
    stream_printf(
        (write_stream_t)&request_stream,
        "https://api.weixin.qq.com/sns/userinfo?access_token=%s&openid=%s",
        sep+1, logic_id->account);
    stream_putc((write_stream_t)&request_stream, 0);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: weixin: total_to_id: request is %s!",
            account_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
    }

    /*发送请求 */
    if (net_trans_task_set_get(task, mem_buffer_make_continuous(&svr->m_dump_buffer, 0)) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: weixin: total_to_id: set request get %s fail!",
            account_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
        net_trans_task_free(task);
        logic_require_set_error(require);
        return -1;
    }

    if (net_trans_task_start(task) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: weixin: total_to_id: start request get %s fail!",
            account_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
        net_trans_task_free(task);
        logic_require_set_error(require);
        return -1;
    }

    return 0;
}

