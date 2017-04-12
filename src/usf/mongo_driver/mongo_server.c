#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "mongo_server_i.h"
#include "mongo_connection_i.h"
#include "mongo_pkg_i.h"

mongo_server_t mongo_server_create(mongo_driver_t driver, const char * host, int port, mongo_server_source_t source) {
    struct mongo_server * server;

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        if (strcmp(server->m_host, host) == 0 && server->m_port == port) {
            CPE_ERROR(
                driver->m_em, "%s: server %s.%d: create: duplicate!",
                mongo_driver_name(driver), host, port);
            return NULL;
        }
    }

    server = mem_alloc(driver->m_alloc, sizeof(struct mongo_server));
    if (server == NULL) {
        CPE_ERROR(
            driver->m_em, "%s: server %s.%d: create: alloc fail!",
            mongo_driver_name(driver), host, port);
        return NULL;
    }

    server->m_driver = driver;
    server->m_source = source;
    server->m_is_offline = 0;
    cpe_str_dup(server->m_host, sizeof(server->m_host), host);
    server->m_port = port;
    server->m_mode = mongo_server_runing_mode_unknown;
    server->m_active_connection_count = 0;
    server->m_connection_count = 0;
    
    TAILQ_INSERT_TAIL(&driver->m_servers, server, m_next);
    ++driver->m_server_count;

    TAILQ_INIT(&server->m_connections);

    mongo_server_reset_info(server);
    
    return server;
}

void mongo_server_free(struct mongo_server * server) {
    mongo_driver_t driver = server->m_driver;

    mongo_connection_free_all(server);

    assert(server->m_active_connection_count == 0);
    assert(server->m_connection_count == 0);

    TAILQ_REMOVE(&driver->m_servers, server, m_next);
    --driver->m_server_count;

    mem_free(driver->m_alloc, server);
}

void mongo_server_free_all(mongo_driver_t driver) {
    while(!TAILQ_EMPTY(&driver->m_servers)) {
        mongo_server_free(TAILQ_FIRST(&driver->m_servers));
    }
}

void mongo_server_reset_info(mongo_server_t server) {
    server->m_mode = mongo_server_runing_mode_unknown;
    server->m_min_wire_version = MONGO_DEFAULT_WIRE_VERSION;
    server->m_max_wire_version = MONGO_DEFAULT_WIRE_VERSION;
    server->m_max_msg_size = MONGO_DEFAULT_MAX_MSG_SIZE;
    server->m_max_bson_obj_size = MONGO_DEFAULT_BSON_OBJ_SIZE;
    server->m_max_write_batch_size = MONGO_DEFAULT_WRITE_BATCH_SIZE;
    server->m_set_name[0] = 0;
    server->m_set_version = MONGO_NO_SET_VERSION;
    server->m_round_trip_time = -1;
    server->m_is_match_me = 1;
    bson_oid_copy_unsafe(&mongo_driver_oid_zero, &server->m_election_id);
}

void mongo_server_update_rtt(mongo_server_t server, int64_t new_time) {
   if (server->m_round_trip_time == -1) {
       server->m_round_trip_time = new_time;
   }
   else {
       server->m_round_trip_time = 0.2 * new_time + (1 - 0.2) * server->m_round_trip_time;
   }
}

void mongo_server_offline(mongo_server_t server) {
    if (server->m_is_offline) return;
    
    server->m_is_offline = 1;
    mongo_driver_tick_once(server->m_driver);
}

const char * mongo_server_mode_to_str(enum mongo_server_runing_mode mode) {
    switch(mode) {
    case mongo_server_runing_mode_standalone:
        return "standalone";
    case mongo_server_runing_mode_mongos:
        return "mongos";
    case mongo_server_runing_mode_possible_primary:
        return "possible";
    case mongo_server_runing_mode_rs_primary:
        return "rs_primary";
    case mongo_server_runing_mode_rs_secondary:
        return "rs_secondary";
    case mongo_server_runing_mode_rs_arbiter:
        return "rs_arbiter";
    case mongo_server_runing_mode_rs_other:
        return "rs_other";
    case mongo_server_runing_mode_rs_ghost:
        return "rs_ghost";
    default:
        return "unknown";
    }
}

