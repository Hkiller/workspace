#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_data.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "cpe/aom/aom_obj_hash.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_context.h"
#include "rank_g_svr.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_season_info.h"

int rank_g_svr_load_init_records(rank_g_svr_t svr) {
    struct cfg_it record_it;
    cfg_t record_cfg;
    int rv;
    ptr_int_t record_idx;
    rank_g_svr_index_t index;
    void * record;

    cfg_it_init(&record_it, cfg_find_cfg(gd_app_cfg(svr->m_app), "init"));

    while((record_cfg = cfg_it_next(&record_it))) {
        void * buf = rank_g_svr_record_buf(svr, svr->m_record_size);
        assert(buf);

        bzero(buf, svr->m_record_size);

        rv = dr_cfg_read(buf, svr->m_record_size, record_cfg, svr->m_record_meta, 0, svr->m_em);
        if (rv < 0) {
            CPE_ERROR(svr->m_em, "%s: load init records: read data fail!", rank_g_svr_name(svr));
            return -1;
        }

        /*带有season的，需要将记录的season设置为当前的season */
        TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
            if (!index->m_season_use) continue;

            if (dr_entry_set_from_uint16(
                    ((char *)buf) + index->m_season_entry_start_pos,
                    (index->m_season_cur ? index->m_season_cur->m_season_id : 0),
                    index->m_season_entry,
                    svr->m_em)
                != 0)
            {
                CPE_ERROR(
                    svr->m_em, "%s: load init record: index %d reset season fail !",
                    rank_g_svr_name(svr), index->m_id);
                return -1;
            }
        }

        if (aom_obj_hash_table_insert_or_update(svr->m_record_hash, buf, &record_idx) != 0) {
            CPE_ERROR(svr->m_em, "%s: load init data: create_or_update record fail!", rank_g_svr_name(svr));
            return -1;
        }

        record = aom_obj_get(svr->m_record_mgr, record_idx);

        TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
            rv = rank_g_svr_index_update(index, record, (uint32_t)record_idx);
            if (rv != 0) {
                CPE_ERROR(svr->m_em, "%s: load init data: update record %d record fail!", rank_g_svr_name(svr), (int)record_idx);
                return -1;
            }
        }

        if (svr->m_debug) {
            CPE_INFO(svr->m_em, "        load init data: update record %s", rank_g_svr_record_dump(svr, record));
        }
    }
    
    return 0;
}
