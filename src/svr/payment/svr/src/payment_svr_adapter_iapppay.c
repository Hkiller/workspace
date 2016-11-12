#include <assert.h>
#include "openssl/x509.h"
#include "openssl/evp.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/base64.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils_openssl/openssl_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_json_tree.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_group.h"
#include "gd/net_trans/net_trans_task.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_sp.h"
#include "payment_svr_adapter_iapppay.h"
#include "payment_svr_adapter.h"
#include "payment_svr_adapter_type.h"

static int payment_svr_iapppay_data_init(payment_svr_t svr, struct payment_svr_adapter_iapppay_data * data, cfg_t cfg);
static void payment_svr_iapppay_data_fini(payment_svr_t svr, struct payment_svr_adapter_iapppay_data * data);

int payment_svr_iapppay_init(payment_svr_adapter_t adapter, cfg_t cfg) {
    payment_svr_t svr = adapter->m_svr;
    struct payment_svr_adapter_iapppay * iapppay = (struct payment_svr_adapter_iapppay *)adapter->m_private;
    cfg_t platform_cfg;
    
    assert(sizeof(*iapppay) <= sizeof(adapter->m_private));

    iapppay->m_trans_group = net_trans_group_create(svr->m_trans_mgr, "iapppay");
    if (iapppay->m_trans_group == NULL) {
        CPE_ERROR(svr->m_em, "%s: iapppay: crate trans group fail!", payment_svr_name(svr));
        return -1;
    }
    
    if ((platform_cfg = cfg_find_cfg(cfg, "ios"))) {
        if (payment_svr_iapppay_data_init(svr, &iapppay->m_ios, platform_cfg) != 0) {
            net_trans_group_free(iapppay->m_trans_group);
            return -1;
        }
    }

    if ((platform_cfg = cfg_find_cfg(cfg, "android"))) {
        if (payment_svr_iapppay_data_init(svr, &iapppay->m_android, platform_cfg) != 0) {
            payment_svr_iapppay_data_fini(svr, &iapppay->m_ios);
            net_trans_group_free(iapppay->m_trans_group);
            return -1;
        }
    }
    
    return 0;
}

void payment_svr_iapppay_fini(payment_svr_adapter_t adapter) {
    payment_svr_t svr = adapter->m_svr;
    struct payment_svr_adapter_iapppay * iapppay = (struct payment_svr_adapter_iapppay *)adapter->m_private;

    payment_svr_iapppay_data_fini(svr, &iapppay->m_ios);
    payment_svr_iapppay_data_fini(svr, &iapppay->m_android);

    net_trans_group_free(iapppay->m_trans_group);
    iapppay->m_trans_group = NULL;
}

static EVP_PKEY * payment_svr_iapppay_make_key(payment_svr_t svr, const char * i_key, uint8_t is_public_key) {
    size_t i_key_len = strlen(i_key);
    uint8_t * key_buf;
    int skip = 0;
    int key_len;
    uint8_t * p;
    RSA * rsa;
    EVP_PKEY * key;

    key_buf = OPENSSL_malloc(i_key_len * 2);
    if (key_buf == NULL) return NULL;
    
    if (i_key[i_key_len - 1] == '=') skip++;
    if (i_key[i_key_len - 2] == '=') skip++;
    
    key_len = EVP_DecodeBlock(key_buf, (const uint8_t *)i_key, i_key_len);
    if (key_len < 0) {
        CPE_ERROR(svr->m_em, "payment_svr: iapppay: decode key block fail");
        OPENSSL_free(key_buf);
        return NULL;
    }
    assert(key_len >= skip);
    key_len -= skip;
    key_buf[key_len] = 0;

    //d2i_RSA_PUBKEY
    p = key_buf;
    rsa = is_public_key
        ? d2i_RSA_PUBKEY(NULL, (const uint8_t **) &p, key_len)
        : d2i_RSAPrivateKey(NULL, (const uint8_t **) &p, key_len);
    if (rsa == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr: iapppay: build key fail");
        OPENSSL_free(key_buf);
        return NULL;
    }
    
    key = EVP_PKEY_new();
    if (!EVP_PKEY_assign_RSA(key, rsa)) {
        RSA_free(rsa);
        EVP_PKEY_free(key);
        return NULL;
    }
    
    OPENSSL_free(key_buf);
    return key;
}

