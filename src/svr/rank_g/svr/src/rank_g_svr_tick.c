#include <assert.h>
#include "usf/logic/logic_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "rank_g_svr.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_season_info.h"
#include "rank_g_svr_ops.h"

ptr_int_t rank_g_svr_tick(void * ctx, ptr_int_t arg, float delta_s) {
    rank_g_svr_t svr = ctx;
    rank_g_svr_index_t index;
    uint32_t cur_time = rank_g_svr_cur_time(svr);
    rank_g_svr_index_t change_season_index = NULL;

    if (svr->m_saving_op) {
        logic_context_t saving_ctx = logic_context_find(set_logic_rsp_manage_logic(svr->m_rsp_manage), svr->m_saving_op);
        if (saving_ctx == NULL) {
            CPE_INFO(
                svr->m_em, "%s: tick: saving operation %d is done!",
                rank_g_svr_name(svr), svr->m_saving_op);
            svr->m_saving_op = 0;
        }

        rank_g_svr_load_init_records(svr);
    }

    /*错误重试保护 */
    if (svr->m_saving_error_time) {
        if (cur_time > svr->m_saving_error_time + 60) {
            svr->m_saving_error_time = 0;
        }
    }
    
    TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
        rank_g_svr_season_info_t next_season;

        if (!index->m_season_use) continue;

        /*切换赛季 */
        if (index->m_season_cur) {
            if (cur_time > index->m_season_cur->m_end_time) {
                next_season = TAILQ_NEXT(index->m_season_cur, m_next);
                if (next_season == NULL) {
                    CPE_INFO(
                        svr->m_em, "%s: tick: index %d: season %d expire, next season is %d!",
                        rank_g_svr_name(svr), index->m_id,
                        index->m_season_cur->m_season_id,
                        next_season->m_season_id);
                }
                else {
                    CPE_INFO(
                        svr->m_em, "%s: tick: index %d: season %d expire, no next season!",
                        rank_g_svr_name(svr), index->m_id,
                        index->m_season_cur->m_season_id);
                }

                index->m_season_cur = next_season;
            }
        }

        /*赛季需要保存 */
        if (change_season_index == NULL && svr->m_saving_op == 0 && svr->m_saving_error_time == 0) { /*没有保存操作正在进行 */
            if (index->m_record_season /*有赛季相关的记录 */
                && (index->m_season_cur == NULL || index->m_season_cur->m_season_id != index->m_record_season)) /*数据和当前赛季不匹配 */
            {
                change_season_index = index;
            }
        }
    }

    if (change_season_index) {
        if (mongo_driver_is_readable((mongo_cli_proxy_driver(svr->m_db)))) {
            /*启动数据保存流程 */
            logic_context_t context = rank_g_svr_op_change_season_start(svr, change_season_index);
            if (context) {
                svr->m_saving_op = logic_context_id(context);
            }
        }
    }
    
    return 0;
}

