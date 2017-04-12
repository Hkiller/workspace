#ifndef UI_DATA_SRC_GROUP_ITEM_I_H
#define UI_DATA_SRC_GROUP_ITEM_I_H
#include "render/model/ui_data_src_group.h"
#include "ui_data_src_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_group_item {
    ui_data_src_group_t m_group;
    TAILQ_ENTRY(ui_data_src_group_item) m_next_for_group;
    ui_data_src_t m_src;
    TAILQ_ENTRY(ui_data_src_group_item) m_next_for_src;
    uint8_t m_is_expand;
};

ui_data_src_group_item_t ui_data_src_group_item_create(ui_data_src_group_t group, ui_data_src_t src);
void ui_data_src_group_item_free(ui_data_src_group_item_t item);

ui_data_src_group_item_t ui_data_src_group_item_find(ui_data_src_group_t group, ui_data_src_t src);
    
#ifdef __cplusplus
}
#endif

#endif
