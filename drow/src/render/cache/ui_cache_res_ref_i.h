#ifndef UI_CACHE_RES_REF_I_H
#define UI_CACHE_RES_REF_I_H
#include "render/cache/ui_cache_types.h"
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_res_ref {
    ui_cache_group_t m_group;
    ui_cache_res_t m_res;
    TAILQ_ENTRY(ui_cache_res_ref) m_next_for_group;
    TAILQ_ENTRY(ui_cache_res_ref) m_next_for_res;
};

ui_cache_res_ref_t ui_cache_res_ref_create(ui_cache_group_t group, ui_cache_res_t res);
void ui_cache_res_ref_free(ui_cache_res_ref_t ref);

#ifdef __cplusplus
}
#endif

#endif
