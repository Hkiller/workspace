#ifndef GD_DR_DM_ROLE_TYPES_H
#define GD_DR_DM_ROLE_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_types.h"
#include "gd/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef gd_id_t dr_dm_data_id_t;
typedef struct dr_dm_data * dr_dm_data_t;
typedef struct dr_dm_manage * dr_dm_manage_t;

#define DR_DM_DATA_ID_INVALID ((dr_dm_data_id_t)0)

struct dr_dm_data_it {
    dr_dm_data_t (*next)(struct dr_dm_data_it * it);
    char m_data[16];
};

#ifdef __cplusplus
}
#endif

#endif
