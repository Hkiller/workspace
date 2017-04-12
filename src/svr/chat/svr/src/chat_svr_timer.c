#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "chat_svr_ops.h"

void chat_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg) {
    chat_svr_t svr = ctx;
    int process_count;
    uint32_t max_process_count = svr->m_check_once_process_count;
    uint32_t cur_time_s = chat_svr_cur_time(svr);

    if (max_process_count > chat_svr_chanel_count(svr)) {
        max_process_count = chat_svr_chanel_count(svr);
    }

    for(process_count = 0; process_count < max_process_count; ++process_count) {
        uint32_t msg_remove_count = 0;
        chat_svr_chanel_t chanel;
        SVR_CHAT_MSG * msgs;

        chanel = TAILQ_FIRST(&svr->m_chanel_check_queue);
        assert(chanel);

        TAILQ_REMOVE(&svr->m_chanel_check_queue, chanel, m_next_for_check);
        TAILQ_INSERT_TAIL(&svr->m_chanel_check_queue, chanel, m_next_for_check);

        if (chanel->m_chanel_info->chanel_expire_time_s > 0) {
            if (chanel->m_last_op_time_s + chanel->m_chanel_info->chanel_expire_time_s < cur_time_s) {
                CPE_INFO(
                    svr->m_em, "%s: chanel %d-"FMT_UINT64_T": on_timer: chanel expire, free!",
                    chat_svr_name(svr), chanel->m_chanel_type, chanel->m_chanel_id);
                chat_svr_chanel_free(chanel);
                continue;
            }
        }

        /*消息超时 */
        msgs = ((SVR_CHAT_MSG *)(chanel + 1));

        while(chanel->m_chanel_msg_r != chanel->m_chanel_msg_w) {
            SVR_CHAT_MSG const * msg = msgs + chanel->m_chanel_msg_r;

            if (msg->send_time + chanel->m_chanel_info->msg_expire_time_s >= cur_time_s) break;

            ++msg_remove_count;
            ++chanel->m_chanel_msg_r;
            if (chanel->m_chanel_msg_r >= chanel->m_chanel_info->msg_expire_count) {
                chanel->m_chanel_msg_r = 0;
            }
        }

        if (svr->m_debug) {
            if (msg_remove_count > 0) {
                CPE_INFO(
                    svr->m_em, "%s: chanel %d-"FMT_UINT64_T": on_timer: %d message(s) expire!",
                    chat_svr_name(svr), chanel->m_chanel_type, chanel->m_chanel_id, msg_remove_count);
            }
        }
    }

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: on_timer: processed %d chanels!", chat_svr_name(svr), process_count);
    }
}
