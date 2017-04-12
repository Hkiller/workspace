#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "chat_svr_ops.h"

chat_svr_chanel_t
chat_svr_chanel_create(chat_svr_t svr, SVR_CHAT_CHANEL_INFO const * chanel_info, uint64_t chanel_id) {
    chat_svr_chanel_t chanel;
    size_t msg_buf_size = sizeof(SVR_CHAT_MSG) * chanel_info->msg_expire_count;

    chanel = mem_alloc(svr->m_alloc, sizeof(struct chat_svr_chanel) + msg_buf_size);
    if (chanel == NULL) {
        CPE_ERROR(svr->m_em, "%s: create chanel: malloc fail!", chat_svr_name(svr));
        return NULL;
    }

    chanel->m_svr = svr;
    chanel->m_chanel_info = chanel_info;
    chanel->m_chanel_id = chanel_id;
    chanel->m_last_op_time_s = 0;
    chanel->m_chanel_type = chanel_info->chanel_type;
    chanel->m_chanel_sn = 0;
    chanel->m_chanel_msg_r = 0;
    chanel->m_chanel_msg_w = 0;

    cpe_hash_entry_init(&chanel->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_chanels, chanel) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: create chanel: insert fail, "FMT_UINT64_T" already exist!",
            chat_svr_name(svr), chanel_id);
        mem_free(svr->m_alloc, chanel);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&svr->m_chanel_check_queue, chanel, m_next_for_check);

    return chanel;
}

void chat_svr_chanel_free(chat_svr_chanel_t chanel) {
    chat_svr_t svr = chanel->m_svr;
    assert(svr);

    /*remove from svr*/
    TAILQ_REMOVE(&svr->m_chanel_check_queue, chanel, m_next_for_check);
    cpe_hash_table_remove_by_ins(&svr->m_chanels, chanel);

    mem_free(svr->m_alloc, chanel);
}

void chat_svr_chanel_free_all(chat_svr_t svr) {
    struct cpe_hash_it chanel_it;
    chat_svr_chanel_t chanel;

    cpe_hash_it_init(&chanel_it, &svr->m_chanels);

    chanel = cpe_hash_it_next(&chanel_it);
    while(chanel) {
        chat_svr_chanel_t next = cpe_hash_it_next(&chanel_it);
        chat_svr_chanel_free(chanel);
        chanel = next;
    }
}

chat_svr_chanel_t
chat_svr_chanel_find(chat_svr_t svr, uint16_t chanel_type, uint64_t chanel_id) {
    struct chat_svr_chanel key;

    key.m_chanel_id = chanel_id;
    key.m_chanel_type = chanel_type;

    return cpe_hash_table_find(&svr->m_chanels, &key);
}

uint64_t chat_svr_chanel_count(chat_svr_t svr) {
    return cpe_hash_table_count(&svr->m_chanels);
}

SVR_CHAT_MSG * chat_svr_chanel_append_msg(chat_svr_chanel_t chanel) {
    SVR_CHAT_MSG * r = ((SVR_CHAT_MSG *)(chanel + 1)) + chanel->m_chanel_msg_w;

    r->sn = ++chanel->m_chanel_sn;

    chanel->m_chanel_msg_w++;
    if (chanel->m_chanel_msg_w >= chanel->m_chanel_info->msg_expire_count) {
        chanel->m_chanel_msg_w = 0;
    }

    if (chanel->m_chanel_msg_r == chanel->m_chanel_msg_w) {
        chanel->m_chanel_msg_r++;
        if (chanel->m_chanel_msg_r >= chanel->m_chanel_info->msg_expire_count) {
            chanel->m_chanel_msg_r = 0;
        }
    }

    return r;
}

SVR_CHAT_MSG * chat_svr_chanel_msg(chat_svr_chanel_t chanel, uint32_t pos) {
    if (chanel->m_chanel_msg_w >= chanel->m_chanel_msg_r) {
        assert(pos < (chanel->m_chanel_msg_w - chanel->m_chanel_msg_r));
        return ((SVR_CHAT_MSG *)(chanel + 1)) + chanel->m_chanel_msg_r + pos;
    }
    else {
        uint32_t left_count = chanel->m_chanel_info->msg_expire_count - chanel->m_chanel_msg_r;
        if (pos < left_count) {
            return ((SVR_CHAT_MSG *)(chanel + 1)) + chanel->m_chanel_msg_r + pos;
        }
        else {
            assert((pos - left_count) < chanel->m_chanel_msg_w);
            return ((SVR_CHAT_MSG *)(chanel + 1)) + (pos - left_count);
        }
    }
}

uint32_t chat_svr_chanel_msg_count(chat_svr_chanel_t chanel) {
    return chanel->m_chanel_msg_w >= chanel->m_chanel_msg_r
            ? (chanel->m_chanel_msg_w - chanel->m_chanel_msg_r)
            : (chanel->m_chanel_msg_w + chanel->m_chanel_info->msg_expire_count - chanel->m_chanel_msg_r);
}

uint32_t chat_svr_chanel_hash(chat_svr_chanel_t chanel) {
    return (((uint32_t)chanel->m_chanel_type) << 29
            | ((uint32_t)chanel->m_chanel_id));
}

int chat_svr_chanel_eq(chat_svr_chanel_t l, chat_svr_chanel_t r) {
    return l->m_chanel_type == r->m_chanel_type
        && l->m_chanel_id == r->m_chanel_id;
}

