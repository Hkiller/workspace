#ifndef USF_MONGO_DRIVER_H
#define USF_MONGO_DRIVER_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "mongo_driver_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_driver_t
mongo_driver_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void mongo_driver_free(mongo_driver_t driver);
int mongo_driver_build_reulst_metalib(mongo_driver_t driver);

mongo_driver_t
mongo_driver_find(gd_app_context_t app, cpe_hash_string_t name);

mongo_driver_t
mongo_driver_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t mongo_driver_app(mongo_driver_t driver);
const char * mongo_driver_name(mongo_driver_t driver);
cpe_hash_string_t mongo_driver_name_hs(mongo_driver_t driver);

int mongo_driver_set_uri(mongo_driver_t driver, const char * uri);
int mongo_driver_set_incoming_send_to(mongo_driver_t driver, const char * incoming_send_to);
int mongo_driver_set_outgoing_recv_at(mongo_driver_t driver, const char * outgoing_recv_at);
int mongo_driver_set_ringbuf_size(mongo_driver_t driver, size_t capacity);

uint8_t mongo_driver_is_enable(mongo_driver_t driver);
int mongo_driver_enable(mongo_driver_t driver);
int mongo_driver_disable(mongo_driver_t driver);
    
int mongo_driver_send(mongo_driver_t driver, mongo_pkg_t pkg);
mongo_pkg_t mongo_driver_pkg_buf(mongo_driver_t driver);

uint8_t mongo_driver_is_readable(mongo_driver_t driver);
uint8_t mongo_driver_is_readable_ex(mongo_driver_t driver, mongo_read_mode_t read_mode);
uint8_t mongo_driver_is_writable(mongo_driver_t driver);
    
mongo_topology_type_t mongo_driver_topology_type(mongo_driver_t driver);

const char * mongo_topology_type_to_str(mongo_topology_type_t t);

#ifdef __cplusplus
}
#endif

#endif
