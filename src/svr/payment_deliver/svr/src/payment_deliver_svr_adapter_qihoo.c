#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/md5.h"
#include "cpe/utils/http_args.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_http_args.h"
#include "gd/app/app_context.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "payment_deliver_svr_adapter_qihoo.h"
#include "payment_deliver_svr_adapter.h"
#include "payment_deliver_svr_adapter_type.h"
#include "payment_deliver_svr_request.h"
#include "payment_deliver_svr_connection.h"
#include "payment_deliver_svr_utils.h"

static int payment_deliver_svr_qihoo_data_init(payment_deliver_svr_t svr, struct payment_deliver_adapter_qihoo_data * data, cfg_t cfg);
static void payment_deliver_svr_qihoo_data_fini(payment_deliver_svr_t svr, struct payment_deliver_adapter_qihoo_data * data);
static const char * payment_deliver_svr_qihoo_sign(payment_deliver_svr_t svr, struct payment_deliver_adapter_qihoo_data * env, cpe_http_arg_t args, uint16_t arg_count);

int payment_deliver_svr_qihoo_init(payment_deliver_adapter_t adapter, cfg_t cfg) {
    payment_deliver_svr_t svr = adapter->m_svr;
    struct payment_deliver_adapter_qihoo * qihoo = (struct payment_deliver_adapter_qihoo *)adapter->m_private;
    cfg_t platform_cfg;
    
    assert(sizeof(*qihoo) <= sizeof(adapter->m_private));

    if ((platform_cfg = cfg_find_cfg(cfg, "android"))) {
        if (payment_deliver_svr_qihoo_data_init(svr, &qihoo->m_android, platform_cfg) != 0) {
            return -1;
        }
    }
    
    return 0;
}

void payment_deliver_svr_qihoo_fini(payment_deliver_adapter_t adapter) {
    payment_deliver_svr_t svr = adapter->m_svr;
    struct payment_deliver_adapter_qihoo * qihoo = (struct payment_deliver_adapter_qihoo *)adapter->m_private;

    payment_deliver_svr_qihoo_data_fini(svr, &qihoo->m_android);
}

static int payment_deliver_svr_qihoo_data_init(payment_deliver_svr_t svr, struct payment_deliver_adapter_qihoo_data * data, cfg_t cfg) {
    const char * appid = cfg_get_string(cfg, "appid", NULL);
    const char * appkey = cfg_get_string(cfg, "appkey", NULL);
    const char * appsecret = cfg_get_string(cfg, "appsecret", NULL);

    if (appid == NULL || appkey == NULL || appkey == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver_svr: qihoo: config error, appid=%s, appkey=%s, appsecret=%s", appid, appkey, appsecret);
        return -1;
    }

    data->m_appid = cpe_str_mem_dup(svr->m_alloc, appid);
    data->m_appkey = cpe_str_mem_dup(svr->m_alloc, appkey);
    data->m_appsecret = cpe_str_mem_dup(svr->m_alloc, appsecret);
    if (data->m_appid == NULL || data->m_appkey == NULL || data->m_appsecret == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver_svr: qihoo: load config alloc fail!");

        if (data->m_appid) { mem_free(svr->m_alloc, data->m_appid); data->m_appid = NULL; }
        if (data->m_appkey) { mem_free(svr->m_alloc, data->m_appkey); data->m_appkey = NULL; }
        if (data->m_appsecret) { mem_free(svr->m_alloc, data->m_appsecret); data->m_appsecret = NULL; }

        return -1;
    }

    return 0;
}

static void payment_deliver_svr_qihoo_data_fini(payment_deliver_svr_t svr, struct payment_deliver_adapter_qihoo_data * data) {
    if (data->m_appid) { mem_free(svr->m_alloc, data->m_appid); data->m_appid = NULL; }
    if (data->m_appkey) { mem_free(svr->m_alloc, data->m_appkey); data->m_appkey = NULL; }
    if (data->m_appsecret) { mem_free(svr->m_alloc, data->m_appsecret); data->m_appsecret = NULL; }
}

static struct payment_deliver_adapter_qihoo_data *
payment_deliver_svr_adapter_qihoo_env(payment_deliver_svr_t svr, payment_deliver_adapter_t adapter, uint8_t device) {
    struct payment_deliver_adapter_qihoo * qihoo = (struct payment_deliver_adapter_qihoo *)adapter->m_private;
    
    switch(device) {
    case svr_payment_device_windows:
        CPE_ERROR(svr->m_em, "%s: qihoo: not support device windows", payment_deliver_svr_name(svr));
        return NULL;
    case svr_payment_device_ios:
        CPE_ERROR(svr->m_em, "%s: qihoo: not support device ios", payment_deliver_svr_name(svr));
        return NULL;
    case svr_payment_device_android:
        return &qihoo->m_android;
    default:
        CPE_ERROR(svr->m_em, "%s: qihoo: unknown device category %d", payment_deliver_svr_name(svr), device);
        return NULL;
    }
}

