#ifndef GD_DR_STORE_TYPES_H
#define GD_DR_STORE_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dr_store_manage * dr_store_manage_t;
typedef struct dr_store * dr_store_t;
typedef struct dr_ref * dr_ref_t;

typedef void (*dr_lib_free_fun_t)(LPDRMETALIB lib, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
