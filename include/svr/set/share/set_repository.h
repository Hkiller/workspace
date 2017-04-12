#ifndef SVR_SET_CHANEL_SHM_H
#define SVR_SET_CHANEL_SHM_H
#include "cpe/utils/buffer.h"
#include "set_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_chanel_t set_chanel_shm_attach(int shmid, error_monitor_t em);
set_chanel_t set_chanel_shm_init(int shmid, uint32_t w_capacity, uint32_t r_capacity, error_monitor_t em);
void set_repository_chanel_detach(set_chanel_t chanel, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
