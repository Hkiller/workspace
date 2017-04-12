#ifndef UI_MODEL_ED_SEARCH_H
#define UI_MODEL_ED_SEARCH_H
#include "ui_ed_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_ed_search_t ui_ed_search_create(ui_ed_mgr_t ed);
void ui_ed_search_free(ui_ed_search_t);

int ui_ed_search_add_obj_type(ui_ed_search_t search, ui_ed_obj_type_t obj_type);
int ui_ed_search_add_root(ui_ed_search_t search, ui_data_src_t root);

ui_ed_obj_t ui_ed_obj_search_next(ui_ed_search_t search);

#ifdef __cplusplus
}
#endif

#endif

