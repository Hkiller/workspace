#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_group.h"
#include "gd/net_trans/net_trans_task.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "account_svr_backend_facebook.h"
#include "yajl/yajl_tree.h"

static int account_svr_backend_facebook_token_to_id_parse_result(
    account_svr_backend_t backend, SVR_ACCOUNT_LOGIN_INFO * login_info, const char * json_text) {
    char error_buf[128];
    yajl_val data_tree;
    yajl_val val;

    data_tree  = yajl_tree_parse(json_text, error_buf, sizeof(error_buf));
    if(data_tree == NULL){
        CPE_ERROR(backend->m_svr->m_em, "%s: facebook: parse result fail\n%s", account_svr_name(backend->m_svr), json_text);
        return -1;
    }

    if (account_svr_backend_facebook_check_error(backend, data_tree, "token_to_id") != 0) {
        return -1;
    }

    if ((val = yajl_tree_get_2(data_tree, "id", yajl_t_string)) == NULL) {
        CPE_ERROR(backend->m_svr->m_em, "%s: facebook: parse result: get id fail\n%s", account_svr_name(backend->m_svr), json_text);
        return -1;
    }
    cpe_str_dup(login_info->logic_id.account, sizeof(login_info->logic_id.account), yajl_get_string(val));

    if ((val = yajl_tree_get_2(data_tree, "name", yajl_t_string)) == NULL) {
        CPE_ERROR(backend->m_svr->m_em, "%s: facebook: parse result: get name fail\n%s", account_svr_name(backend->m_svr), json_text);
        return -1;
    }
    cpe_str_dup(login_info->name, sizeof(login_info->name), yajl_get_string(val));

    if ((val = yajl_tree_get_2(data_tree, "picture/data/url", yajl_t_string))) {
        cpe_str_dup(login_info->avatar, sizeof(login_info->avatar), yajl_get_string(val));
    }
    else{
        CPE_ERROR(backend->m_svr->m_em, "facebook: yajl get avatar = %s!", yajl_get_string(val));
        CPE_ERROR(backend->m_svr->m_em, "facebook: avatar = %s!", login_info->avatar);
    }
    /* CPE_ERROR(backend->m_svr->m_em, "facebook: yajl get name = %s!", login_info->name); */
    /* CPE_ERROR(backend->m_svr->m_em, "facebook: yajl get id = %s!", login_info->logic_id.account); */
    /* CPE_ERROR(backend->m_svr->m_em, "facebook: yajl get avatar = %s!", login_info->avatar); */

    return 0;
}

static void account_svr_backend_facebook_token_to_id_on_result(net_trans_task_t task, void * ctx) {
    account_svr_backend_t backend = ctx;
    account_svr_t svr = backend->m_svr;
    logic_require_t require = * (logic_require_t *)net_trans_task_data(task);
    const char * json_text;
    logic_data_t result_data;
    SVR_ACCOUNT_LOGIN_INFO * login_info;

    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: total_to_id: task execute fail, result=%s, errno=%d!",
            account_svr_name(svr),
            net_trans_task_result_str(net_trans_task_result(task)),
            net_trans_task_errno(task));
        logic_require_set_error(require);
        return;
    }

    json_text = net_trans_task_buffer_to_string(task);
    if (json_text == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: total_to_id: task execute fail, no result json data!",
            account_svr_name(svr));
        logic_require_set_error(require);
        return;
    }

    CPE_ERROR(svr->m_em, "%s: xxxxxxx: %s", account_svr_name(svr), json_text);
    
    result_data = logic_require_data_find(require, dr_meta_name(svr->m_meta_login_info));
    assert(result_data);
    login_info = logic_data_data(result_data);

    if (account_svr_backend_facebook_token_to_id_parse_result(backend, login_info, json_text) != 0) {
        logic_require_set_error(require);
        return;
    }
    login_info->logic_id.account_type = SVR_ACCOUNT_FACEBOOK;

    logic_require_set_done(require);
}

