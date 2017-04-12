#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "mongo_connection_i.h"
#include "mongo_pkg_i.h"

static mongo_server_t mongo_driver_find_primary(mongo_driver_t driver) {
    mongo_server_t server;

    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        if (server->m_mode == mongo_server_runing_mode_rs_primary || server->m_mode == mongo_server_runing_mode_standalone) {
            return server;
        }
    }
    
    return NULL;
}

static uint8_t mongo_driver_topology_later_election(mongo_driver_t driver, mongo_server_t server) {
    /* initially max_set_version is -1 and max_election_id is zeroed */
    return (driver->m_max_set_version > server->m_set_version
            ||
            (driver->m_max_set_version == server->m_set_version
             && bson_oid_compare (&driver->m_max_election_id, &server->m_election_id) > 0)
        ) ? 1 : 0;
}

static uint8_t
mongo_driver_topology_server_is_candidate(mongo_server_t server, mongo_read_mode_t read_mode, mongo_topology_type_t topology_type) {
    mongo_server_runing_mode_t desc_type = server->m_mode;

    if (server->m_is_offline) return 0;
    if (server->m_active_connection_count == 0) return 0;
    
    switch (topology_type) {
    case mongo_topology_type_single:
        switch (desc_type) {
        case mongo_server_runing_mode_standalone:
            return 1;
        default:
            return 0;
        }

    case mongo_topology_type_rs_no_primary:
    case mongo_topology_type_rs_with_primary:
        switch (read_mode) {
        case mongo_read_primary:
            switch (desc_type) {
            case mongo_server_runing_mode_possible_primary:
            case mongo_server_runing_mode_rs_primary:
                return 1;
            default:
                return 0;
            }
        case mongo_read_secondary:
            switch (desc_type) {
            case mongo_server_runing_mode_rs_secondary:
                return 1;
            default:
                return 0;
            }
        default:
            switch (desc_type) {
            case mongo_server_runing_mode_possible_primary:
            case mongo_server_runing_mode_rs_primary:
            case mongo_server_runing_mode_rs_secondary:
                return 1;
            default:
                return 0;
            }
        }

    case mongo_topology_type_sharded:
        switch (desc_type) {
        case mongo_server_runing_mode_mongos:
            return 1;
        default:
            return 0;
        }
    default:
        return 0;
    }
}

