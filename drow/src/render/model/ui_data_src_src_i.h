#ifndef UI_DATA_SRC_SRC_I_H
#define UI_DATA_SRC_SRC_I_H
#include "render/model/ui_data_src_src.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_src {
    ui_data_src_t m_using_src;
    TAILQ_ENTRY(ui_data_src_src) m_next_for_using;
    ui_data_src_t m_user_src;
    TAILQ_ENTRY(ui_data_src_src) m_next_for_user;
};

void ui_data_src_src_real_free(ui_data_src_src_t src_src);

#ifdef __cplusplus
}
#endif

#endif
