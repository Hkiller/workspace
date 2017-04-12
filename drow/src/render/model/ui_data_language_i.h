#ifndef UI_DATA_LANGUAGE_I_H
#define UI_DATA_LANGUAGE_I_H
#include "render/model/ui_data_language.h"
#include "ui_data_mgr_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_language {
    ui_data_mgr_t m_data_mgr;
    TAILQ_ENTRY(ui_data_language) m_next;
    char m_name[32];
    ui_data_src_list_t m_srcs;
};

void ui_data_language_connect_src(ui_data_language_t language, ui_data_src_t language_src, ui_data_src_t base_src);
void ui_data_language_disconnect_src(ui_data_src_t language_src, ui_data_src_t base_src);
    
#ifdef __cplusplus
}
#endif

#endif
