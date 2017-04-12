#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/string_url.h"
#include "cpe/utils/md5.h"
#include "cpe/utils/http_args.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_http_args.h"
#include "gd/app/app_context.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "payment_deliver_svr_adapter_damai.h"
#include "payment_deliver_svr_adapter.h"
#include "payment_deliver_svr_adapter_type.h"
#include "payment_deliver_svr_request.h"
#include "payment_deliver_svr_connection.h"
#include "payment_deliver_svr_utils.h"

static int payment_deliver_svr_damai_data_init(payment_deliver_svr_t svr, struct payment_deliver_adapter_damai_data * data, cfg_t cfg);
static void payment_deliver_svr_damai_data_fini(payment_deliver_svr_t svr, struct payment_deliver_adapter_damai_data * data);
static const char * payment_deliver_svr_damai_sign(payment_deliver_svr_t svr, struct payment_deliver_adapter_damai_data * env, cpe_http_arg_t args, uint16_t arg_count);

int payment_deliver_svr_damai_init(payment_deliver_adapter_t adapter, cfg_t cfg) {
    payment_deliver_svr_t svr = adapter->m_svr;
    struct payment_deliver_adapter_damai * damai = (struct payment_deliver_adapter_damai *)adapter->m_private;
    cfg_t platform_cfg;
    
    assert(sizeof(*damai) <= sizeof(adapter->m_private));

    if ((platform_cfg = cfg_find_cfg(cfg, "android"))) {
        if (payment_deliver_svr_damai_data_init(svr, &damai->m_android, platform_cfg) != 0) {
            return -1;
        }
    }
    
    return 0;
}

void payment_deliver_svr_damai_fini(payment_deliver_adapter_t adapter) {
    payment_deliver_svr_t svr = adapter->m_svr;
    struct payment_deliver_adapter_damai * damai = (struct payment_deliver_adapter_damai *)adapter->m_private;

    payment_deliver_svr_damai_data_fini(svr, &damai->m_android);
}

static int payment_deliver_svr_damai_data_init(payment_deliver_svr_t svr, struct payment_deliver_adapter_damai_data * data, cfg_t cfg) {
    const char * appid = cfg_get_string(cfg, "appid", NULL);
    const char * appkey = cfg_get_string(cfg, "appkey", NULL);

    if (appid == NULL || appkey == NULL || appkey == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver_svr: damai: config error, appid=%s, appkey=%s", appid, appkey);
        return -1;
    }

    data->m_appid = cpe_str_mem_dup(svr->m_alloc, appid);
    data->m_appkey = cpe_str_mem_dup(svr->m_alloc, appkey);
    if (data->m_appid == NULL || data->m_appkey == NULL) {
        CPE_ERROR(svr->m_em, "payment_deliver_svr: damai: load config alloc fail!");

        if (data->m_appid) { mem_free(svr->m_alloc, data->m_appid); data->m_appid = NULL; }
        if (data->m_appkey) { mem_free(svr->m_alloc, data->m_appkey); data->m_appkey = NULL; }

        return -1;
    }

    return 0;
}

static void payment_deliver_svr_damai_data_fini(payment_deliver_svr_t svr, struct payment_deliver_adapter_damai_data * data) {
    if (data->m_appid) { mem_free(svr->m_alloc, data->m_appid); data->m_appid = NULL; }
    if (data->m_appkey) { mem_free(svr->m_alloc, data->m_appkey); data->m_appkey = NULL; }
}

static struct payment_deliver_adapter_damai_data *
payment_deliver_svr_adapter_damai_env(payment_deliver_svr_t svr, payment_deliver_adapter_t adapter, uint8_t device) {
    struct payment_deliver_adapter_damai * damai = (struct payment_deliver_adapter_damai *)adapter->m_private;
    
    switch(device) {
    case svr_payment_device_windows:
        CPE_ERROR(svr->m_em, "%s: damai: not support device windows", payment_deliver_svr_name(svr));
        return NULL;
    case svr_payment_device_ios:
        CPE_ERROR(svr->m_em, "%s: damai: not support device ios", payment_deliver_svr_name(svr));
        return NULL;
    case svr_payment_device_android:
        return &damai->m_android;
    default:
        CPE_ERROR(svr->m_em, "%s: damai: unknown device category %d", payment_deliver_svr_name(svr), device);
        return NULL;
    }
}

