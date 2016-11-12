#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "chat_svr_ops.h"

EXPORT_DIRECTIVE
int chat_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    chat_svr_t chat_svr;
    set_svr_stub_t stub;
    uint32_t check_span_ms;
    uint32_t check_once_process_count;
    const char * send_to;
    const char * recv_at;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    if ((send_to = cfg_get_string(cfg, "send-to", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((recv_at = cfg_get_string(cfg, "recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "check-span-ms", &check_span_ms) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: check-span-ms not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cfg_try_get_uint32(cfg, "check-once-process-count", &check_once_process_count) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: check-once-process-count not configured!", gd_app_module_name(module));
        return -1;
    }

    chat_svr =
        chat_svr_create(
            app, gd_app_module_name(module), stub,
            gd_app_alloc(app), gd_app_em(app));
    if (chat_svr == NULL) return -1;

    chat_svr->m_debug = cfg_get_int8(cfg, "debug", chat_svr->m_debug);

    if (chat_svr_set_send_to(chat_svr, send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to %s fail!", gd_app_module_name(module), send_to);
        return -1;
    }

    if (chat_svr_set_recv_at(chat_svr, recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set recv-at %s fail!", gd_app_module_name(module), recv_at);
        return -1;
    }

    if (chat_svr_set_check_span(chat_svr, check_span_ms) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set check-span-ms %d fail!", gd_app_module_name(module), check_span_ms);
        chat_svr_free(chat_svr);
        return -1;
    }

    chat_svr->m_check_once_process_count = check_once_process_count;

    if (chat_svr_meta_chanel_load(chat_svr, cfg_find_cfg(gd_app_cfg(app), "meta.chanel_def")) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load chanel_def fail!", gd_app_module_name(module));
        chat_svr_free(chat_svr);
        return -1;
    }

    if (chat_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void chat_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    chat_svr_t chat_svr;

    chat_svr = chat_svr_find_nc(app, gd_app_module_name(module));
    if (chat_svr) {
        chat_svr_free(chat_svr);
    }
}
