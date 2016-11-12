#ifndef GD_UTILS_TYPES_H
#define GD_UTILS_TYPES_H
#include "cpe/pal/pal_types.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t gd_id_t;
typedef struct gd_id_generator {
    uint32_t magic;
    gd_app_context_t app;
    int (*gen_fun)(gd_id_t * id, struct gd_id_generator * g, const char * id_name);
} * gd_id_generator_t;

typedef struct gd_id_file_generator * gd_id_file_generator_t;

#define GD_ID_GENERATOR_MAGIC ((uint32_t)0xdF34DF34)

#define gd_id_hash(value) ((uint32_t)(((value >> 32) & 0xFFFF) | (value & 0xFFFFFFFF)))

#ifdef __cplusplus
}
#endif

#endif


