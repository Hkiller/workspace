#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "set_svr_listener.h"

int set_svr_app_init_listener(set_svr_listener_t listener, cfg_t cfg, const char * listener_addr) {
    const char * sep;
    int accept_queue_size;
    char ip[64];
    uint16_t port;
    
    assert(listener->m_fd == -1);

    if (listener_addr == NULL) {
        if (listener->m_svr->m_debug) {
            CPE_INFO(listener->m_svr->m_em, "%s: create: listener: no listener ip!", set_svr_name(listener->m_svr));
        }
        return 0;
    }
    
    sep = strchr(listener_addr, ':');
    if (sep == NULL) {
        CPE_ERROR(
            listener->m_svr->m_em, "%s: create: listener: listener_addr %s format error!",
            set_svr_name(listener->m_svr), listener_addr);
        return -1;
    }
        
    if (cpe_str_dup_range(ip, sizeof(ip), listener_addr, sep) == NULL) {
        CPE_ERROR(
            listener->m_svr->m_em, "%s: create: listener: listener_addr %s too long!",
            set_svr_name(listener->m_svr), listener_addr);
        return -1;
    }
        
    port = atoi(sep + 1);

    accept_queue_size = cfg_get_int32(cfg, "accept-queue-size", 64);

    return set_svr_listener_start(listener, ip, port, accept_queue_size);
}


