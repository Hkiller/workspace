#ifndef SVR_DBLOG_SVR_TYPES_H
#define SVR_DBLOG_SVR_TYPES_H
#include "ev.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/pal/pal_socket.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"

typedef struct dblog_svr * dblog_svr_t;
typedef struct dblog_svr_meta * dblog_svr_meta_t;

struct dblog_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;
    struct ev_loop * m_ev_loop;

    char m_ip[32];
    uint16_t m_port;
    int m_fd;
    ev_io m_watcher;

    char m_input_buf[2048];
    char m_decode_buf[4096];

    mongo_driver_t m_db;
    char m_db_ns[32];
    
    struct cpe_hash_table m_metas;

    struct mem_buffer m_dump_buffer;
};

/*operations of dblog_svr */
dblog_svr_t
dblog_svr_create(
    gd_app_context_t app, const char * name,
    set_svr_stub_t stub, mongo_driver_t db,
    mem_allocrator_t alloc, error_monitor_t em);

void dblog_svr_free(dblog_svr_t svr);

dblog_svr_t dblog_svr_find(gd_app_context_t app, cpe_hash_string_t name);
dblog_svr_t dblog_svr_find_nc(gd_app_context_t app, const char * name);
const char * dblog_svr_name(dblog_svr_t svr);

int dblog_svr_listen(dblog_svr_t dblog_svr, const char * ip, uint16_t port);

void dblog_svr_net_cb(EV_P_ ev_io *w, int revents);

#endif
