#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_context.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "gift_svr_generate_record.h"

const char * gift_svr_record_dump(gift_svr_t svr, void const * record) {
    return dr_json_dump_inline(
        gd_app_tmp_buffer(svr->m_app), record, svr->m_generate_record_size, svr->m_generate_record_meta);
}

void * gift_svr_record_find(gift_svr_t svr, uint32_t generate_id) {
    SVR_GIFT_GENERATE_RECORD key;
    key._id = generate_id;
    return aom_obj_hash_table_find(svr->m_generate_record_hash, &key);
}
