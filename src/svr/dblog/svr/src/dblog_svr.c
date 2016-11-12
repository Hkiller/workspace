#include <assert.h>
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/string_utils.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "dblog_svr_i.h"
#include "dblog_svr_meta.h"

static void dblog_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_dblog_svr = {
    "dblog_svr",
    dblog_svr_clear
};

dblog_svr_t
dblog_svr_create(
    gd_app_context_t app, const char * name,
    set_svr_stub_t stub, mongo_driver_t db,
    mem_allocrator_t alloc, error_monitor_t em)
{
    dblog_svr_t svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct dblog_svr));
    if (svr_node == NULL) return NULL;

    svr = (dblog_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_db = db;
    svr->m_debug = 0;
    svr->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    svr->m_fd = -1;
    
    if (cpe_hash_table_init(
            &svr->m_metas,
            svr->m_alloc,
            (cpe_hash_fun_t) dblog_svr_meta_hash,
            (cpe_hash_eq_t) dblog_svr_meta_eq,
            CPE_HASH_OBJ2ENTRY(dblog_svr_meta, m_hh),
            -1) != 0)
    {
        mem_free(alloc, svr);
        return NULL;
    }

    mem_buffer_init(&svr->m_dump_buffer, NULL);

    nm_node_set_type(svr_node, &s_nm_node_type_dblog_svr);

    return svr;
}

static void dblog_svr_clear(nm_node_t node) {
    dblog_svr_t svr;
    svr = (dblog_svr_t)nm_node_data(node);

    if (svr->m_fd >= 0) {
        ev_io_stop(svr->m_ev_loop, &svr->m_watcher);
        cpe_sock_close(svr->m_fd);
        svr->m_fd = -1;
    }

    dblog_svr_meta_free_all(svr);
    cpe_hash_table_fini(&svr->m_metas);

    mem_buffer_clear(&svr->m_dump_buffer);
}

void dblog_svr_free(dblog_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_dblog_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t dblog_svr_app(dblog_svr_t svr) {
    return svr->m_app;
}

dblog_svr_t
dblog_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dblog_svr) return NULL;
    return (dblog_svr_t)nm_node_data(node);
}

dblog_svr_t
dblog_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dblog_svr) return NULL;
    return (dblog_svr_t)nm_node_data(node);
}

const char * dblog_svr_name(dblog_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
dblog_svr_name_hs(dblog_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t dblog_svr_cur_time(dblog_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int dblog_svr_listen(dblog_svr_t dblog_svr, const char * ip, uint16_t port) {
    struct sockaddr_in addr;

    if (dblog_svr->m_fd != -1) {
        cpe_sock_close(dblog_svr->m_fd);
    }

    cpe_str_dup(dblog_svr->m_ip, sizeof(dblog_svr->m_ip), ip);
    dblog_svr->m_port = port;
    
    dblog_svr->m_fd = cpe_sock_open(PF_INET, SOCK_DGRAM, 0);
    if (dblog_svr->m_fd < 0) {
        CPE_ERROR(
            dblog_svr->m_em, "%s: start listen: socket error, errno=%d (%s)",
            dblog_svr_name(dblog_svr), errno, strerror(errno));
        return -1;
    }

    if (cpe_sock_set_reuseaddr(dblog_svr->m_fd, 1) != 0) {
        CPE_ERROR(
            dblog_svr->m_em, "%s: start listen: set sock reuseaddr fail, errno=%d (%s)!",
            dblog_svr_name(dblog_svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        close(dblog_svr->m_fd);
        dblog_svr->m_fd = -1;
        return -1;
    }
    
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip[0] ? inet_addr(ip) : INADDR_ANY;
    if (bind(dblog_svr->m_fd, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
        CPE_ERROR(
            dblog_svr->m_em, "%s: start listen: bind error, port=%d, errno=%d (%s)",
            dblog_svr_name(dblog_svr), port, errno, strerror(errno));
        close(dblog_svr->m_fd);
        dblog_svr->m_fd = -1;
        return -1;
    }

    dblog_svr->m_watcher.data = dblog_svr;
    ev_io_init(&dblog_svr->m_watcher, dblog_svr_net_cb, dblog_svr->m_fd, EV_READ);
    ev_io_start(dblog_svr->m_ev_loop, &dblog_svr->m_watcher);

    if (dblog_svr->m_debug) {
        CPE_INFO(dblog_svr->m_em, "%s: start listen at %s:%d", dblog_svr_name(dblog_svr), ip, port);
    }

    return 0;
}