static int account_svr_backend_facebook_token_to_id_send(account_svr_backend_t backend, const char * token, logic_require_t require) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_facebook_t facebook = account_svr_backend_data(backend);
    struct write_stream_buffer request_stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_dump_buffer);
    logic_data_t result_data;
    SVR_ACCOUNT_LOGIN_INFO * login_info;

    result_data = logic_require_data_get_or_create(require, svr->m_meta_login_info, 0);
    if (result_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: total_to_id: create result data fail!", account_svr_name(svr));
        logic_require_set_error(require);
        return -1;
    }

    login_info = logic_data_data(result_data);
    cpe_str_dup(login_info->token, sizeof(login_info->token), token);

    /*构造请求 */
    mem_buffer_clear_data(&svr->m_dump_buffer);
    stream_printf((write_stream_t)&request_stream, "%s/me/?access_token=%s&fields=id,name,picture.limit(1)", facebook->m_url, token);
    stream_putc((write_stream_t)&request_stream, 0);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: facebook: total_to_id: request is %s!",
            account_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
    }

    return account_svr_backend_start_http_get(
        backend, facebook->m_trans_group, require,
        "total_to_id",
        mem_buffer_make_continuous(&svr->m_dump_buffer, 0),
        account_svr_backend_facebook_token_to_id_on_result);
}

static void account_svr_backend_facebook_exchange_token_on_result(net_trans_task_t task, void * ctx) {
    account_svr_backend_t backend = ctx;
    account_svr_t svr = backend->m_svr;
    logic_require_t require = * (logic_require_t *)net_trans_task_data(task);
    char * result_text;
    char * access_token;
    
    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: exchange_token: task execute fail, result=%s, errno=%d!",
            account_svr_name(svr),
            net_trans_task_result_str(net_trans_task_result(task)),
            net_trans_task_errno(task));
        logic_require_set_error(require);
        return;
    }

    result_text = net_trans_task_buffer_to_string(task);
    if (result_text == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: exchange_token: task execute fail, no result json data!",
            account_svr_name(svr));
        logic_require_set_error(require);
        return;
    }

    access_token = cpe_str_read_and_remove_arg(result_text, "access_token", '&', '=');
    if (access_token == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: exchange_token: return error: %s", 
            account_svr_name(svr), result_text);
        logic_require_set_error(require);
        return;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: facebook: exchange_token: token is %s", 
            account_svr_name(svr), access_token);
    }
    
    if (account_svr_backend_facebook_token_to_id_send(backend, access_token, require) != 0) {
        return;
    }
}

static int account_svr_backend_facebook_exchange_token_send(
    account_svr_backend_t backend, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_require_t require)
{
    account_svr_t svr = backend->m_svr;
    account_svr_backend_facebook_t facebook = account_svr_backend_data(backend);
    struct write_stream_buffer request_stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_dump_buffer);

    /*构造请求 */
    mem_buffer_clear_data(&svr->m_dump_buffer);
    stream_printf(
        (write_stream_t)&request_stream, "%s/oauth/access_token?client_id=%s&client_secret=%s&grant_type=fb_exchange_token&fb_exchange_token=%s",
        facebook->m_url, facebook->m_app_id, facebook->m_app_secret, logic_id->account);
    stream_putc((write_stream_t)&request_stream, 0);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: facebook: exchange_token: request is %s",
            account_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
    }

    return account_svr_backend_start_http_get(
        backend, facebook->m_trans_group, require,
        "exchange_token",
        mem_buffer_make_continuous(&svr->m_dump_buffer, 0),
        account_svr_backend_facebook_exchange_token_on_result);
}

int account_svr_backend_facebook_token_to_id(account_svr_backend_t backend, SVR_ACCOUNT_LOGIC_ID const * logic_id, logic_require_t require) {
    account_svr_backend_facebook_t facebook = account_svr_backend_data(backend);
    
    if (facebook->m_exchange_token) {
        return account_svr_backend_facebook_exchange_token_send(backend, logic_id, require);
    }
    else {
        return account_svr_backend_facebook_token_to_id_send(backend, logic_id->account, require);
    }
}