void mongo_driver_topology_suitable_servers(
    mongo_server_t * servers, uint32_t * server_count,
    mongo_driver_t driver, uint8_t for_writes, mongo_read_mode_t read_mode)
{
    //uint32_t servers_capacity = *server_count;
    mongo_server_t server;
    int64_t nearest = -1;
    uint32_t i;

    *server_count = 0;
    
    /* Single server --
     * Either it is suitable or it isn't */
    if (driver->m_topology_type == mongo_topology_type_single) {
        server = TAILQ_FIRST(&driver->m_servers);
        if (mongo_driver_topology_server_is_candidate(server, read_mode, driver->m_topology_type)) {
            servers[(*server_count)++] = server;
        }
        return;
    }

    /* Replica sets --
     * Find suitable servers based on read mode */
    if (driver->m_topology_type == mongo_topology_type_rs_no_primary || driver->m_topology_type == mongo_topology_type_rs_with_primary) {
        if (!for_writes) {
            mongo_server_t primary = NULL;
            uint8_t has_secondary = 0;

            TAILQ_FOREACH(server, &driver->m_servers, m_next) {
                /* primary's used in staleness calculation, even with mode SECONDARY */
                if (server->m_mode == mongo_server_runing_mode_rs_primary) {
                    primary = server;
                }

                if (mongo_driver_topology_server_is_candidate(server, read_mode, driver->m_topology_type)) {
                    if (server->m_mode == mongo_server_runing_mode_rs_primary) {
                        if (read_mode == mongo_read_primary || read_mode == mongo_read_primary_preferred) {
                            /* we want a primary and we have one, done! */
                            break;
                        }
                    }

                    if (server->m_mode == mongo_server_runing_mode_rs_secondary) {
                        has_secondary = 1;
                    }

                    /* add to our candidates */
                    servers[(*server_count)++] = server;
                }
            }

            if (read_mode == mongo_read_primary) {
                (*server_count) = 0;
                if (primary) servers[(*server_count)++] = primary;
                return;
            }

            if (read_mode == mongo_read_primary_preferred && primary) {
                (*server_count) = 0;
                servers[(*server_count)++] = primary;
                return;
            }

            if (read_mode == mongo_read_secondary_preferred) {
                if (primary && has_secondary) {
                    for(i = 0; i < *server_count; ++i) {
                        if (servers[i] == primary) {
                            memmove(servers + i, servers + i + 1, sizeof(servers[0]) * (*server_count - i - 1));
                            (*server_count) --;
                            break;;
                        }
                    }
                }
                return;
            }

            if (read_mode == mongo_read_secondary) {
                for(i = 0; i < *server_count;) {
                    if (servers[i]->m_mode != mongo_server_runing_mode_rs_secondary) {
                        memmove(servers + i, servers + i + 1, sizeof(servers[0]) * (*server_count - i - 1));
                        (*server_count) --;
                    }
                    else {
                        ++i;
                    }
                }
            }

            /* mode is SECONDARY or NEAREST, filter by staleness and tags */
            /* mongoc_server_description_filter_stale (data.candidates, */
            /*                                         data.candidates_len, */
            /*                                         data.primary, */
            /*                                         heartbeat_frequency_ms, */
            /*                                         read_pref); */

            /* mongoc_server_description_filter_tags (data.candidates, */
            /*                                        data.candidates_len, */
            /*                                        read_pref); */
        }
        else {
            if (driver->m_topology_type == mongo_topology_type_rs_with_primary) {
                mongo_server_t primary = NULL;
            
                TAILQ_FOREACH(server, &driver->m_servers, m_next) {
                    /* primary's used in staleness calculation, even with mode SECONDARY */
                    if (server->m_mode == mongo_server_runing_mode_rs_primary) {
                        primary = server;
                        break;
                    }
                }

                if (primary) {
                    servers[(*server_count)++] = primary;
                }
            }
        }
    }

    /* Sharded clusters --
     * All candidates in the latency window are suitable */
    if (driver->m_topology_type == mongo_topology_type_sharded) {
        TAILQ_FOREACH(server, &driver->m_servers, m_next) {
            if (mongo_driver_topology_server_is_candidate(server, read_mode, driver->m_topology_type)) {
                servers[(*server_count)++] = server;
            }
        }
    }

    /* Ways to get here:
     *   - secondary read
     *   - secondary preferred read
     *   - primary_preferred and no primary read
     *   - sharded anything
     * Find the nearest, then select within the window */
    for (i = 0; i < *server_count; i++) {
        if (nearest == -1 || nearest > servers[i]->m_round_trip_time) {
            nearest = servers[i]->m_round_trip_time;
        }
    }

    if (nearest < 0) {
        *server_count = 0;
    }
    else {
        for (i = 0; i < *server_count;) {
            if (servers[i]->m_round_trip_time <= nearest + driver->m_select_threshold_ms) {
                ++i;
            }
            else {
                memmove(servers + i, servers + i + 1, sizeof(servers[0]) * (*server_count - i - 1));
                (*server_count) --;
            }
        }
    }
}

static void mongo_driver_topology_remove_server(mongo_driver_t driver, mongo_server_t server) {
    mongo_server_offline(server);
}

static void mongo_driver_topology_label_unknown_member(mongo_driver_t driver, const char *address, mongo_server_runing_mode_t mode) {
    mongo_server_t server;
    
    TAILQ_FOREACH(server, &driver->m_servers, m_next) {
        if (strcasecmp(server->m_address, address) == 0 && server->m_mode == mongo_server_runing_mode_unknown) {
            server->m_mode = mode;
            break;
        }
    }
}