static int payment_svr_iapppay_data_init(payment_svr_t svr, struct payment_svr_adapter_iapppay_data * data, cfg_t cfg) {
    const char * app_id = cfg_get_string(cfg, "app-id", NULL);
    const char * appv_key = cfg_get_string(cfg, "appv-key", NULL);
    const char * platp_key = cfg_get_string(cfg, "platp-key", NULL);

    if (app_id == NULL || appv_key == NULL || platp_key == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr: iapppay: config error, app-id=%s, appv_key=%s, platp-key=%s", app_id, appv_key, platp_key);
        return -1;
    }

    data->m_appid = cpe_str_mem_dup(svr->m_alloc, app_id);
    if (data->m_appid == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr: iapppay: alloc for appid fail!");
        return -1;
    }

    data->m_appv_key = payment_svr_iapppay_make_key(svr, appv_key, 0);
    if (data->m_appv_key == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr: iapppay: build appv-key fail!\n%s", appv_key);
        mem_free(svr->m_alloc, data->m_appid);
        data->m_appid = NULL;
        return -1;
    }
    
    data->m_platp_key = payment_svr_iapppay_make_key(svr, platp_key, 1);
    if (data->m_platp_key == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr: iapppay: build platp-key fail!\n%s", platp_key);

        EVP_PKEY_free(data->m_appv_key);
        data->m_appv_key = NULL;
        
        mem_free(svr->m_alloc, data->m_appid);
        data->m_appid = NULL;

        return -1;
    }
    
    return 0;
}

static void payment_svr_iapppay_data_fini(payment_svr_t svr, struct payment_svr_adapter_iapppay_data * data) {
    if (data->m_appid) {
        mem_free(svr->m_alloc, data->m_appid);
        data->m_appid = NULL;
    }

    if (data->m_appv_key) {
        EVP_PKEY_free(data->m_appv_key);
        data->m_appv_key = NULL;
    }

    if (data->m_platp_key) {
        EVP_PKEY_free(data->m_platp_key);
        data->m_platp_key = NULL;
    }
}

static void payment_svr_iapppay_on_query_result(net_trans_task_t task, void * ctx) {
    payment_svr_adapter_t adapter = ctx;
    payment_svr_t svr = adapter->m_svr;
    const char * result_text;
    logic_require_t require = * (logic_require_t *)net_trans_task_data(task);
    char * p;
    char * transdata;
    yajl_val val;
    char error_buf[128];
    yajl_val data_tree = NULL;
    LPDRMETA result_meta;
    logic_data_t result_data;

    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            svr->m_em, "%s: iapppay: task execute fail, result=%s, errno=%d!",
            payment_svr_name(svr),
            net_trans_task_result_str(net_trans_task_result(task)),
            net_trans_task_errno(task));
        goto PROCESS_ERROR;
    }

    result_text = net_trans_task_buffer_to_string(task);
    if (result_text == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: qihoo: total_to_id: task execute fail, no result json data!",
            payment_svr_name(svr));
        goto PROCESS_ERROR;
    }

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: iapppay: query result: (%d) %s", payment_svr_name(svr), (int)strlen(result_text), result_text);
    }

    transdata = (char*)strstr(result_text, "transdata=");
    if (transdata == NULL) {
        CPE_ERROR(svr->m_em, "%s: iapppay: get transdata from result fail\ninput: %s", payment_svr_name(svr), result_text);
        goto PROCESS_ERROR;
    }

    transdata += strlen("transdata=");
    p = strchr(transdata, '&');
    if (p) *p = 0;
    
    data_tree  = yajl_tree_parse(transdata, error_buf, sizeof(error_buf));
    if(data_tree == NULL){
        CPE_ERROR(svr->m_em, "%s: iapppay: parse result fail, error=%s\ninput: %s", payment_svr_name(svr), error_buf, transdata);
        goto PROCESS_ERROR;
    }

    if ((val = yajl_tree_get_2(data_tree, "code", yajl_t_number))) { /*处理错误 */
        result_meta = svr->m_meta_iapppay_error;
    }
    else {
        result_meta = svr->m_meta_iapppay_record;
    }

    result_data = logic_require_data_get_or_create(require, result_meta, 0);
    if (result_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: iapppay: create result buff fail", payment_svr_name(svr));
        goto PROCESS_ERROR;
    }
    
    if (dr_json_tree_read(
            logic_data_data(result_data),
            logic_data_capacity(result_data),
            data_tree, logic_data_meta(result_data), svr->m_em)
        < 0)
    {
        CPE_ERROR(svr->m_em, "%s: iapppay: read data to %s fail!\n%s", payment_svr_name(svr), dr_meta_name(result_meta), result_text);
        goto PROCESS_ERROR;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: iapppay: dump parse result: %s!",
            payment_svr_name(svr),
            dr_json_dump_inline(
                gd_app_tmp_buffer(svr->m_app),
                logic_data_data(result_data), logic_data_capacity(result_data), logic_data_meta(result_data)));
    }
    
    yajl_tree_free(data_tree);    
    logic_require_set_done(require);

    return;