int payment_deliver_svr_qihoo_on_request(payment_deliver_svr_t svr, payment_deliver_request_t request) {
    struct payment_deliver_adapter_qihoo_data * env;
    void * data;
    struct cpe_http_arg args[64];
    uint16_t arg_count = CPE_ARRAY_SIZE(args);
    const char * require_args[] = {"app_key", "product_id", "app_uid", "order_id", "sign_type", "gateway_flag", "sign", "sign_return","amount" };
    char * app_order_id;
    char * sign_return;
    char * sign;
    char * app_key;
    uint8_t device_category;
    uint16_t set_id;
    uint16_t service;
    const char * build_sign;
    dp_req_t notify_pkg;
    SVR_PAYMENT_REQ_NOTIFY * notify;

    data = payment_deliver_request_data(request);
    if (data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: on request: no request data!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    if (cpe_http_args_parse_inline(args, &arg_count, data, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: on request: parse data fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    if (!cpe_http_args_all_exists(args, arg_count, require_args, (uint16_t)CPE_ARRAY_SIZE(require_args))) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: on request: not all require args exists!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    app_order_id = cpe_http_args_find_value(args, arg_count, "app_order_id");
    if (app_order_id == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: app_order_id not exist!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }
    if (payment_deliver_svr_parse_product_id(&device_category, &set_id, &service, app_order_id, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: app_order_id %s parse fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, app_order_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    if (service != PAYMENT_RECHARGE_SERVICE_QIHOO) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: app_order_id %s is not build for qihoo(service=%d)!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, app_order_id, service);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    env = payment_deliver_svr_adapter_qihoo_env(svr, request->m_adapter, device_category);
    if (env == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: app_order_id %s qihoo not support device category %d!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, app_order_id, device_category);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    app_key = cpe_http_args_find_value(args, arg_count, "app_key");
    assert(app_key);
    if (strcmp(env->m_appkey, app_key) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: app_order_id %s device category %d app id mismatch, expect %s, but %s!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, app_order_id, device_category,
            env->m_appkey, app_key);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    sign_return = cpe_http_args_find_value(args, arg_count, "sign_return");
    assert(sign_return);

    sign = cpe_http_args_find_value(args, arg_count, "sign");
    assert(sign);

    build_sign = payment_deliver_svr_qihoo_sign(svr, env, args, arg_count);
    if (build_sign == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: build sign fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    if (strcmp(sign, build_sign) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: check sign fail, expect=%s, but=%s!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, sign, build_sign);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }

    notify = payment_deliver_svr_notify_pkg(svr, &notify_pkg, service, device_category, app_order_id);
    if (notify == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: build pkg fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }
    assert(notify_pkg);

    if (dr_http_args_read_args(&notify->data.qihoo, sizeof(notify->data.qihoo), args, arg_count, svr->m_meta_qihoo_record, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: build record args fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }
    
    if (payment_deliver_request_send_pkg(request, notify_pkg, set_id) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: send pkg fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }
    
    payment_deliver_request_data_clear(request);
    request->m_state = payment_deliver_request_runing;

    return 0;
}

int payment_deliver_svr_qihoo_on_response(payment_deliver_svr_t svr, payment_deliver_request_t request, int svr_error) {
    if (svr_error == 0) {
        payment_deliver_request_set_response(request, "ok");
        return 0;
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: qihoo: server process error, rv=%d!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, svr_error);
        payment_deliver_request_set_error(request, 500, "Internal Server Error");
        return -1;
    }
}

static int payment_deliver_svr_qihoo_sign_arg_cmp(void const * l, void const * r) {
    return strcmp(((cpe_http_arg_t)l)->name, ((cpe_http_arg_t)r)->name);
}

static const char *
payment_deliver_svr_qihoo_sign(
    payment_deliver_svr_t svr, struct payment_deliver_adapter_qihoo_data * env, cpe_http_arg_t args, uint16_t arg_count)
{
    mem_buffer_t buffer = gd_app_tmp_buffer(svr->m_app);
    struct cpe_md5_ctx md5_ctx;
    uint16_t i;

    cpe_md5_ctx_init(&md5_ctx);

    qsort(args, arg_count, sizeof(args[0]), payment_deliver_svr_qihoo_sign_arg_cmp);
    
    for(i = 0; i < arg_count; ++i) {
        cpe_http_arg_t one = &args[i];
        
        if (strcmp(one->name, "sign") == 0) continue;
        if (strcmp(one->name, "sign_return") == 0) continue;

        if (one->value[0] == 0 || strcmp(one->value, "0") == 0) continue;

        cpe_md5_ctx_update(&md5_ctx, one->value, strlen(one->value));
        cpe_md5_ctx_update(&md5_ctx, "#", 1);
	}

    cpe_md5_ctx_update(&md5_ctx, env->m_appsecret, strlen(env->m_appsecret));
    cpe_md5_ctx_final(&md5_ctx);

    return cpe_md5_dump(&md5_ctx.value, buffer);
}