static void mongo_driver_topology_update_rs_type(mongo_driver_t driver) {
    if (mongo_driver_find_primary(driver)) {
       driver->m_topology_type = mongo_topology_type_rs_with_primary;
   }
   else {
       driver->m_topology_type = mongo_topology_type_rs_no_primary;
   }
}

static void mongo_driver_topology_check_if_has_primary(mongo_driver_t driver, mongo_server_t server) {
    mongo_driver_topology_update_rs_type(driver);
}

void mongo_driver_topology_invalidate_server(mongo_driver_t driver, mongo_server_t server) {
    //TODO: Loki
    //mongoc_topology_description_handle_ismaster (topology, sd, NULL, 0, NULL);
}

/* Remove and destroy all replica set members not in primary's hosts lists */
static void mongo_driver_topology_remove_unreported_servers(mongo_driver_t driver, mongo_server_t primary) {
   /* mongoc_array_t to_remove; */
   /* int i; */
   /* mongoc_server_description_t *member; */
   /* const char *address; */

   /* _mongoc_array_init (&to_remove, */
   /*                     sizeof (mongoc_server_description_t *)); */

   /* /\* Accumulate servers to be removed - do this before calling */
   /*  * mongo_driver_topology_remove_server, which could call */
   /*  * mongoc_server_description_cleanup on the primary itself if it */
   /*  * doesn't report its own connection_address in its hosts list. */
   /*  * See hosts_differ_from_seeds.json *\/ */
   /* for (i = 0; i < topology->servers->items_len; i++) { */
   /*    member = (mongoc_server_description_t *)mongoc_set_get_item (topology->servers, i); */
   /*    address = member->connection_address; */
   /*    if (!mongoc_server_description_has_rs_member(primary, address)) { */
   /*       _mongoc_array_append_val (&to_remove, member); */
   /*    } */
   /* } */

   /* /\* now it's safe to call mongo_driver_topology_remove_server, */
   /*  * even on the primary *\/ */
   /* for (i = 0; i < to_remove.len; i++) { */
   /*    member = _mongoc_array_index ( */
   /*       &to_remove, mongoc_server_description_t *, i); */

   /*    mongo_driver_topology_remove_server (topology, member); */
   /* } */

   /* _mongoc_array_destroy (&to_remove); */
}

static void mongo_driver_topology_update_rs_from_primary(mongo_driver_t driver, mongo_server_t server) {
    mongo_server_t check_server;
    
    /* If server->set_name was null this function wouldn't be called from
     * mongoc_server_description_handle_ismaster(). static code analyzers however
     * don't know that so we check for it explicitly. */
    if (server->m_set_name[0]) {
        /* 'Server' can only be the primary if it has the right rs name  */

        if (driver->m_replica_set[0] == 0) {
            cpe_str_dup(driver->m_replica_set, sizeof(driver->m_replica_set), server->m_set_name);
        }
        else if (strcmp(driver->m_replica_set, server->m_set_name) != 0) {
            mongo_driver_topology_remove_server(driver, server);
            mongo_driver_topology_update_rs_type(driver);
            return;
        }
    }

    if (server->m_set_version != MONGO_NO_SET_VERSION
        && bson_oid_compare(&server->m_election_id, &mongo_driver_oid_zero) != 0)
    {
        /* Server Discovery And Monitoring Spec: "The client remembers the
         * greatest electionId reported by a primary, and distrusts primaries
         * with lesser electionIds. This prevents the client from oscillating
         * between the old and new primary during a split-brain period."
         */
        if (mongo_driver_topology_later_election(driver, server)) {
            CPE_ERROR(driver->m_em, "member's setVersion or electionId is stale");
            mongo_driver_topology_invalidate_server(driver, server);
            mongo_driver_topology_update_rs_type(driver);
            return;
        }

        /* server's electionId >= topology's max electionId */
        bson_oid_copy(&server->m_election_id, &driver->m_max_election_id);
    }

    if (server->m_set_version != MONGO_NO_SET_VERSION) {
        if ((driver->m_max_set_version == MONGO_NO_SET_VERSION || server->m_set_version > driver->m_max_set_version)) {
            driver->m_max_set_version = server->m_set_version;
        }
    }

    /* 'Server' is the primary! Invalidate other primaries if found */
    TAILQ_FOREACH(check_server, &driver->m_servers, m_next) {
        if (check_server != server && check_server->m_mode == mongo_server_runing_mode_rs_primary) {
            mongo_connection_free_all(check_server);
            mongo_server_reset_info(check_server);
        }
    }

    /* Add to topology description any new servers primary knows about */
    /* mongo_driver_topology_add_new_servers(driver, server); */

    /* Remove from topology description any servers primary doesn't know about */
    mongo_driver_topology_remove_unreported_servers(driver, server);

    /* Finally, set topology type */
    mongo_driver_topology_update_rs_type(driver);
}

