#ifndef USF_MONGO_USE_INTERNAL_OPS_H
#define USF_MONGO_USE_INTERNAL_OPS_H
#include "mongo_use_internal_types.h"

uint32_t mongo_id_info_hash(const struct mongo_id_info * id_info);
int mongo_id_info_eq(const struct mongo_id_info * l, const struct mongo_id_info * r);
void mongo_id_info_free_all(mongo_id_generator_t generator);
struct mongo_id_info * mongo_id_info_find(mongo_id_generator_t generator, const char * id_name);

int mongo_id_info_have_waiting_require(mongo_id_generator_t generator, struct mongo_id_info * id_info);

#endif
