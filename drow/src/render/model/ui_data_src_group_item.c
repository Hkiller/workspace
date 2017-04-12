#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "ui_data_src_group_item_i.h"
#include "ui_data_src_group_i.h"
#include "ui_data_src_i.h"

ui_data_src_group_item_t
ui_data_src_group_item_create(ui_data_src_group_t group, ui_data_src_t src) {
    ui_data_src_group_item_t new_item;

    TAILQ_FOREACH(new_item, &src->m_items, m_next_for_src) {
        if (new_item->m_group == group) return new_item;
    }

    new_item = mem_alloc(group->m_mgr->m_alloc, sizeof(struct ui_data_src_group_item));
    if (new_item == NULL) {
        CPE_ERROR(group->m_mgr->m_em, "ui_data_src_group_add_src: alloc item fail");
        return NULL;
    }

    new_item->m_group = group;
    TAILQ_INSERT_TAIL(&group->m_items, new_item, m_next_for_group);
    new_item->m_src = src;
    TAILQ_INSERT_TAIL(&src->m_items, new_item, m_next_for_src);
    new_item->m_is_expand = 0;
    
    return new_item;
}

void ui_data_src_group_item_free(ui_data_src_group_item_t item) {
    ui_data_src_group_t group = item->m_group;

    TAILQ_REMOVE(&item->m_group->m_items, item, m_next_for_group);
    TAILQ_REMOVE(&item->m_src->m_items, item, m_next_for_src);
    
    mem_free(group->m_mgr->m_alloc, item);
}
