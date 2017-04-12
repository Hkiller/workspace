#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/cfg/cfg_read.h" 
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "chat_svr_ops.h"

extern char g_metalib_svr_chat_pro[];

static int chat_svr_meta_chanel_meta_cmp(void const * l, void const * r) {
    return ((int)((SVR_CHAT_CHANEL_INFO const *)l)->chanel_type)
        - ((int)((SVR_CHAT_CHANEL_INFO const *)r)->chanel_type);
}

int chat_svr_meta_chanel_load(chat_svr_t svr, cfg_t cfg) {
    struct cfg_it child_it;
    uint16_t meta_count = cfg_seq_count(cfg);
    uint16_t i;
    SVR_CHAT_CHANEL_INFO * metas;
    LPDRMETA record_meta;

    if (meta_count == 0) {
        if (svr->m_chanel_infos) mem_free(svr->m_alloc, svr->m_chanel_infos);
        svr->m_chanel_infos = NULL;
        svr->m_chanel_info_count = 0;
        return 0;
    }

    record_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_chat_pro, "svr_chat_chanel_info");
    if (record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: load chat meta: meta svr_chat_chanel_meta not exist!", chat_svr_name(svr));
        return -1;
    }

    metas = mem_alloc(svr->m_alloc, sizeof(SVR_CHAT_CHANEL_INFO) * meta_count);
    cfg_it_init(&child_it, cfg);
    for(i = 0; i < meta_count; ++i) {
        cfg_t record_cfg = cfg_it_next(&child_it);

        assert(record_cfg);

        if (dr_cfg_read(&metas[i], sizeof(metas[i]), record_cfg, record_meta, 0, svr->m_em) <= 0) {
            CPE_ERROR(svr->m_em, "%s: load chat meta: load svr_chat_chanel_meta fail!", chat_svr_name(svr));
            return -1;
        }
    }

    if (svr->m_chanel_infos) mem_free(svr->m_alloc, svr->m_chanel_infos);
    svr->m_chanel_infos = metas;
    svr->m_chanel_info_count = meta_count;

    qsort(metas, meta_count, sizeof(metas[0]), chat_svr_meta_chanel_meta_cmp);

    return 0;
}

SVR_CHAT_CHANEL_INFO const * chat_svr_meta_chanel_find(chat_svr_t svr, uint16_t chanel_type) {
    SVR_CHAT_CHANEL_INFO key;
    key.chanel_type = chanel_type;

    return bsearch(&key, svr->m_chanel_infos, svr->m_chanel_info_count, sizeof(key), chat_svr_meta_chanel_meta_cmp);
}
