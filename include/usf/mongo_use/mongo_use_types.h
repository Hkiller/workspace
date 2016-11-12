#ifndef USF_MONGO_USE_SYSTEM_H
#define USF_MONGO_USE_SYSTEM_H
#include "usf/mongo_cli/mongo_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MONGO_ID_GENERATOR_INVALID_ID ((uint64_t)0)

typedef struct mongo_id_generator * mongo_id_generator_t;

#ifdef __cplusplus
}
#endif

#endif
