#ifndef UI_MODEL_DATA_LANGUAGE_H
#define UI_MODEL_DATA_LANGUAGE_H
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_language_it {
    ui_data_language_t (*next)(struct ui_data_language_it * it);
    char m_data[64];
};
    
ui_data_language_t
ui_data_language_create(ui_data_mgr_t data_mgr, const char * name);
void ui_data_language_free(ui_data_language_t language);

const char * ui_data_language_name(ui_data_language_t language);

ui_data_language_t ui_data_active_language(ui_data_mgr_t data_mgr);
    
ui_data_language_t
ui_data_language_find(ui_data_mgr_t data_mgr, const char * name);

void ui_data_language_srcs(ui_data_src_it_t it, ui_data_language_t language);

void ui_data_language_active(ui_data_language_t language);
void ui_data_language_deactive(ui_data_language_t language);

ui_data_src_t ui_data_language_find_src(ui_data_language_t language, ui_data_src_t base_src);

void ui_data_languages(ui_data_language_it_t it, ui_data_mgr_t mgr);
    
#define ui_data_language_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif 
