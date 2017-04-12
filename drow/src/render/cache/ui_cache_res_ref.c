#include <assert.h>
#include "ui_cache_res_ref_i.h"
#include "ui_cache_res_i.h"
#include "ui_cache_group_i.h"

ui_cache_res_ref_t ui_cache_res_ref_create(ui_cache_group_t group, ui_cache_res_t res) {
    ui_cache_res_ref_t ref;
    ui_cache_manager_t mgr = group->m_mgr;

    ref = mem_alloc(mgr->m_alloc, sizeof(struct ui_cache_res_ref));
    if (ref == NULL) {
        CPE_ERROR(mgr->m_em, "ui_cache_res_ref alloc fail!");
        return NULL;
    }

    ref->m_group = group;
    ref->m_res = res;

    TAILQ_INSERT_TAIL(&group->m_using_ress, ref, m_next_for_group);
    TAILQ_INSERT_TAIL(&res->m_in_groups, ref, m_next_for_res);

    return ref;
}

void ui_cache_res_ref_free(ui_cache_res_ref_t ref) {
    ui_cache_manager_t mgr = ref->m_group->m_mgr;

    TAILQ_REMOVE(&ref->m_group->m_using_ress, ref, m_next_for_group);
    TAILQ_REMOVE(&ref->m_res->m_in_groups, ref, m_next_for_res);

    mem_free(mgr->m_alloc, ref);
}

