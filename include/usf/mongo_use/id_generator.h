#ifndef USF_MONGO_USE_ID_GENERATOR_H
#define USF_MONGO_USE_ID_GENERATOR_H
#include "mongo_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_id_generator_t
mongo_id_generator_create(
    gd_app_context_t app,
    const char * name,
    mongo_cli_proxy_t mongo_cli,
    mem_allocrator_t alloc,
    error_monitor_t em);

void mongo_id_generator_free(mongo_id_generator_t generator);

mongo_id_generator_t
mongo_id_generator_find(gd_app_context_t app, cpe_hash_string_t name);

mongo_id_generator_t
mongo_id_generator_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t mongo_id_generator_app(mongo_id_generator_t generator);
const char * mongo_id_generator_name(mongo_id_generator_t generator);
cpe_hash_string_t mongo_id_generator_name_hs(mongo_id_generator_t generator);

int mongo_id_generator_regist_id(mongo_id_generator_t generator, const char * id_name, uint32_t id_start, uint32_t id_inc);

#ifdef __cplusplus
}
#endif

#endif