static void mongo_driver_topology_update_rs_without_primary(mongo_driver_t driver, mongo_server_t server) {
    /* make sure we're talking about the same replica set */
    if (server->m_set_name[0]) {
        if (driver->m_replica_set[0]) {
            cpe_str_dup(driver->m_replica_set, sizeof(driver->m_replica_set), server->m_set_name);
        }
        else if (strcmp(driver->m_replica_set, server->m_set_name)!= 0) {
            mongo_driver_topology_remove_server(driver, server);
            return;
        }
    }

   /* Add new servers that this replica set member knows about */
   /* mongo_driver_topology_add_new_servers(driver, server); */
   if (!server->m_is_match_me) {
       mongo_driver_topology_remove_server(driver, server);
       return;
   }

   /* If this server thinks there is a primary, label it POSSIBLE_PRIMARY */
   if (server->m_current_primary[0]) {
       mongo_driver_topology_label_unknown_member(driver, server->m_current_primary, mongo_server_runing_mode_possible_primary);
   }
}

static void
mongo_driver_topology_update_rs_with_primary_from_member(mongo_driver_t driver, mongo_server_t server) {
   /* set_name should never be null here */
   if (strcmp(driver->m_replica_set, server->m_set_name) != 0) {
       mongo_driver_topology_remove_server(driver, server);
       mongo_driver_topology_update_rs_type(driver);
       return;
   }

   if (!server->m_is_match_me) {
       mongo_driver_topology_remove_server(driver, server);
       return;
   }

   /* If there is no primary, label server's current_primary as the POSSIBLE_PRIMARY */
   if (mongo_driver_find_primary(driver) == NULL && server->m_current_primary[0]) {
       driver->m_topology_type = mongo_topology_type_rs_no_primary;
       mongo_driver_topology_label_unknown_member(
           driver, server->m_current_primary, mongo_server_runing_mode_possible_primary);
   }
}

static void mongo_driver_topology_set_topology_type_to_sharded(mongo_driver_t driver, mongo_server_t server) {
    driver->m_topology_type = mongo_topology_type_sharded;
}

static void mongo_driver_topology_transition_unknown_to_rs_no_primary(mongo_driver_t driver, mongo_server_t server) {
    driver->m_topology_type = mongo_topology_type_rs_no_primary;
    mongo_driver_topology_update_rs_without_primary(driver, server);
}

static void mongo_driver_topology_remove_and_check_primary(mongo_driver_t driver, mongo_server_t server) {
    mongo_driver_topology_remove_server(driver, server);
    mongo_driver_topology_update_rs_type(driver);
}

static void mongo_driver_topology_update_unknown_with_standalone(mongo_driver_t driver, mongo_server_t server) {
    if (driver->m_server_count > 1) {
        /* This cluster contains other servers, it cannot be a standalone. */
        mongo_driver_topology_remove_server(driver, server);
    }
    else {
        driver->m_topology_type = mongo_topology_type_single;
    }
}

