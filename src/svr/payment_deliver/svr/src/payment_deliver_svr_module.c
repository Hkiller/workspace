#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "payment_deliver_svr.h"
#include "payment_deliver_svr_adapter.h"

EXPORT_DIRECTIVE
int payment_deliver_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    payment_deliver_svr_t payment_deliver_svr;
    const char * response_recv_at;
    const char * str_port;
    uint16_t port;
    const char * str_buf_size;
    uint64_t buf_size;
    const char * payment_svr_type_name;
    set_svr_svr_info_t payment_svr_info;

    payment_svr_type_name = gd_app_arg_find(app, "--payment-svr-type");
    if (payment_svr_type_name == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: payment-svr-type not set in args!", gd_app_module_name(module));
        return -1;
    }
    
    str_port = gd_app_arg_find(app, "--port");
    if (str_port == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: port not set in args!", gd_app_module_name(module));
        return -1;
    }

    port = atoi(str_port);;

    str_buf_size = gd_app_arg_find(app, "--buf-size");
    if (str_buf_size == NULL) str_buf_size = cfg_get_string(cfg, "buf-size", NULL);
    if (str_buf_size == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: buf-size not configured in args or config!", gd_app_module_name(module));
        return -1;
    }
    if (cpe_str_parse_byte_size(&buf_size, str_buf_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: buf-size %s format error!", gd_app_module_name(module), str_buf_size);
        return -1;
    }

    if ((response_recv_at = cfg_get_string(cfg, "response-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: response-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    payment_svr_info = set_svr_svr_info_find_by_name(stub, payment_svr_type_name);
    if (payment_svr_info == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: svr type %s not exist!", gd_app_module_name(module), payment_svr_type_name);
        return -1;
    }
    
    payment_deliver_svr =
        payment_deliver_svr_create(
            app, gd_app_module_name(module),
            stub, payment_svr_info, port, gd_app_alloc(app), gd_app_em(app));
    if (payment_deliver_svr == NULL) return -1;

    payment_deliver_svr->m_debug = cfg_get_int8(cfg, "debug", payment_deliver_svr->m_debug);

    if (payment_deliver_svr_set_response_recv_at(payment_deliver_svr, response_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set response-recv-at %s fail!", gd_app_module_name(module), response_recv_at);
        payment_deliver_svr_free(payment_deliver_svr);
        return -1;
    }

    if (payment_deliver_svr_set_ringbuf_size(payment_deliver_svr, buf_size) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set buf size to %d fail!",
            payment_deliver_svr_name(payment_deliver_svr), (int)buf_size);
        return -1;
    }

    if (payment_deliver_adapter_load(payment_deliver_svr, cfg_find_cfg(gd_app_cfg(app), "adapter")) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load adapters fail!", gd_app_module_name(module));
        payment_deliver_svr_free(payment_deliver_svr);
        return -1;
    }
    
    if (payment_deliver_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done. listen at %d", gd_app_module_name(module), port);
    }

    return 0;
}

EXPORT_DIRECTIVE
void payment_deliver_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    payment_deliver_svr_t payment_deliver_svr;

    payment_deliver_svr = payment_deliver_svr_find_nc(app, gd_app_module_name(module));
    if (payment_deliver_svr) {
        payment_deliver_svr_free(payment_deliver_svr);
    }
}
