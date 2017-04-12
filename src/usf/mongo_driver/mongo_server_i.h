#ifndef USF_MONGO_DRIVER_SERVER_I_H
#define USF_MONGO_DRIVER_SERVER_I_H
#include "mongo_driver_i.h"

#define MONGO_DEFAULT_WIRE_VERSION 0
#define MONGO_DEFAULT_WRITE_BATCH_SIZE 1000
#define MONGO_DEFAULT_BSON_OBJ_SIZE 16 * 1024 * 1024
#define MONGO_DEFAULT_MAX_MSG_SIZE 48000000

#define WIRE_VERSION_SCRAM_DEFAULT 3

typedef enum mongo_server_source {
    mongo_server_source_uri,
    mongo_server_source_found
} mongo_server_source_t;

typedef enum mongo_server_runing_mode {
    mongo_server_runing_mode_unknown,
    mongo_server_runing_mode_standalone,
    mongo_server_runing_mode_mongos,
    mongo_server_runing_mode_possible_primary,
    mongo_server_runing_mode_rs_primary,
    mongo_server_runing_mode_rs_secondary,
    mongo_server_runing_mode_rs_arbiter,
    mongo_server_runing_mode_rs_other,
    mongo_server_runing_mode_rs_ghost,
    mongo_server_runing_mode_count,
} mongo_server_runing_mode_t;

struct mongo_server {
    mongo_driver_t m_driver;
    char m_host[64];
    int m_port;
    mongo_server_source_t m_source;
    enum mongo_server_runing_mode m_mode;
    uint8_t m_is_offline;
    uint16_t m_connection_count;
    uint16_t m_active_connection_count;
    mongo_connection_list_t m_connections;
    
    uint8_t m_has_is_master;
    
    char m_address[32];
    int32_t m_min_wire_version;
    int32_t m_max_wire_version;
    int32_t m_max_msg_size;
    int32_t m_max_bson_obj_size;
    int32_t m_max_write_batch_size;
    int64_t m_round_trip_time;
    uint8_t m_is_match_me;
    
    /*set infos*/
    bson_oid_t m_election_id;
    int64_t m_set_version;
    char m_set_name[64];
    char m_current_primary[64];

    TAILQ_ENTRY(mongo_server) m_next;
};

/*server ops*/
mongo_server_t mongo_server_create(mongo_driver_t driver, const char * host, int port, mongo_server_source_t source);
void mongo_server_free(struct mongo_server * server);
void mongo_server_free_all(mongo_driver_t driver);

void mongo_server_reset_info(mongo_server_t server);
int mongo_server_update_info(mongo_server_t server, mongo_pkg_t pkg);
void mongo_server_update_rtt(mongo_server_t server, int64_t new_time);
void mongo_server_offline(mongo_server_t server);

const char * mongo_server_mode_to_str(enum mongo_server_runing_mode mode);

#endif