typedef void (*mongo_driver_topology_transition_t)(mongo_driver_t driver, mongo_server_t server);

static mongo_driver_topology_transition_t
s_topology_transes[mongo_server_runing_mode_count][mongo_topology_type_count] = {
   { /* unknown */
      NULL, /* mongo_topology_unknown */
      NULL, /* mongo_topology_sharded */
      NULL, /* mongo_topology_rs_no_primary */
      mongo_driver_topology_check_if_has_primary /* mongo_topology_rs_with_primary */
   },
   { /* standalone */
      mongo_driver_topology_update_unknown_with_standalone,
      mongo_driver_topology_remove_server,
      mongo_driver_topology_remove_server,
      mongo_driver_topology_remove_and_check_primary
   },
   { /* mongos */
      mongo_driver_topology_set_topology_type_to_sharded,
      NULL,
      mongo_driver_topology_remove_server,
      mongo_driver_topology_remove_and_check_primary
   },
   { /* possible_primary */
      NULL,
      NULL,
      NULL,
      NULL
   },
   { /* primary */
      mongo_driver_topology_update_rs_from_primary,
      mongo_driver_topology_remove_server,
      mongo_driver_topology_update_rs_from_primary,
      mongo_driver_topology_update_rs_from_primary
   },
   { /* secondary */
      mongo_driver_topology_transition_unknown_to_rs_no_primary,
      mongo_driver_topology_remove_server,
      mongo_driver_topology_update_rs_without_primary,
      mongo_driver_topology_update_rs_with_primary_from_member
   },
   { /* arbiter */
      mongo_driver_topology_transition_unknown_to_rs_no_primary,
      mongo_driver_topology_remove_server,
      mongo_driver_topology_update_rs_without_primary,
      mongo_driver_topology_update_rs_with_primary_from_member
   },
   { /* rs_other */
      mongo_driver_topology_transition_unknown_to_rs_no_primary,
      mongo_driver_topology_remove_server,
      mongo_driver_topology_update_rs_without_primary,
      mongo_driver_topology_update_rs_with_primary_from_member
   },
   { /* rs_ghost */
      NULL,
      mongo_driver_topology_remove_server,
      NULL,
      mongo_driver_topology_check_if_has_primary
   }
};

void mongo_driver_topology_update(mongo_driver_t driver, mongo_server_t server) {
   if (s_topology_transes[server->m_mode][driver->m_topology_type]) {
       mongo_topology_type_t old_type = driver->m_topology_type;
       
       if (driver->m_debug) {
           CPE_INFO(
               driver->m_em, "%s: topology update: transitioning to %s for %s",
               mongo_driver_name(driver), mongo_topology_type_to_str(driver->m_topology_type), mongo_server_mode_to_str(server->m_mode));
       }
       
       s_topology_transes[server->m_mode][driver->m_topology_type](driver, server);

       if (old_type != driver->m_topology_type && driver->m_debug) {
           CPE_INFO(
               driver->m_em, "%s: topology update: topology type %s ==> %s",
               mongo_driver_name(driver),
               mongo_topology_type_to_str(old_type),
               mongo_topology_type_to_str(driver->m_topology_type));
       }
   }
   else {
       if (driver->m_debug) {
           CPE_INFO(
               driver->m_em, "%s: topology update: no transitioning to %s for %s",
               mongo_driver_name(driver), mongo_topology_type_to_str(driver->m_topology_type), mongo_server_mode_to_str(server->m_mode));
       }
   }
}

const char * mongo_topology_type_to_str(mongo_topology_type_t t) {
    switch(t) {
    case mongo_topology_type_sharded:
        return "sharded";
    case mongo_topology_type_rs_no_primary:
        return "rs_no_primary";
    case mongo_topology_type_rs_with_primary:
        return "rs_with_primary";
    case mongo_topology_type_single:
        return "single";
    default:
        return "unknown";
    }
}
