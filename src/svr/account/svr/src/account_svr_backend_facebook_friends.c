#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_group.h"
#include "gd/net_trans/net_trans_task.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "account_svr_backend_facebook.h"
#include "account_svr_login_info.h"

static void account_svr_backend_facebook_query_friends_on_result(net_trans_task_t task, void * ctx);

static int account_svr_backend_facebook_query_friends_start_next_query(
    account_svr_backend_t backend, logic_require_t require, const char * url)
{
    account_svr_t svr = backend->m_svr;
    account_svr_backend_facebook_t facebook = account_svr_backend_data(backend);
    int pre_query_id = 0;;
    char require_name[64];
    logic_require_t next_require;
    logic_stack_node_t state_node;
    
    if (cpe_str_start_with(logic_require_name(require), "query_friends_")) {
        pre_query_id = atoi(logic_require_name(require) + strlen("query_friends_"));
    }
    snprintf(require_name, sizeof(require_name), "query_friends_%d", pre_query_id + 1);

    state_node = logic_require_stack(require);
    next_require = logic_require_create(state_node, require_name);
    if (next_require == NULL) {
        CPE_ERROR(svr->m_em, "%s: facebook: query_friends: create next request fail", account_svr_name(svr));
        return -1;
    }
        
    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: facebook: query_friends: request is %s", account_svr_name(svr), url);
    }

    /*发送请求 */
    return account_svr_backend_start_http_get(
        backend, facebook->m_trans_group, next_require,
        "query_friends(next)", url, account_svr_backend_facebook_query_friends_on_result);
}

static int account_svr_backend_facebook_query_friends_parse_result(account_svr_backend_t backend, logic_require_t require, yajl_val val) {
    uint32_t array_num, i;
    logic_data_t result_data;
    SVR_ACCOUNT_LOGIC_ID_LIST * result;
    
    array_num = (uint32_t)val->u.array.len;

    result_data = logic_require_data_dyn_reserve_for_append(require, backend->m_svr->m_meta_logic_id_list, array_num);
    if (result_data == NULL) {
        CPE_ERROR(
            backend->m_svr->m_em, "%s: facebook: query_friends: create result data fail!", account_svr_name(backend->m_svr));
        logic_require_set_error(require);
        return -1;
    }
    result = logic_data_data(result_data);

    for (i = 0; i < array_num; i++) {
        yajl_val obj = val->u.array.values[i];
        yajl_val v;
        SVR_ACCOUNT_LOGIC_ID * friend = &result->data[result->count];
        
        if (obj == NULL) {
            CPE_ERROR(
                backend->m_svr->m_em, "%s: facebook: query_friends: result array fail",
                account_svr_name(backend->m_svr));
            continue;
        }
        
        if ((v = yajl_tree_get_2(obj, "id", yajl_t_string)) == NULL) {
            CPE_ERROR(
                backend->m_svr->m_em, "%s: facebook: query_friends: result no id",
                account_svr_name(backend->m_svr));
            continue;
        }
        friend->account_type = SVR_ACCOUNT_FACEBOOK;
        
        cpe_str_dup(friend->account, sizeof(friend->account), yajl_get_string(v));
        
        result->count++;
    }
    
    return 0;
}

static void account_svr_backend_facebook_query_friends_on_result(net_trans_task_t task, void * ctx) {
    account_svr_backend_t backend = ctx;
    account_svr_t svr = backend->m_svr;
    logic_require_t require = * (logic_require_t *)net_trans_task_data(task);
    const char * json_text;
    char error_buf[128];
    yajl_val data_tree;
    yajl_val val;
    
    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: query_friends: task execute fail, result=%s, errno=%d!",
            account_svr_name(svr),
            net_trans_task_result_str(net_trans_task_result(task)),
            net_trans_task_errno(task));
        logic_require_set_error(require);
        return;
    }

    json_text = net_trans_task_buffer_to_string(task);
    if (json_text == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: query_friends: task execute fail, no result json data!",
            account_svr_name(svr));
        logic_require_set_error(require);
        return;
    }

    //CPE_ERROR(svr->m_em, "xxxxxx: result=%s\n", json_text);
    
    data_tree  = yajl_tree_parse(json_text, error_buf, sizeof(error_buf));
    if(data_tree == NULL) {
        CPE_ERROR(svr->m_em, "%s: facebook: parse result fail\n%s", account_svr_name(svr), json_text);
        logic_require_set_error(require);
        return;
    }

    if (account_svr_backend_facebook_check_error(backend, data_tree, "query_friends") != 0) {
        logic_require_set_error(require);
        yajl_tree_free(data_tree);
        return;
    }

    val = yajl_tree_get_2(data_tree, "data", yajl_t_array);
    if (val == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: facebook: query_friends: get data in result fail!\n%s",
            account_svr_name(svr), json_text);
        logic_require_set_error(require);
        yajl_tree_free(data_tree);
        return;
    }
    if (account_svr_backend_facebook_query_friends_parse_result(backend, require, val) != 0) {
        logic_require_set_error(require);
        yajl_tree_free(data_tree);
        return;
    }
    
    /*如果有分页，启动新的请求 */
    val = yajl_tree_get_2(data_tree, "paging", yajl_t_object);
    if (val) {
        val = yajl_tree_get_2(val, "next", yajl_t_string);
        if (val) {
            if (account_svr_backend_facebook_query_friends_start_next_query(backend, require, yajl_get_string(val)) != 0) {
                logic_require_set_error(require);
                yajl_tree_free(data_tree);
                return;
            }
        }
    }
    
    logic_require_set_done(require);
    yajl_tree_free(data_tree);
}

int account_svr_backend_facebook_query_friends(account_svr_backend_t backend, account_svr_login_info_t login_info, logic_require_t require) {
    account_svr_t svr = backend->m_svr;
    account_svr_backend_facebook_t facebook = account_svr_backend_data(backend);
    struct write_stream_buffer request_stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_dump_buffer);
    
    /*构造请求 */
    mem_buffer_clear_data(&svr->m_dump_buffer);
    stream_printf(
        (write_stream_t)&request_stream, "%s/me/friends?access_token=%s",
        facebook->m_url, login_info->m_data.token);
    stream_putc((write_stream_t)&request_stream, 0);

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: facebook: query_friends: request is %s",
            account_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
    }

    /*发送请求 */
    return account_svr_backend_start_http_get(
        backend, facebook->m_trans_group, require,
        "query_friends",
        mem_buffer_make_continuous(&svr->m_dump_buffer, 0),
        account_svr_backend_facebook_query_friends_on_result);
}
