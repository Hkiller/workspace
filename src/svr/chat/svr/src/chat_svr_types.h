#ifndef SVR_CHAT_SVR_TYPES_H
#define SVR_CHAT_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/net/net_types.h"
#include "cpe/aom/aom_types.h"
#include "gd/timer/timer_types.h"
#include "protocol/svr/chat/svr_chat_internal.h"
#include "protocol/svr/chat/svr_chat_meta.h"

typedef struct chat_svr * chat_svr_t;
typedef struct chat_svr_chanel * chat_svr_chanel_t;

typedef TAILQ_HEAD(chat_svr_chanel_list, chat_svr_chanel) chat_svr_chanel_list_t;

struct chat_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    uint32_t m_check_once_process_count;
    gd_timer_id_t m_check_timer_id;

    uint32_t m_chanel_info_count;
    SVR_CHAT_CHANEL_INFO * m_chanel_infos;

    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;

    LPDRMETA m_meta_res_query;
    LPDRMETA m_meta_res_error;

    struct cpe_hash_table m_chanels;

    chat_svr_chanel_list_t m_chanel_check_queue;
};

struct chat_svr_chanel {
    chat_svr_t m_svr;
    SVR_CHAT_CHANEL_INFO const * m_chanel_info;
    uint64_t m_chanel_id;
    uint16_t m_chanel_type;
    uint32_t m_last_op_time_s;
    uint32_t m_chanel_sn;
    uint32_t m_chanel_msg_r;
    uint32_t m_chanel_msg_w;

    struct cpe_hash_entry m_hh;
    TAILQ_ENTRY(chat_svr_chanel) m_next_for_check;
};

#endif
