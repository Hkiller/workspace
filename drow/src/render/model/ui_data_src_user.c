#include "ui_data_src_user_i.h"

ui_data_src_user_t
ui_data_src_user_create(ui_data_src_t src, void * ctx, ui_data_src_on_unload_fun_t fun) {
    ui_data_mgr_t mgr = src->m_mgr;
    ui_data_src_user_t user;

    user = TAILQ_FIRST(&mgr->m_free_src_users);
    if (user) {
        TAILQ_REMOVE(&mgr->m_free_src_users, user, m_next);
    }
    else {
        user = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_src_user));
        if (user == NULL) {
            CPE_ERROR(mgr->m_em, "ui_data_src_user_create: alloc fail!");
            return NULL;
        }
    }

    user->m_src = src;
    user->m_ctx = ctx;
    user->m_fun = fun;

    TAILQ_INSERT_TAIL(&src->m_users, user, m_next);

    return user;
}

void ui_data_src_user_free(ui_data_src_user_t src_user) {
    ui_data_mgr_t mgr = src_user->m_src->m_mgr;
    
    TAILQ_REMOVE(&src_user->m_src->m_users, src_user, m_next);

    src_user->m_src = (void*)mgr;
    TAILQ_INSERT_TAIL(&mgr->m_free_src_users, src_user, m_next);
}

uint8_t ui_data_src_remove_user(ui_data_src_t src, void * ctx) {
    ui_data_src_user_t user, next_user;
    uint8_t removed_count = 0;
    
    for(user = TAILQ_FIRST(&src->m_users); user; user = next_user) {
        next_user = TAILQ_NEXT(user, m_next);
        if (user->m_ctx == ctx) {
            ui_data_src_user_free(user);
            removed_count++;
        }
    }

    return removed_count;
}

void ui_data_src_user_real_free(ui_data_src_user_t src_user) {
    ui_data_mgr_t mgr = (void*)src_user->m_src;
    TAILQ_REMOVE(&mgr->m_free_src_users, src_user, m_next);
    mem_free(mgr->m_alloc, src_user);
}
