#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "mongo_driver_i.h"

EXPORT_DIRECTIVE
int mongo_driver_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    const char * incoming_send_to;
    const char * outgoing_recv_at;
    const char * uri;
    mongo_driver_t driver;
    const char * str_ringbuf_size;
    uint64_t ringbuf_size;

    /*读取uri*/
    if ((uri = cfg_get_string(cfg, "uri", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: uri not configured!", gd_app_module_name(module));
        return -1;
    }
    
    if (uri[0] == '$') {
        char arg_name_buf[64];
        snprintf(arg_name_buf, sizeof(arg_name_buf), "--%s", uri + 1);
        uri = gd_app_arg_find(app, arg_name_buf);
        if (uri == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read uri %s not configured in command line!",
                gd_app_module_name(module), arg_name_buf);
            return -1;
        }
    }
    
    /*读取ringbuf-size*/
    if ((str_ringbuf_size = cfg_get_string(cfg, "ringbuf-size", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ringbuf-size not configured!", gd_app_module_name(module));
        return -1;
    }
    
    if (str_ringbuf_size[0] == '$') {
        char arg_name_buf[64];
        snprintf(arg_name_buf, sizeof(arg_name_buf), "--%s", str_ringbuf_size + 1);
        str_ringbuf_size = gd_app_arg_find(app, arg_name_buf);
        if (str_ringbuf_size == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read ringbuf-size %s not configured in command line!",
                gd_app_module_name(module), arg_name_buf);
            return -1;
        }
    }

    if (cpe_str_parse_byte_size(&ringbuf_size, str_ringbuf_size) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: read ringbuf-size %s fail!",
            gd_app_module_name(module), str_ringbuf_size);
        return -1;
    }

    /*读取可选参数 */
    incoming_send_to = cfg_get_string(cfg, "incoming-send-to", NULL);
    outgoing_recv_at = cfg_get_string(cfg, "outgoing-recv-at", NULL);

    driver = mongo_driver_create(app, gd_app_module_name(module), gd_app_alloc(app), gd_app_em(app));
    if (driver == NULL) return -1;

    driver->m_pkg_buf_max_size = cfg_get_uint32(cfg, "pkg-buf-max-size", driver->m_pkg_buf_max_size);
    driver->m_reconnect_span_s = cfg_get_uint32(cfg, "reconnect-span-s", driver->m_reconnect_span_s);
    driver->m_op_timeout_ms = cfg_get_uint32(cfg, "op-timeout-ms", driver->m_op_timeout_ms);
    driver->m_read_block_size = cfg_get_uint32(cfg, "read-block-size", driver->m_read_block_size);

    driver->m_debug = cfg_get_int32(cfg, "debug", driver->m_debug);

    if (mongo_driver_set_ringbuf_size(driver, ringbuf_size) != 0
        || mongo_driver_set_uri( driver, uri) != 0
        || mongo_driver_set_incoming_send_to(driver, incoming_send_to) != 0
        || mongo_driver_set_outgoing_recv_at(driver, outgoing_recv_at) != 0
        )
    {
        mongo_driver_free(driver);
        return -1;
    }

    if (cfg_get_int32(cfg, "auto-enable", 0)) {
        if (mongo_driver_enable(driver) != 0) {
            CPE_ERROR(gd_app_em(app), "%s create: enable error!", gd_app_module_name(module));
            mongo_driver_free(driver);
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void mongo_driver_app_fini(gd_app_context_t app, gd_app_module_t module) {
    mongo_driver_t driver;

    driver = mongo_driver_find_nc(app, gd_app_module_name(module));
    if (driver) {
        mongo_driver_free(driver);
    }
}
