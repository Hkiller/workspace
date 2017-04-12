#include "render/cache/ui_cache_res.h"
#include "ui_data_src_res_i.h"

ui_data_src_res_t
ui_data_src_res_create(ui_data_src_t user, ui_cache_res_t be_using) {
    ui_data_mgr_t mgr = user->m_mgr;
    ui_data_src_res_t src_res;

    TAILQ_FOREACH(src_res, &user->m_using_ress, m_next_for_src) {
        if (src_res->m_using_res == be_using) return src_res;
    }
    
    src_res = TAILQ_FIRST(&mgr->m_free_src_ress);
    if (src_res) {
        TAILQ_REMOVE(&mgr->m_free_src_ress, src_res, m_next_for_src);
    }
    else {
        src_res = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_src_res));
        if (src_res == NULL) {
            CPE_ERROR(mgr->m_em, "ui_data_src_res_create: alloc fail!");
            return NULL;
        }
    }

    src_res->m_user_src = user;
    src_res->m_using_res = be_using;

    TAILQ_INSERT_TAIL(&user->m_using_ress, src_res, m_next_for_src);

    return src_res;
}

ui_data_src_res_t ui_data_src_res_create_by_path(ui_data_src_t src, const char * path) {
    ui_data_mgr_t mgr = src->m_mgr;
    ui_cache_res_t res;

    res = ui_cache_res_find_by_path(src->m_mgr->m_cache_mgr, path);
    if (res == NULL) {
        CPE_ERROR(
            mgr->m_em, "ui_data_src_res_create: %s using res %s not exist!",
            ui_data_src_data(src), path);
        return NULL;
    }

    return ui_data_src_res_create(src, res);
}

uint16_t ui_data_src_res_remove(ui_data_src_t user, ui_cache_res_t bu_using) {
    ui_data_src_res_t src_res, next_src_res;
    uint8_t removed_count = 0;
    
    for(src_res = TAILQ_FIRST(&user->m_using_ress); src_res; src_res = next_src_res) {
        next_src_res = TAILQ_NEXT(src_res, m_next_for_src);

        if (src_res->m_using_res == bu_using) {
            ui_data_src_res_free(src_res);
            removed_count++;
        }
    }

    return removed_count;
}

void ui_data_src_res_free(ui_data_src_res_t src_res) {
    ui_data_mgr_t mgr = src_res->m_user_src->m_mgr;
    
    TAILQ_REMOVE(&src_res->m_user_src->m_using_ress, src_res, m_next_for_src);

    src_res->m_user_src = (void*)mgr;
    TAILQ_INSERT_TAIL(&mgr->m_free_src_ress, src_res, m_next_for_src);
}

void ui_data_src_res_real_free(ui_data_src_res_t src_res) {
    ui_data_mgr_t mgr = (void*)src_res->m_user_src;
    TAILQ_REMOVE(&mgr->m_free_src_ress, src_res, m_next_for_src);
    mem_free(mgr->m_alloc, src_res);
}
