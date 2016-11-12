#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "mongo_cli_internal_ops.h"

static void mongo_cli_proxy_clear(nm_node_t node);
extern char g_metalib_mongo_cli[];

struct nm_node_type s_nm_node_type_mongo_cli_proxy = {
    "usf_mongo_cli_proxy",
    mongo_cli_proxy_clear
};

mongo_cli_proxy_t
mongo_cli_proxy_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    mongo_driver_t mongo_driver,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    char name_buf[64];
    mongo_cli_proxy_t proxy;
    nm_node_t proxy_node;

    proxy_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct mongo_cli_proxy));
    if (proxy_node == NULL) return NULL;

    proxy = (mongo_cli_proxy_t)nm_node_data(proxy_node);
    bzero(proxy, sizeof(struct mongo_cli_proxy));

    proxy->m_app = app;
    proxy->m_alloc = alloc;
    proxy->m_em = em;
    proxy->m_debug = 0;
    proxy->m_driver = mongo_driver;
    proxy->m_pkg_buf_max_size = 4 * 1024;

    proxy->m_meta_lasterror = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_mongo_cli, "mongo_lasterror");
    if (proxy->m_meta_lasterror == NULL) {
        CPE_ERROR(em, "%s: find lasterror meta mongo_lasterror fail!", name);
        nm_node_free(proxy_node);
        return NULL;
    }

    snprintf(name_buf, sizeof(name_buf), "%s.require_queue", name);
    proxy->m_require_queue = logic_require_queue_create(app, alloc, em, name_buf, logic_mgr, sizeof(struct mongo_cli_require_keep));
    if (proxy->m_require_queue == NULL) {
        CPE_ERROR(em, "%s: create logic usr require queue fail!", name);
        nm_node_free(proxy_node);
        return NULL;
    }

    nm_node_set_type(proxy_node, &s_nm_node_type_mongo_cli_proxy);

    return proxy;
} 

static void mongo_cli_proxy_clear(nm_node_t node) {
    mongo_cli_proxy_t proxy;
    proxy = (mongo_cli_proxy_t)nm_node_data(node);

    logic_require_queue_free(proxy->m_require_queue);

    if (proxy->m_outgoing_send_to) mem_free(proxy->m_alloc, proxy->m_outgoing_send_to);
}

void mongo_cli_proxy_free(mongo_cli_proxy_t proxy) {
    nm_node_t proxy_node;
    assert(proxy);

    proxy_node = nm_node_from_data(proxy);
    if (nm_node_type(proxy_node) != &s_nm_node_type_mongo_cli_proxy) return;
    nm_node_free(proxy_node);
}

mongo_cli_proxy_t
mongo_cli_proxy_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_cli_proxy) return NULL;
    return (mongo_cli_proxy_t)nm_node_data(node);
}

mongo_cli_proxy_t
mongo_cli_proxy_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "mongo_cli_proxy";
    
    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_cli_proxy) return NULL;
    return (mongo_cli_proxy_t)nm_node_data(node);
}

gd_app_context_t mongo_cli_proxy_app(mongo_cli_proxy_t proxy) {
    return proxy->m_app;
}

const char * mongo_cli_proxy_name(mongo_cli_proxy_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

const char * mongo_cli_proxy_dft_db(mongo_cli_proxy_t agent) {
    return agent->m_dft_db;
}

void mongo_cli_proxy_set_dft_db(mongo_cli_proxy_t agent, const char * db_name) {
    if (db_name) {
        cpe_str_dup(agent->m_dft_db, sizeof(agent->m_dft_db), db_name);
    }
    else {
        agent->m_dft_db[0] = 0;
    }
}

cpe_hash_string_t
mongo_cli_proxy_name_hs(mongo_cli_proxy_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

logic_manage_t mongo_cli_proxy_logic_manage(mongo_cli_proxy_t agent) {
    return logic_require_queue_logic_manage(agent->m_require_queue);
}

mongo_driver_t mongo_cli_proxy_driver(mongo_cli_proxy_t agent) {
    return agent->m_driver;
}

int mongo_cli_proxy_set_outgoing_send_to(mongo_cli_proxy_t proxy, const char * outgoing_send_to) {
    size_t name_len = cpe_hs_len_to_binary_len(strlen(outgoing_send_to));
    cpe_hash_string_t buf;

    buf = mem_alloc(proxy->m_alloc, name_len);
    if (buf == NULL) return -1;

    cpe_hs_init(buf, name_len, outgoing_send_to);

    if (proxy->m_outgoing_send_to) mem_free(proxy->m_alloc, proxy->m_outgoing_send_to);

    proxy->m_outgoing_send_to = buf;

    return 0;
}

int mongo_cli_proxy_set_incoming_recv_at(mongo_cli_proxy_t proxy, const char * incoming_recv_at) {
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.incoming-recv-rsp", mongo_cli_proxy_name(proxy));

    if (proxy->m_incoming_recv_at) dp_rsp_free(proxy->m_incoming_recv_at);

    proxy->m_incoming_recv_at = dp_rsp_create(gd_app_dp_mgr(proxy->m_app), name_buf);
    if (proxy->m_incoming_recv_at == NULL) return -1;

    dp_rsp_set_processor(proxy->m_incoming_recv_at, mongo_cli_proxy_recv, proxy);

    if (dp_rsp_bind_string(proxy->m_incoming_recv_at, incoming_recv_at, proxy->m_em) != 0) {
        CPE_ERROR(
            proxy->m_em, "%s: set incoming_recv_at: bind to %s fail!",
            mongo_cli_proxy_name(proxy), incoming_recv_at);
        dp_rsp_free(proxy->m_incoming_recv_at);
        proxy->m_incoming_recv_at = NULL;
        return -1;
    }

    return 0;
}

mongo_pkg_t
mongo_cli_proxy_pkg_buf(mongo_cli_proxy_t proxy) {
    if (proxy->m_pkg_buf) {
        if (mongo_pkg_capacity(proxy->m_pkg_buf) < proxy->m_pkg_buf_max_size) {
            mongo_pkg_free(proxy->m_pkg_buf);
            proxy->m_pkg_buf = NULL;
        }
    }

    if (proxy->m_pkg_buf == NULL) {
        proxy->m_pkg_buf = mongo_pkg_create(proxy->m_driver, proxy->m_pkg_buf_max_size);
    }

    return proxy->m_pkg_buf;
}

mongo_pkg_t
mongo_cli_proxy_cmd_buf(mongo_cli_proxy_t proxy) {
    if (proxy->m_cmd_buf == NULL) {
        proxy->m_cmd_buf = mongo_pkg_create(proxy->m_driver, 128);
    }

    return proxy->m_cmd_buf;
}
