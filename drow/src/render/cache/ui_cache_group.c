#include <assert.h>
#include "cpe/utils/file.h"
#include "ui_cache_group_i.h"
#include "ui_cache_res_i.h"
#include "ui_cache_res_ref_i.h"

ui_cache_group_t ui_cache_group_create(ui_cache_manager_t mgr) {
    ui_cache_group_t group;

    group = mem_alloc(mgr->m_alloc, sizeof(struct ui_cache_group));
    if (group == NULL) {
        CPE_ERROR(mgr->m_em, "%s: group: alloc fail!", ui_cache_manager_name(mgr));
    }

    group->m_mgr = mgr;
    group->m_res_using_state = ui_cache_res_using_state_free;

    TAILQ_INIT(&group->m_using_ress);

    TAILQ_INSERT_TAIL(&mgr->m_groups, group, m_next_for_mgr);

    return group;
}

void ui_cache_group_free(ui_cache_group_t group) {
    ui_cache_manager_t mgr = group->m_mgr;

    while(!TAILQ_EMPTY(&group->m_using_ress)) {
        ui_cache_res_ref_free(TAILQ_FIRST(&group->m_using_ress));
    }

    TAILQ_REMOVE(&mgr->m_groups, group, m_next_for_mgr);

    mem_free(mgr->m_alloc, group);
}

void ui_cache_group_free_all(ui_cache_manager_t mgr) {
    while(!TAILQ_EMPTY(&mgr->m_groups)) {
        ui_cache_group_free(TAILQ_FIRST(&mgr->m_groups));
    }
}

ui_cache_manager_t ui_cache_group_mgr(ui_cache_group_t group) {
    return group->m_mgr;
}

ui_cache_res_using_state_t ui_cache_group_using_state(ui_cache_group_t group) {
    return group->m_res_using_state;
}

void ui_cache_group_set_using_state(ui_cache_group_t group, ui_cache_res_using_state_t using_state) {
    group->m_res_using_state = using_state;
}

int ui_cache_group_add_res(ui_cache_group_t group, ui_cache_res_t res) {
    if (ui_cache_res_in_group(res, group)) return 0;
    
    return ui_cache_res_ref_create(group, res) ? 0 : -1;
}

int ui_cache_group_add_res_by_path(ui_cache_group_t group, const char * res_path) {
    ui_cache_res_t res;
    
    res = ui_cache_res_find_by_path(group->m_mgr, res_path);
    if (res == NULL) {
        const char * postfix = file_name_suffix(res_path);
        ui_cache_res_type_t type;

        if (strcmp(postfix, "ogg") == 0) {
            type = ui_cache_res_type_sound;
        }
        else if (strcmp(postfix, "ttf") == 0) {
            type = ui_cache_res_type_font;
        }
        else if (strcmp(postfix, "png") == 0 || strcmp(postfix, "pzd") == 0) {
            type = ui_cache_res_type_texture;
        }
        else {
            CPE_ERROR(group->m_mgr->m_em, "ui_cache_group_add_res_by_path: unknown postfix %s", postfix);
            return -1;
        }
        
        res = ui_cache_res_create(group->m_mgr, type);
        if (res == NULL) return -1;

        if (ui_cache_res_set_path(res, res_path) != 0) {
            ui_cache_res_free(res);
            return -1;
        }
    }
    
    return ui_cache_group_add_res(group, res);
}

int ui_cache_group_add_res_by_cache_group(ui_cache_group_t group, ui_cache_group_t from_group) {
    ui_cache_res_ref_t ref;
    int rv = 0;
    
    TAILQ_FOREACH(ref, &from_group->m_using_ress, m_next_for_group) {
        if (ui_cache_group_add_res(group, ref->m_res) != 0) rv = -1;
    }

    return rv;
}

static void ui_cache_group_remove_res_i(ui_cache_res_ref_t ref) {
    ui_cache_group_t group = ref->m_group;
    ui_cache_res_t res = ref->m_res;
    
    ui_cache_res_ref_free(ref);

    if (group->m_res_using_state == ui_cache_res_using_state_ref_count) {
        if (ui_cache_res_using_state(res) == ui_cache_res_using_state_free) {
            ui_cache_res_unload(res, 0);
        }
    }
}

void ui_cache_group_remove_res(ui_cache_group_t group, ui_cache_res_t res) {
    ui_cache_res_ref_t ref;

    TAILQ_FOREACH(ref, &res->m_in_groups, m_next_for_res) {
        if (ref->m_group == group) {
            ui_cache_group_remove_res_i(ref);
            break;
        }
    }
}

void ui_cache_group_clear(ui_cache_group_t group) {
    while(!TAILQ_EMPTY(&group->m_using_ress)) {
        ui_cache_group_remove_res_i(TAILQ_FIRST(&group->m_using_ress));
    }
}

ui_cache_res_t ui_cache_group_first(ui_cache_group_t group) {
    ui_cache_res_ref_t ref = TAILQ_FIRST(&group->m_using_ress);
    return ref ? ref->m_res : NULL;
}

struct ui_cache_group_using_res_it_data {
    ui_cache_res_ref_t m_cur;
};

static ui_cache_res_t ui_cache_group_using_res_next(struct ui_cache_res_it * it) {
    struct ui_cache_group_using_res_it_data * data = (struct ui_cache_group_using_res_it_data *)(it->m_data);
    ui_cache_res_ref_t r;

    if (data->m_cur == NULL) return NULL;

    r = data->m_cur;

    data->m_cur = TAILQ_NEXT(r, m_next_for_group);

    return r->m_res;
}

void ui_cache_group_using_resources(ui_cache_res_it_t it, ui_cache_group_t group) {
    struct ui_cache_group_using_res_it_data * data = (struct ui_cache_group_using_res_it_data *)(it->m_data);

    data->m_cur = TAILQ_FIRST(&group->m_using_ress);
    it->next = ui_cache_group_using_res_next;
}

void ui_cache_group_load_all_async(ui_cache_group_t group) {
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    
    ui_cache_group_using_resources(&res_it, group);

    while((res = ui_cache_res_it_next(&res_it))) {
        if (ui_cache_res_load_state(res) != ui_cache_res_not_load) continue;

        if (ui_cache_res_load_async(res, NULL) != 0) {
            CPE_ERROR(group->m_mgr->m_em, "res %s load async fail", ui_cache_res_path(res));
        }
    }
}

void ui_cache_group_load_all_sync(ui_cache_group_t group) {
    ui_cache_res_ref_t ref;
    
    TAILQ_FOREACH_REVERSE(ref, &group->m_using_ress, ui_cache_res_ref_list, m_next_for_group) {
        switch(ref->m_res->m_load_state) {
        case ui_cache_res_loaded:
        case ui_cache_res_load_fail:
            break;
        default:
            ui_cache_res_load_sync(ref->m_res, NULL);
        }
    }
}
