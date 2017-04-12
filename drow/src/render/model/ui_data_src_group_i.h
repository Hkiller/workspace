#ifndef UI_DATA_SRC_GROUP_I_H
#define UI_DATA_SRC_GROUP_I_H
#include "render/model/ui_data_src_group.h"
#include "ui_data_mgr_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_group {
    ui_data_mgr_t m_mgr;
    uint32_t m_item_count;
    ui_data_src_group_item_list_t m_items;
};

#ifdef __cplusplus
}
#endif

#endif
