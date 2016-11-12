#ifndef SVR_SET_SHARE_UTILS_H
#define SVR_SET_SHARE_UTILS_H
#include "set_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int set_shm_key_get(uint16_t svr_type, uint16_t svr_id, char tag);
    
#ifdef __cplusplus
}
#endif

#endif
