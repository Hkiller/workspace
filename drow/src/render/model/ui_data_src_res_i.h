#ifndef UI_DATA_SRC_REF_I_H
#define UI_DATA_SRC_REF_I_H
#include "render/model/ui_data_src_res.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_res {
    ui_data_src_t m_user_src;
    TAILQ_ENTRY(ui_data_src_res) m_next_for_src;
    ui_cache_res_t m_using_res;
};

void ui_data_src_res_real_free(ui_data_src_res_t src_res);

#ifdef __cplusplus
}
#endif

#endif
