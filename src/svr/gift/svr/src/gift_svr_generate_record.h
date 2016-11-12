#ifndef SVR_GIFT_SVR_GENERATE_RECORD_H
#define SVR_GIFT_SVR_GENERATE_RECORD_H
#include "gift_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * gift_svr_record_dump(gift_svr_t svr, void const * record);
void * gift_svr_record_find(gift_svr_t svr, uint32_t generate_id);

int gift_svr_generate_record_init(gift_svr_t svr, LPDRMETA data_meta, uint32_t generate_record_count, float bucket_ratio);

#ifdef __cplusplus
}
#endif

#endif
