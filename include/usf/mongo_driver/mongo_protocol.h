#ifndef USF_MONGO_WRITE_PROTOCOL__H
#define USF_MONGO_WRITE_PROTOCOL__H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mongo_db_op {
    mongo_db_op_replay = 1,
    mongo_db_op_msg = 1000,
    mongo_db_op_update = 2001,
    mongo_db_op_insert = 2002,
    mongo_db_op_query = 2004,
    mongo_db_op_get_more = 2005,
    mongo_db_op_delete = 2006,
    mongo_db_op_kill_cursors = 2007
} mongo_db_op_t;

/*UPDATE*/
typedef struct mongo_pro_data_update {
    int32_t flags;
} mongo_pro_data_update_t;

typedef enum mongo_pro_flags_update {
    /*If set, the database will insert the supplied object into the collection if no matching document is found.*/
    mongo_pro_flags_update_upsert = 1 << 0
    /*MultiUpdate If set, the database will update all matching objects in the collection. Otherwise only updates first matching doc.*/
    , mongo_pro_flags_update_multi_update = 1 << 1
} mongo_pro_flags_update_t;

/*INSERT*/
typedef struct mongo_pro_data_insert {
    int32_t flags;
} mongo_pro_data_insert_t;

typedef enum mongo_pro_flags_insert {
    /*If set, the database will not stop processing a bulk insert if one fails (eg due to duplicate IDs).
      This makes bulk insert behave similarly to a series of single inserts,
      except lastError will be set if any insert fails, not just the last one. 
      If multiple errors occur, only the most recent will be reported by getLastError. (new in 1.9.1)
    */
    mongo_pro_flags_insert_continue_on_error = 1 << 0
} mongo_pro_flags_insert_t;

/*QUERY*/
typedef struct mongo_pro_data_query {
    int32_t flags;
    int32_t number_to_skip;
    int32_t number_to_return;
} mongo_pro_data_query_t;

typedef enum mongo_pro_flags_query {
    /*Tailable means cursor is not closed when the last data is retrieved. 
      Rather, the cursor marks the final object's position. 
      You can resume using the cursor later, from where it was located, if more data were received. 
      Like any "latent cursor", the cursor may become invalid at some point (CursorNotFound)
      – for example if the final object it references were deleted.*/
    mongo_pro_flags_query_tailable = 1 << 1
    /*SlaveOk	 Allow query of replica slave. Normally these return an error except for namespace "local".*/
    , mongo_pro_flags_query_slave_ok = 1 << 2
    /*OplogReplay	 Internal replication use only - driver should not set*/
    , mongo_pro_flags_query_op_log_replay = 1 << 3
    /*NoCursorTimeout	 The server normally times out idle cursors after an inactivity period (10 minutes) to prevent excess memory use. 
      Set this option to prevent that.*/
    , mongo_pro_flags_query_no_cursor_timeout = 1 << 4
    /*AwaitData	 Use with TailableCursor. If we are at the end of the data, block for a while rather than returning no data. After a timeout period, we do return as normal.*/
    , mongo_pro_flags_query_await_data = 1 << 5
    /*Exhaust	 Stream the data down full blast in multiple "more" packages, on the assumption that the client will fully read all data queried. 
      Faster when you are pulling a lot of data and know you want to pull it all down. 
      Note: the client is not allowed to not read all the data unless it closes the connection.*/
    , mongo_pro_flags_query_exhaust = 1 << 6
    /*Partial	 Get partial results from a mongos if some shards are down (instead of throwing an error)*/
    , mongo_pro_flags_query_partial = 1 << 7
} mongo_pro_flags_query_t;

/*GETMORE*/
typedef struct mongo_pro_data_get_more {
    int32_t number_to_return;
    int64_t cursor_id;
} mongo_pro_data_get_more_t;

/*DELETE*/
typedef struct mongo_pro_data_delete {
    int32_t flags;
} mongo_pro_data_delete_t;

typedef enum mongo_pro_flags_delete {
    /*If set, the database will remove only the first matching document in the collection. Otherwise all matching documents will be removed.*/
    mongo_pro_flags_delete_single = 1 << 0
} mongo_pro_flags_delete_t;

/*KILL_CURSORS*/
typedef struct mongo_pro_data_kill_cursors {
    int32_t cursor_count;
} mongo_pro_data_kill_cursors_t;

/*MSG*/

/*REPLY*/
typedef struct mongo_pro_data_reply {
    int32_t response_flag;   /* bit vector - see details below */
    int64_t cursor_id;       /* cursor id if client needs to do get more's */
    int32_t starting_from;   /* where in the cursor this reply is starting */
    int32_t number_retuned;  /* number of documents in the reply */
} mongo_pro_data_reply_t;

typedef enum mongo_pro_flags_reply {
     /*Set when getMore is called but the cursor id is not valid at the server. Returned with zero results.*/
    mongo_pro_flags_reply_cursor_not_found = 1 << 0
     /*Set when query failed. Results consist of one document containing an "$err" field describing the failure.*/
    , mongo_pro_flags_reply_query_fail = 1 << 1
     /*Drivers should ignore this. Only mongos will ever see this set, in which case, it needs to update config from the server.*/
    , mongo_pro_flags_reply_shared_config_state = 1 << 2
    /*Set when the server supports the AwaitData Query option. 
      If it doesn't, a client should sleep a little between getMore's of a Tailable cursor. 
      Mongod version 1.6 supports AwaitData and thus always sets AwaitCapable.*/
    , mongo_pro_flags_reply_await_capable = 1 << 3
} mongo_pro_flags_reply_t;


#ifdef __cplusplus
}
#endif

#endif