PROCESS_ERROR:
    logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
    if (data_tree) yajl_tree_free(data_tree);    
}

static
struct payment_svr_adapter_iapppay_data *
payment_svr_adapter_iapppay_env(payment_svr_t svr, payment_svr_adapter_t adapter, PAYMENT_RECHARGE_RECORD * record) {
    struct payment_svr_adapter_iapppay * iapppay = (struct payment_svr_adapter_iapppay *)adapter->m_private;
    
    switch(record->device_category) {
    case svr_payment_device_windows:
        CPE_ERROR(svr->m_em, "%s: iapppay: not support device windows", payment_svr_name(svr));
        return NULL;
    case svr_payment_device_ios:
        return &iapppay->m_ios;
    case svr_payment_device_android:
        return &iapppay->m_android;
    default:
        CPE_ERROR(
            svr->m_em, "%s: iapppay: unknown device category %d",
            payment_svr_name(svr), record->device_category);
        return NULL;
    }
}

int payment_svr_iapppay_charge_send(
    logic_context_t ctx, logic_stack_node_t stack, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    payment_svr_t svr = adapter->m_svr;
    struct payment_svr_adapter_iapppay * iapppay = (struct payment_svr_adapter_iapppay *)adapter->m_private;
    struct payment_svr_adapter_iapppay_data * env;
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_buffer);
    net_trans_task_t task;
    logic_require_t require;
    char transdata[128];

    env = payment_svr_adapter_iapppay_env(svr, adapter, record);
    if (env == NULL) {
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    if (env->m_platp_key == NULL) {
        CPE_ERROR(svr->m_em, "%s: iapppay: query result: env %d no platp key!", payment_svr_name(svr), record->device_category);
        return -1;
    }
    
    snprintf(transdata, sizeof(transdata), "{\"appid\":\"%s\",\"cporderid\":\"%s\"}", env->m_appid, record->_id);

    if (cpe_openssl_sign_with_rsa(
            &svr->m_rsa_buffer, &svr->m_sign_buffer,
            transdata, strlen(transdata), EVP_md5(), env->m_appv_key, svr->m_em) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: iapppay: query result: sign with rsa fail!", payment_svr_name(svr));
        return -1;
    }
    
    mem_buffer_clear_data(&svr->m_buffer);
    stream_printf((write_stream_t)&s, "transdata=%s&sign=%s&signtype=RSA", transdata, (const char *)mem_buffer_make_continuous(&svr->m_rsa_buffer, 0));
    
    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: iapppay: query result: request is %s!",
            payment_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_buffer, 0));
    }

    require = logic_require_create(stack, "iapppay_query_ersult");
    if (require == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: iapppay: create require fail %d",
            payment_svr_name(svr), record->device_category);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }
    
    task = net_trans_task_create(iapppay->m_trans_group, sizeof(require));
    if (task == NULL) {
        CPE_ERROR(svr->m_em, "%s: qihoo: query result: create task fail!", payment_svr_name(svr));
        logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    * (logic_require_t *)net_trans_task_data(task) = require;
    net_trans_task_set_commit_op(task, payment_svr_iapppay_on_query_result, adapter, NULL);

    if (net_trans_task_set_useragent(task, "libcurl-agent/1.0") != 0
        || net_trans_task_append_header(task, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8") != 0)
    {
        CPE_ERROR(svr->m_em, "%s: qihoo: query result: setup curl fail!", payment_svr_name(svr));
        logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    /*发送请求 */
    if (net_trans_task_set_post_to(
            task,
            "http://ipay.iapppay.com:9999/payapi/queryresult",
            mem_buffer_make_continuous(&svr->m_buffer, 0),
            mem_buffer_size(&svr->m_buffer)) != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: iapppay: query result: set request get %s fail!",
            payment_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_buffer, 0));
        logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    if (net_trans_task_start(task) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: iapppay: query result: start request get %s fail!",
            payment_svr_name(svr), (char*)mem_buffer_make_continuous(&svr->m_buffer, 0));
        logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    return 0;
}

int payment_svr_iapppay_charge_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, payment_svr_adapter_t adapter,
    PAYMENT_RECHARGE_RECORD * record, SVR_PAYMENT_REQ_RECHARGE_COMMIT const * req)
{
    payment_svr_t svr = adapter->m_svr;
    logic_data_t result_data;

    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            logic_context_errno_set(ctx, logic_require_error(require));
        }
        else {
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        }
        return -1;
    }

    if ((result_data = logic_require_data_find(require, dr_meta_name(svr->m_meta_iapppay_error)))) {
        SVR_PAYMENT_IAPPPAY_ERROR * iapppay_error = logic_data_data(result_data);

        switch(iapppay_error->code) {
        case 2001: /* 登录令牌已过期 */
        case 2002: /* 登录令牌认证失败 */
            logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_LOGIN_AGAIN);
            return -1;
        case 3001: /* 商户订单号已存在 */
        case 3002: /* 商品数据异常 */
            record->state = PAYMENT_RECHARGE_FAIL;
            break;
        case 3003: /* 支付金额为0，不需要支付 */
            record->state = PAYMENT_RECHARGE_SUCCESS;
            break;
        case 4001: /* 订单正在处理 */
        case 3004: /* 用户存在有效契约，不需要支付 */
        case 3005: /* 商户订单数据异常 */
        case 5001: /* 用户不存在有效契约 */
        case 5002: /* 用户契约已过期 */
        case 5003: /* 用户契约已过完 */
        case 9999: /* 系统繁忙，请稍后再试 */
        case 1001: /*应用发起的请求签名验证失败*/
        default:
            break;
        }

        record->error = iapppay_error->code;
        cpe_str_dup(record->error_msg, sizeof(record->error_msg), iapppay_error->errmsg);
    }
    else if ((result_data = logic_require_data_find(require, dr_meta_name(svr->m_meta_iapppay_record)))) {
        SVR_PAYMENT_IAPPPAY_RECORD * iapppay_query_result = logic_data_data(result_data);
        struct payment_svr_adapter_iapppay_data * env;

        env = payment_svr_adapter_iapppay_env(svr, adapter, record);
        assert(env);

        if (strcmp(iapppay_query_result->appid, env->m_appid) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: iapppay: recv: return appid %s mismatch with %s!",
                payment_svr_name(svr), iapppay_query_result->appid, env->m_appid);
            logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
            return -1;
        }

        if (iapppay_query_result->money != record->cost) {
            CPE_ERROR(
                svr->m_em, "%s: iapppay: recv: money %f mismatch with record require money %f!",
                payment_svr_name(svr), iapppay_query_result->money, record->cost);
            logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
            return -1;
        }

        if (iapppay_query_result->result == 2) { /*待支付 */
            logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_PAY_AGAIN);
            return -1;
        }
        else if (iapppay_query_result->result == 0) {
            record->state = PAYMENT_RECHARGE_SUCCESS;
        }
        
        record->vendor_record.iapppay = *iapppay_query_result;
    }
    
    return 0;
}

