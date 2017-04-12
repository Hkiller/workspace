#ifndef UI_MODEL_DATA_SRC_SRC_H
#define UI_MODEL_DATA_SRC_SRC_H
#include "protocol/render/model/ui_object_ref.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_src_it {
    ui_data_src_t (*next)(struct ui_data_src_src_it * it);
    char m_data[64];
};

/*src create and free*/
ui_data_src_src_t ui_data_src_src_create(ui_data_src_t user, ui_data_src_t be_used);
ui_data_src_src_t ui_data_src_src_create_by_id(ui_data_src_t user, uint32_t src_id, uint8_t type);
ui_data_src_src_t ui_data_src_src_create_by_path(ui_data_src_t user, const char * path, uint8_t type);
ui_data_src_src_t ui_data_src_src_create_by_src_ref(ui_data_src_t user, UI_OBJECT_SRC_REF const * obj_ref, uint8_t type);
int ui_data_src_src_create_by_url(ui_data_src_t user, UI_OBJECT_URL const * url);

uint16_t ui_data_src_src_remove(ui_data_src_t user, ui_data_src_t be_used);

void ui_data_src_src_free(ui_data_src_src_t src_src);

#define ui_data_src_src_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
