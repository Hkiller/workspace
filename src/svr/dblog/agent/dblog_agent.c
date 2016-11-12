#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/string_utils.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/net/net_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "protocol/svr/dblog/svr_dblog_pro.h"
#include "dblog_agent_i.h"

static void dblog_agent_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_dblog_agent = {
    "dblog_agent",
    dblog_agent_clear
};

dblog_agent_t
dblog_agent_create(
    gd_app_context_t app, const char * name, uint16_t set_id, uint16_t set_type, mem_allocrator_t alloc, error_monitor_t em)
{
    struct dblog_agent * agent;
    nm_node_t agent_node;

    assert(app);

    agent_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct dblog_agent));
    if (agent_node == NULL) return NULL;

    agent = (dblog_agent_t)nm_node_data(agent_node);

    agent->m_app = app;
    agent->m_alloc = alloc;
    agent->m_em = em;
    agent->m_set_id = set_id;
    agent->m_set_type = set_type;
    agent->m_debug = 0;
    agent->m_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if(agent->m_fd < 0) { 
        CPE_ERROR(agent->m_em, "dblog: init: create socked fail, rv=%d (%s)", errno, strerror(errno));
        nm_node_free(agent_node);
        return NULL;
    }

    mem_buffer_init(&agent->m_buffer, alloc);

    nm_node_set_type(agent_node, &s_nm_node_type_dblog_agent);

    return agent;
}

static void dblog_agent_clear(nm_node_t node) {
    dblog_agent_t agent;
    agent = (dblog_agent_t)nm_node_data(node);

    assert(agent->m_fd != -1);
    cpe_sock_close(agent->m_fd);
    agent->m_fd = -1;

    mem_buffer_clear(&agent->m_buffer);
}

void dblog_agent_free(dblog_agent_t agent) {
    nm_node_t agent_node;
    assert(agent);

    agent_node = nm_node_from_data(agent);
    if (nm_node_type(agent_node) != &s_nm_node_type_dblog_agent) return;
    nm_node_free(agent_node);
}

gd_app_context_t dblog_agent_app(dblog_agent_t agent) {
    return agent->m_app;
}

dblog_agent_t
dblog_agent_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dblog_agent) return NULL;
    return (dblog_agent_t)nm_node_data(node);
}

dblog_agent_t
dblog_agent_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "dblog_agent";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dblog_agent) return NULL;
    return (dblog_agent_t)nm_node_data(node);
}

const char * dblog_agent_name(dblog_agent_t agent) {
    return nm_node_name(nm_node_from_data(agent));
}

cpe_hash_string_t
dblog_agent_name_hs(dblog_agent_t agent) {
    return nm_node_name_hs(nm_node_from_data(agent));
}

uint32_t dblog_agent_cur_time(dblog_agent_t agent) {
    return tl_manage_time_sec(gd_app_tl_mgr(agent->m_app));
}

int dblog_agent_set_target(dblog_agent_t agent, const char * ip, uint16_t port) {
    cpe_str_dup(agent->m_ip, sizeof(agent->m_ip), ip);
    agent->m_port = port;

    bzero(&agent->m_target, sizeof(agent->m_target)); 
    agent->m_target.sin_family = AF_INET; 
    agent->m_target.sin_addr.s_addr = inet_addr(ip); 
    agent->m_target.sin_port = htons(port);

    return 0;
}

int dblog_agent_log(dblog_agent_t agent, void const * data, size_t data_size, LPDRMETA data_meta) {
    char * pkg;
    size_t pkg_capacity = 12 + data_size * 3;
    uint16_t pkg_used_size = 0; 
    int rv;
    uint64_t buf64;
    uint16_t buf16;

    if (agent->m_ip[0] == 0 || agent->m_port == 0) {
        if (agent->m_debug >= 2) {
            CPE_INFO(agent->m_em, "dblog: log: target not set, ignore send %s[size=%d]", dr_meta_name(data_meta), (int)data_size);
        }
        return -1;
    }

    if (pkg_capacity > mem_buffer_size(&agent->m_buffer)) {
        mem_buffer_set_size(&agent->m_buffer, pkg_capacity);
    }
    pkg = (char *)mem_buffer_make_continuous(&agent->m_buffer, 0);

    pkg[pkg_used_size] = SVR_DBLOG_CMD_REQ_LOG;
    pkg_used_size++;
    
    CPE_COPY_HTON16(pkg + pkg_used_size, &agent->m_set_type);
    pkg_used_size+=2;

    CPE_COPY_HTON16(pkg + pkg_used_size, &agent->m_set_id);
    pkg_used_size+=2;

    buf64 = tl_manage_time(gd_app_tl_mgr(agent->m_app));
    CPE_COPY_HTON64(pkg + pkg_used_size, &buf64);
    pkg_used_size += 8;
        
    buf16 = dr_meta_id(data_meta);
    CPE_COPY_HTON16(pkg + pkg_used_size, &buf16);
    pkg_used_size += 2;

    rv = dr_pbuf_write(pkg + pkg_used_size, pkg_capacity - pkg_used_size, data, data_size, data_meta, agent->m_em);
    if (rv < 0) {
        CPE_ERROR(agent->m_em, "dblog: %s: encode fail, rv=%d", dr_meta_name(data_meta), rv);
        return -1;
    }
    else if (rv > UINT16_MAX) {
        CPE_ERROR(agent->m_em, "dblog: %s: pkg-size %d size overflow", dr_meta_name(data_meta), (int)rv);
        return -1;
    }

    pkg_used_size += (uint16_t)rv;

    if (pkg_used_size > 1472  /*max udp package size*/) {
        CPE_ERROR(agent->m_em, "dblog: log: %s: package size %d too long, max size 1472", dr_meta_name(data_meta), pkg_used_size);
        return -1;
    }
    
    if(sendto(agent->m_fd, pkg, pkg_used_size, 0, (struct sockaddr*)&agent->m_target, sizeof(agent->m_target)) < 0) {
        CPE_ERROR(agent->m_em, "dblog: log: %s: send fail, size=%d", dr_meta_name(data_meta), pkg_used_size);
        return -1;
    }

    if (agent->m_debug >= 2) {
        CPE_INFO(
            agent->m_em, "dblog: log: %s(data-size=%d, net-size=%d): %s",
            dr_meta_name(data_meta), rv, pkg_used_size,
            dr_json_dump_inline(&agent->m_buffer, data, data_size, data_meta));
    }
    
    return 0;
}