int payment_deliver_svr_damai_on_request(payment_deliver_svr_t svr, payment_deliver_request_t request) {
    struct payment_deliver_adapter_damai_data * env;
    void * data;
    struct cpe_http_arg args[64];
    uint16_t arg_count = CPE_ARRAY_SIZE(args);
    const char * require_args[] = { "orderid", "username", "appid", "roleid", "serverid", "amount", "paytime", "attach", "productname" };
    char * app_order_id;
    char * sign;
    uint8_t device_category;
    uint16_t set_id;
    uint16_t service;
    const char * build_sign;
    dp_req_t notify_pkg;
    SVR_PAYMENT_REQ_NOTIFY * notify;

    data = payment_deliver_request_data(request);
    if (data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: on request: no request data!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }

    if (cpe_http_args_parse_inline(args, &arg_count, data, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: on request: parse data fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }

    if (!cpe_http_args_all_exists(args, arg_count, require_args, (uint16_t)CPE_ARRAY_SIZE(require_args))) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: on request: not all require args exists!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }

    app_order_id = cpe_http_args_find_value(args, arg_count, "attach");
    if (app_order_id == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: attach not exist!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }
    if (payment_deliver_svr_parse_product_id(&device_category, &set_id, &service, app_order_id, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: app_order_id %s parse fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, app_order_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }

    if (service != PAYMENT_RECHARGE_SERVICE_DAMAI) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: app_order_id %s is not build for damai(service=%d)!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, app_order_id, service);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }

    env = payment_deliver_svr_adapter_damai_env(svr, request->m_adapter, device_category);
    if (env == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: app_order_id %s damai not support device category %d!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, app_order_id, device_category);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }

    sign = cpe_http_args_find_value(args, arg_count, "sign");
    assert(sign);

    build_sign = payment_deliver_svr_damai_sign(svr, env, args, arg_count);
    if (build_sign == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: build sign fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }

    if (strcmp(sign, build_sign) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: check sign fail, expect=%s, but=%s!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, sign, build_sign);
        payment_deliver_request_set_response(request, "errorSign");
        return -1;
    }

    notify = payment_deliver_svr_notify_pkg(svr, &notify_pkg, service, device_category, app_order_id);
    if (notify == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: build pkg fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }
    assert(notify_pkg);

    if (dr_http_args_read_args(&notify->data.damai, sizeof(notify->data.damai), args, arg_count, svr->m_meta_damai_record, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: build record args fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }
    
    if (payment_deliver_request_send_pkg(request, notify_pkg, set_id) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: send pkg fail!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }
    
    payment_deliver_request_data_clear(request);
    request->m_state = payment_deliver_request_runing;

    return 0;
}

int payment_deliver_svr_damai_on_response(payment_deliver_svr_t svr, payment_deliver_request_t request, int svr_error) {
    if (svr_error == 0 || svr_error == SVR_PAYMENT_ERRNO_RECHARGE_PROCESSED) {
        payment_deliver_request_set_response(request, "success");
        //payment_deliver_request_set_http_response(request, "text/plain", "success");
        return 0;
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: request %d at connection %d: damai: server process error, rv=%d!",
            payment_deliver_svr_name(svr), request->m_id, request->m_connection->m_id, svr_error);
        payment_deliver_request_set_response(request, "error");
        return -1;
    }
}

static const char *
payment_deliver_svr_damai_sign(
    payment_deliver_svr_t svr, struct payment_deliver_adapter_damai_data * env, cpe_http_arg_t args, uint16_t arg_count)
{
    mem_buffer_t buffer = gd_app_tmp_buffer(svr->m_app);
    struct cpe_md5_ctx md5_ctx;
    uint16_t i;
    const char * sign_args[] = { "orderid", "username", "appid", "roleid", "serverid", "amount", "paytime", "attach", "productname" };

    cpe_md5_ctx_init(&md5_ctx);

    for(i = 0; i < CPE_ARRAY_SIZE(sign_args); ++i) {
        const char * name = sign_args[i];
        char * value = cpe_http_args_find_value(args, arg_count, name);

        cpe_md5_ctx_update(&md5_ctx, name, strlen(name));
        cpe_md5_ctx_update(&md5_ctx, "=", 1);

        if (value) {
            char encode_buf[256];
            ssize_t encode_buf_len = cpe_url_encode(encode_buf, sizeof(encode_buf), value, strlen(value), svr->m_em);
            if (encode_buf_len < 0) {
                CPE_ERROR(svr->m_em, "payment_deliver_svr_damai_sign: urlencode %s fail", value);
                return NULL;
            }
            //cpe_str_tolower(buf);
            cpe_md5_ctx_update(&md5_ctx, encode_buf, encode_buf_len);
        }
        cpe_md5_ctx_update(&md5_ctx, "&", 1);
	}

    cpe_md5_ctx_update(&md5_ctx, "appkey=", strlen("appkey="));
    cpe_md5_ctx_update(&md5_ctx, env->m_appkey, strlen(env->m_appkey));
    cpe_md5_ctx_final(&md5_ctx);

    return cpe_md5_dump(&md5_ctx.value, buffer);
}
