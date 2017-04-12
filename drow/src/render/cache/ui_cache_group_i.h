#ifndef UI_CACHE_GROUP_I_H
#define UI_CACHE_GROUP_I_H
#include "render/cache/ui_cache_group.h"
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_group {
    ui_cache_manager_t m_mgr;
    ui_cache_res_using_state_t m_res_using_state;
    TAILQ_ENTRY(ui_cache_group) m_next_for_mgr;
    ui_cache_res_ref_list_t m_using_ress;
};

void ui_cache_group_free_all(ui_cache_manager_t mgr);

uint32_t ui_cache_group_hash(const ui_cache_group_t group);
int ui_cache_group_eq(const ui_cache_group_t l, const ui_cache_group_t r);

#ifdef __cplusplus
}
#endif

#endif
