#ifndef UI_DATA_SRC_USER_I_H
#define UI_DATA_SRC_USER_I_H
#include "render/model/ui_data_src_user.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_user {
    ui_data_src_t m_src;
    TAILQ_ENTRY(ui_data_src_user) m_next;
    void * m_ctx;
    ui_data_src_on_unload_fun_t m_fun;
};

void ui_data_src_user_real_free(ui_data_src_user_t src_user);

#ifdef __cplusplus
}
#endif

#endif
