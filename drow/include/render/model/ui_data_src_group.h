#ifndef UI_MODEL_DATA_SRC_GROUP_H
#define UI_MODEL_DATA_SRC_GROUP_H
#include "ui_model_types.h"
#include "protocol/render/model/ui_object_ref.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_data_src_group_t ui_data_src_group_create(ui_data_mgr_t mgr);
void ui_data_src_group_free(ui_data_src_group_t group);

ui_data_mgr_t ui_data_src_group_mgr(ui_data_src_group_t group);

void ui_data_src_group_clear(ui_data_src_group_t group);
    
int ui_data_src_group_add_src(ui_data_src_group_t group, ui_data_src_t src);
int ui_data_src_group_add_src_by_path(ui_data_src_group_t group, const char * src_path, ui_data_src_type_t src_type);
int ui_data_src_group_add_src_by_id(ui_data_src_group_t group, uint32_t src_id);
int ui_data_src_group_add_src_by_url(ui_data_src_group_t group, UI_OBJECT_URL const * url);
int ui_data_src_group_add_src_by_res(ui_data_src_group_t group, const char * url);
int ui_data_src_group_add_src_by_group(ui_data_src_group_t group, ui_data_src_group_t from_group);
    
int ui_data_src_group_remove_src(ui_data_src_group_t group, ui_data_src_t src);

void ui_data_src_group_srcs(ui_data_src_it_t src_it, ui_data_src_group_t group);

int ui_data_src_group_load_all(ui_data_src_group_t group);
int ui_data_src_group_expand_dir(ui_data_src_group_t group);

int ui_data_src_group_collect_ress(ui_data_src_group_t group, ui_cache_group_t res_group);

#ifdef __cplusplus
}
#endif

#endif
