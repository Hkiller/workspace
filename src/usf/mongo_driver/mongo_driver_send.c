#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/random.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "mongo_server_i.h"
#include "mongo_connection_i.h"
#include "mongo_pkg_i.h"

uint8_t mongo_driver_is_readable(mongo_driver_t driver) {
    return mongo_driver_is_readable_ex(driver, mongo_read_unknown);
}

uint8_t mongo_driver_is_readable_ex(mongo_driver_t driver, mongo_read_mode_t read_mode) {
    mongo_server_t suitable_servers[128];
    uint32_t suitable_server_count = CPE_ARRAY_SIZE(suitable_servers);
    
    mongo_driver_topology_suitable_servers(suitable_servers, &suitable_server_count, driver, 0, read_mode);

    return suitable_server_count > 0 ? 1 : 0;
}

uint8_t mongo_driver_is_writable(mongo_driver_t driver) {
    mongo_server_t suitable_servers[128];
    uint32_t suitable_server_count = CPE_ARRAY_SIZE(suitable_servers);
    mongo_driver_topology_suitable_servers(suitable_servers, &suitable_server_count, driver, 1, mongo_read_unknown);
    return suitable_server_count > 0 ? 1 : 0;
}

mongo_server_t
mongo_driver_select(mongo_driver_t driver, uint8_t for_writes, mongo_read_mode_t read_mode) {
    mongo_server_t suitable_servers[128];
    uint32_t suitable_server_count = CPE_ARRAY_SIZE(suitable_servers);
    mongo_server_t sd = NULL;

   if (driver->m_topology_type == mongo_topology_type_single) {
       sd = TAILQ_FIRST(&driver->m_servers);
       return sd->m_has_is_master ? sd : NULL;
   }

   mongo_driver_topology_suitable_servers(suitable_servers, &suitable_server_count, driver, for_writes, read_mode);
   CPE_ERROR(driver->m_em, "*************count = %d, server.host= %s, m_mode=%d*************", suitable_server_count, &suitable_servers[0]->m_host, suitable_servers[0]->m_mode);
   if (suitable_server_count != 0) {
       sd = suitable_servers[cpe_rand_dft(suitable_server_count)];
   }

   return sd;
}

int mongo_driver_send(mongo_driver_t driver, mongo_pkg_t pkg) {
    mongo_server_t server;
    mongo_connection_t connection;
    uint8_t for_writes = 0;
    bson_iter_t it;
    
    if (!driver->m_is_enable) {
        CPE_ERROR(driver->m_em, "%s: send: driver not enable!", mongo_driver_name(driver));
        return -1;
    }

    switch(pkg->m_pro_head.op) {
    case mongo_db_op_update:
    case mongo_db_op_insert:
    case mongo_db_op_delete:
        for_writes = 1;
        break;
    case mongo_db_op_query:
        if (mongo_pkg_find(&it, pkg, 0, "findandmodify") == 0) {
            for_writes = 1;
        }
        break;
    default:
        break;
    }
CPE_ERROR(driver->m_em, "************for_writes=%d************3", for_writes);
    /*选择一个服务 */
    server = mongo_driver_select(driver, for_writes, pkg->m_read_mode);
    if (server == NULL) {
        uint32_t online_server_count = 0;
        TAILQ_FOREACH(server, &driver->m_servers, m_next) {
            if (!server->m_is_offline) online_server_count++;
        }
        CPE_ERROR(
            driver->m_em, "%s: send: no compatible server, online-server=%d, offline-server=%d!",
            mongo_driver_name(driver), online_server_count, (driver->m_server_count - online_server_count));
        return -1;
    }

    /*选择一个连接 */
    TAILQ_FOREACH(connection, &server->m_connections, m_next_for_server) {
        if (fsm_machine_curent_state(&connection->m_fsm) == mongo_connection_state_connected) {
            TAILQ_REMOVE(&server->m_connections, connection, m_next_for_server);
            TAILQ_INSERT_TAIL(&server->m_connections, connection, m_next_for_server);
            break;
        }
    }
    
    if (connection == NULL) {
        CPE_ERROR(driver->m_em, "%s: send: server %s:%d no connection!", mongo_driver_name(driver), server->m_host, server->m_port);
        return -1;
    }
    
    return mongo_connection_send(connection, pkg);
}

int mongo_driver_on_send(dp_req_t req, void * ctx, error_monitor_t em) {
    mongo_driver_t driver = ctx;
    mongo_pkg_t pkg;

    pkg = mongo_pkg_from_dp_req(req);
    if (pkg == NULL) {
        CPE_ERROR(
            em, "%s: send: input req type %s error!",
            mongo_driver_name(driver), dp_req_type(req));
        return -1;
    }

    return mongo_driver_send(driver, pkg);
}