int mongo_server_update_info(mongo_server_t server, mongo_pkg_t pkg) {
    mongo_driver_t driver = server->m_driver;
    uint8_t is_master = 0;
    uint8_t is_shard = 0;
    uint8_t is_secondary = 0;
    uint8_t is_arbiter = 0;
    uint8_t is_replicaset = 0;
    uint8_t is_hidden = 0;
    uint32_t num_keys = 0;
    bson_iter_t it;

    mongo_server_reset_info(server);
    
    mongo_pkg_it(&it, pkg, 0);
    while (bson_iter_next(&it)) {
        num_keys++;
        if (strcmp ("ok", bson_iter_key (&it)) == 0) {
            if (!bson_iter_as_bool(&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: return not ok!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
        }
        else if (strcmp ("ismaster", bson_iter_key (&it)) == 0) {
            is_master = bson_iter_bool(&it);
        }
        else if (strcmp ("me", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_UTF8 (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: me format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            server->m_is_match_me = strcmp(server->m_address, bson_iter_utf8(&it, NULL)) == 0 ? 1 : 0;
            if (!server->m_is_match_me) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: match me fail, attress=%s, me=%s!",
                    mongo_driver_name(driver), server->m_host, server->m_port,
                    server->m_address, bson_iter_utf8(&it, NULL));
            }
        }
        else if (strcmp ("maxMessageSizeBytes", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_INT32(&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: maxMessageSizeBytes format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            server->m_max_msg_size = bson_iter_int32 (&it);
        }
        else if (strcmp ("maxBsonObjectSize", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_INT32(&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: maxBsonObjectSize format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            server->m_max_bson_obj_size = bson_iter_int32(&it);
        }
        else if (strcmp ("maxWriteBatchSize", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_INT32(&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: maxWriteBatchSize format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            server->m_max_write_batch_size = bson_iter_int32 (&it);
        }
        else if (strcmp ("minWireVersion", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_INT32 (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: minWireVersion format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            server->m_min_wire_version = bson_iter_int32 (&it);
        }
        else if (strcmp ("maxWireVersion", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_INT32 (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: maxWireVersion format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            server->m_max_wire_version = bson_iter_int32 (&it);
        }
        else if (strcmp ("msg", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_UTF8 (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: msg format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            is_shard = !!bson_iter_utf8 (&it, NULL);
        } else if (strcmp ("setName", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_UTF8 (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: setName format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            cpe_str_dup(server->m_set_name, sizeof(server->m_set_name), bson_iter_utf8 (&it, NULL));
        }
        else if (strcmp ("setVersion", bson_iter_key (&it)) == 0) {
            server->m_set_version = bson_iter_as_int64(&it);
        }
        else if (strcmp ("electionId", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_OID (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: electionId format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            bson_oid_copy_unsafe(bson_iter_oid (&it), &server->m_election_id);
        }
        else if (strcmp ("secondary", bson_iter_key (&it)) == 0) {
            is_secondary = bson_iter_bool (&it);
        }
        else if (strcmp ("hosts", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_ARRAY (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: hosts format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            /* bson_iter_array (&it, &len, &bytes); */
            /* bson_init_static (&server->m_hosts, bytes, len); */
        }
        else if (strcmp ("passives", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_ARRAY (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: passives format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            /* bson_iter_array (&it, &len, &bytes); */
            /* bson_init_static (&server->m_passives, bytes, len); */
        }
        else if (strcmp ("arbiters", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_ARRAY (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: arbiters format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            /* bson_iter_array (&it, &len, &bytes); */
            /* bson_init_static (&server->m_arbiters, bytes, len); */
        }
        else if (strcmp ("primary", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_UTF8 (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: primary format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            cpe_str_dup(server->m_current_primary, sizeof(server->m_current_primary), bson_iter_utf8(&it, NULL));
        }
        else if (strcmp ("arbiterOnly", bson_iter_key (&it)) == 0) {
            is_arbiter = bson_iter_bool (&it);
        }
        else if (strcmp ("isreplicaset", bson_iter_key (&it)) == 0) {
            is_replicaset = bson_iter_bool (&it);
        }
        else if (strcmp ("tags", bson_iter_key (&it)) == 0) {
            if (! BSON_ITER_HOLDS_DOCUMENT (&it)) {
                CPE_ERROR(
                    driver->m_em, "%s: server %s.%d: check-is-master: tags format error!",
                    mongo_driver_name(driver), server->m_host, server->m_port);
                return -1;
            }
            /* bson_iter_document (&it, &len, &bytes); */
            /* bson_init_static (&server->m_tags, bytes, len); */
        }
        else if (strcmp ("hidden", bson_iter_key (&it)) == 0) {
            is_hidden = bson_iter_bool (&it);
        }
    }

    if (is_shard) {
        server->m_mode = mongo_server_runing_mode_mongos;
    }
    else if (server->m_set_name[0]) {
        if (is_hidden) {
            server->m_mode = mongo_server_runing_mode_rs_other;
        }
        else if (is_master) {
            server->m_mode = mongo_server_runing_mode_rs_primary;
        }
        else if (is_secondary) {
            server->m_mode = mongo_server_runing_mode_rs_secondary;
        }
        else if (is_arbiter) {
            server->m_mode = mongo_server_runing_mode_rs_arbiter;
        }
        else {
            server->m_mode = mongo_server_runing_mode_rs_other;
        }
    }
    else if (is_replicaset) {
        server->m_mode = mongo_server_runing_mode_rs_ghost;
    }
    else if (num_keys > 0) {
        server->m_mode = mongo_server_runing_mode_standalone;
    }
    else {
        server->m_mode = mongo_server_runing_mode_unknown;
    }

    server->m_has_is_master = 1;

    mongo_driver_topology_update(driver, server);
    
    if (driver->m_debug) {
        CPE_INFO(
            driver->m_em, "%s: server %s %d: update info: mode=%s, topo=%s!",
            mongo_driver_name(driver), server->m_host, server->m_port,
            mongo_server_mode_to_str(server->m_mode), mongo_topology_type_to_str(driver->m_topology_type));
    }

    return 0;
}
