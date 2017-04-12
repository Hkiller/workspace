#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_layout.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_page_meta_i.h"
#include "plugin_ui_phase_use_page_i.h"
#include "plugin_ui_page_eh_i.h"
#include "plugin_ui_page_plugin_i.h"
#include "plugin_ui_page_slot_i.h"
#include "plugin_ui_state_node_page_i.h"
#include "plugin_ui_state_node_i.h"
#include "plugin_ui_popup_i.h"
#include "plugin_ui_control_meta_i.h"

plugin_ui_page_t
plugin_ui_page_create(plugin_ui_env_t env, const char * name, plugin_ui_page_meta_t page_meta) {
    plugin_ui_module_t module = env->m_module;
    plugin_ui_page_t page;
    plugin_ui_control_meta_t window_meta;
    
    window_meta = plugin_ui_control_meta_find(module, ui_control_type_window);
    if (window_meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_page: create: ui_control_type_window meta not exist!");
        return NULL;
    }
        
    page = mem_alloc(module->m_alloc, sizeof(struct plugin_ui_page) + (page_meta ? page_meta->m_data_capacity : 0));
    if (page == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_page: create: alloc fail!");
        return NULL;
    }

    page->m_env = env;
    page->m_page_meta = page_meta;
    page->m_control_count = 1;
    page->m_load_from = NULL;
    page->m_src = NULL;
    page->m_page_data_meta = NULL;
    page->m_page_data = NULL;
    page->m_page_data_size = 0;
    page->m_is_in_visible_queue = 0;
    page->m_hiding = 0;
    page->m_load_policy = plugin_ui_page_load_policy_visiable;
    page->m_visible_in_popup = NULL;
    page->m_force_change = 0;
    page->m_changed = 1;    
    page->m_packages = NULL;
    page->m_packages_r = NULL;
    
    TAILQ_INIT(&page->m_ehs);
    TAILQ_INIT(&page->m_plugins);
    TAILQ_INIT(&page->m_slots);
    TAILQ_INIT(&page->m_control_actions);

    TAILQ_INIT(&page->m_created_popups);
    TAILQ_INIT(&page->m_building_controls);
    TAILQ_INIT(&page->m_used_by_phases);
    TAILQ_INIT(&page->m_visible_in_states);
    TAILQ_INIT(&page->m_need_process_bindings);

    if (plugin_ui_control_do_init(page, &page->m_control, window_meta) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_page: page %s init root window control fail!", name);
        mem_free(module->m_alloc, page);
        return NULL;
    }
    cpe_ba_set(&page->m_control.m_flag, plugin_ui_control_flag_visible, cpe_ba_false);
    
    page->m_control.m_render_pt_to_p = UI_VECTOR_2_ZERO;
    page->m_control.m_render_pt_abs = UI_VECTOR_2_ZERO;
    page->m_control.m_render_scale = UI_VECTOR_2_IDENTITY;
    page->m_control.m_render_angle = UI_VECTOR_3_ZERO;
    page->m_control.m_render_color.a = 1.0f;
    page->m_control.m_render_color.r = 1.0f;
    page->m_control.m_render_color.g = 1.0f;
    page->m_control.m_render_color.b = 1.0f;
    page->m_control.m_editor_sz = env->m_origin_sz;
    page->m_control.m_cliped_rt_abs.lt = UI_VECTOR_2_ZERO;
    page->m_control.m_client_real_pd.lt = UI_VECTOR_2_ZERO;
    page->m_control.m_client_real_pd.rb = UI_VECTOR_2_ZERO;
    page->m_control.m_cache_flag = 0;

    plugin_ui_control_set_name(&page->m_control, name);

    if (page_meta && page_meta->m_init(page) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_page: init page %s type %s fail!", name ? name : "", page_meta->m_name);
        plugin_ui_control_do_fini(&page->m_control);
        mem_free(module->m_alloc, page);
        return NULL;
    }

    if (page->m_control.m_name) {
        cpe_hash_entry_init(&page->m_hh_for_env);
        if (cpe_hash_table_insert(&env->m_pages_by_name, page) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_page: name %s duplicate!", name);
            if (page_meta) page_meta->m_fini(page);
            plugin_ui_control_do_fini(&page->m_control);
            mem_free(module->m_alloc, page);
            return NULL;
        }
    }

    if (page_meta) {
        TAILQ_INSERT_TAIL(&page_meta->m_pages, page, m_next_for_meta);
    }
    env->m_page_count++;
    TAILQ_INSERT_TAIL(&env->m_pages, page, m_next_for_env);

    plugin_ui_page_update_area(page);

    return page;
}

void plugin_ui_page_free(plugin_ui_page_t page) {
    plugin_ui_env_t env = page->m_env;

    if (page->m_visible_in_popup) {
        assert(page->m_visible_in_popup->m_page == page);
        page->m_visible_in_popup->m_page = NULL;
        page->m_visible_in_popup = NULL;
    }

    if (page->m_control.m_is_processing) {
        assert(!page->m_control.m_is_free);
        page->m_control.m_is_free = 1;

        while(!TAILQ_EMPTY(&page->m_control.m_childs)) {
            plugin_ui_control_free(TAILQ_FIRST(&page->m_control.m_childs));
        }

        return;
    }

    if (page->m_is_in_visible_queue) {
        assert(env->m_visible_page_count > 0);
        env->m_visible_page_count--;
        TAILQ_REMOVE(&env->m_visible_pages, page, m_next_for_visible_queue);
        page->m_is_in_visible_queue = 0;

        if (!page->m_hiding) {
            if (page->m_page_meta && page->m_page_meta->m_on_hide) {
                page->m_page_meta->m_on_hide(page);
            }
        }
    }

    if (page->m_hiding) {
        if (page->m_page_meta && page->m_page_meta->m_on_hide) {
            page->m_page_meta->m_on_hide(page);
        }
        
        TAILQ_REMOVE(&env->m_hiding_pages, page, m_next_for_hiding);
        page->m_hiding = 0;
    }
    
    while(!TAILQ_EMPTY(&page->m_visible_in_states)) {
        plugin_ui_state_node_page_free(TAILQ_FIRST(&page->m_visible_in_states));
    }

    while(!TAILQ_EMPTY(&page->m_used_by_phases)) {
        plugin_ui_phase_use_page_free(TAILQ_FIRST(&page->m_used_by_phases));
    }

    while(!TAILQ_EMPTY(&page->m_created_popups)) {
        plugin_ui_popup_free(TAILQ_FIRST(&page->m_created_popups));
    }
    
    if (page->m_page_meta) page->m_page_meta->m_fini(page);

    while(!TAILQ_EMPTY(&page->m_ehs)) {
        assert(TAILQ_FIRST(&page->m_ehs)->m_is_processing == 0);
        plugin_ui_page_eh_free(TAILQ_FIRST(&page->m_ehs));
    }

    while(!TAILQ_EMPTY(&page->m_plugins)) {
        plugin_ui_page_plugin_free(TAILQ_FIRST(&page->m_plugins));
    }

    while(!TAILQ_EMPTY(&page->m_slots)) {
        plugin_ui_page_slot_free(TAILQ_FIRST(&page->m_slots));
    }
    
    if (page->m_control.m_name) {
        cpe_hash_table_remove_by_ins(&env->m_pages_by_name, page);
    }
    plugin_ui_control_do_fini(&page->m_control);

    if (page->m_packages) {
        plugin_package_group_clear(page->m_packages);
        page->m_packages = NULL;
    }

    if (page->m_packages_r) {
        plugin_package_group_clear(page->m_packages_r);
        page->m_packages_r = NULL;
    }

    assert(env->m_page_count > 0);
    env->m_page_count--;
    
    TAILQ_REMOVE(&env->m_pages, page, m_next_for_env);    

    while(!TAILQ_EMPTY(&page->m_building_controls)) {
        plugin_ui_control_free(TAILQ_FIRST(&page->m_building_controls));
    }

    if (page->m_load_from) {
        mem_free(env->m_module->m_alloc, page->m_load_from);
        page->m_load_from = NULL;
    }

    assert(TAILQ_EMPTY(&page->m_need_process_bindings));
    assert(TAILQ_EMPTY(&page->m_control_actions));

    if (page->m_page_meta) {
        TAILQ_REMOVE(&page->m_page_meta->m_pages, page, m_next_for_meta);
    }

    assert(page->m_control_count == 1);
        
    mem_free(env->m_module->m_alloc, page);
}

plugin_ui_page_load_policy_t plugin_ui_page_load_policy(plugin_ui_page_t page) {
    return page->m_load_policy;
}

void plugin_ui_page_set_load_policy(plugin_ui_page_t page, plugin_ui_page_load_policy_t policy) {
    page->m_load_policy = policy;
}

plugin_ui_page_t plugin_ui_page_find(plugin_ui_env_t env, const char * page_name) {
    struct plugin_ui_page key;
    key.m_control.m_name = (char*)page_name;
    return cpe_hash_table_find(&env->m_pages_by_name, &key);
}

static plugin_ui_page_t plugin_ui_env_page_next(struct plugin_ui_page_it * it) {
    plugin_ui_page_t * data = (plugin_ui_page_t *)(it->m_data);
    plugin_ui_page_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_env);
    return r;
}

void plugin_ui_env_pages(plugin_ui_env_t env, plugin_ui_page_it_t it) {
    *(plugin_ui_page_t *)(it->m_data) = TAILQ_FIRST(&env->m_pages);
    it->next = plugin_ui_env_page_next;
}

static plugin_ui_page_t plugin_ui_env_visiable_page_next(struct plugin_ui_page_it * it) {
    plugin_ui_page_t * data = (plugin_ui_page_t *)(it->m_data);
    plugin_ui_page_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_visible_queue);
    return r;
}

void plugin_ui_env_visiable_pages(plugin_ui_env_t env, plugin_ui_page_it_t page_it) {
    *(plugin_ui_page_t *)(page_it->m_data) = TAILQ_FIRST(&env->m_visible_pages);
    page_it->next = plugin_ui_env_visiable_page_next;
}

plugin_ui_popup_t plugin_ui_page_visible_in_popup(plugin_ui_page_t page) {
    return page->m_visible_in_popup;
}

uint8_t plugin_ui_page_have_popup(plugin_ui_page_t page) {
    return TAILQ_EMPTY(&page->m_created_popups) ? 0 : 1;
}

plugin_ui_popup_t plugin_ui_page_find_popup_by_name(plugin_ui_page_t page, const char * popup_name) {
    plugin_ui_popup_t popup;

    TAILQ_FOREACH(popup, &page->m_created_popups, m_next_for_create_from_page) {
        if (strcmp(popup->m_name, popup_name) == 0) return popup;
    }

    return NULL;
}

const char * plugin_ui_page_name(plugin_ui_page_t page) {
    if (page->m_control.m_name) return page->m_control.m_name;
    if (page->m_visible_in_popup) return page->m_visible_in_popup->m_name;
    return "";
}

gd_app_context_t plugin_ui_page_app(plugin_ui_page_t page) {
    return page->m_env->m_module->m_app;
}

plugin_ui_env_t plugin_ui_page_env(plugin_ui_page_t page) {
    return page->m_env;
}

uint8_t plugin_ui_page_force_change(plugin_ui_page_t page) {
    return page->m_force_change;
}

void plugin_ui_page_set_force_change(plugin_ui_page_t page, uint8_t force_change) {
    page->m_force_change = force_change;
}

uint8_t plugin_ui_page_changed(plugin_ui_page_t page) {
    return page->m_changed;
}

void plugin_ui_page_set_changed(plugin_ui_page_t page, uint8_t changed) {
    page->m_changed = changed;
}

plugin_ui_page_meta_t plugin_ui_page_meta(plugin_ui_page_t page) {
    return page->m_page_meta;
}

void * plugin_ui_page_product(plugin_ui_page_t page) {
    return page + 1;
}

plugin_ui_page_t plugin_ui_page_from_product(void * p) {
    return ((plugin_ui_page_t)p) - 1;
}

uint16_t plugin_ui_page_product_capacity(plugin_ui_page_t page) {
    return page->m_page_meta ? page->m_page_meta->m_data_capacity : 0;
}

plugin_ui_control_t plugin_ui_page_root_control(plugin_ui_page_t page) {
    return &page->m_control;
}

void plugin_ui_page_set_data(plugin_ui_page_t page, LPDRMETA meta, void * data, uint32_t data_size) {
    page->m_page_data_meta = meta;
    page->m_page_data = data;
    page->m_page_data_size = data_size; 
}

LPDRMETA plugin_ui_page_data_meta(plugin_ui_page_t page) {
    return page->m_page_data_meta;
}

void * plugin_ui_page_data(plugin_ui_page_t page) {
    return page->m_page_data;
}

uint32_t plugin_ui_page_data_size(plugin_ui_page_t page) {
    return page->m_page_data_size;
}

static int plugin_ui_page_pacakge_build_r(plugin_ui_page_t page) {
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    int rv = 0;
    
    assert(page->m_packages_r);
    assert(page->m_packages);

    plugin_package_group_clear(page->m_packages_r);
    plugin_package_group_packages(&package_it, page->m_packages);

    while((package = plugin_package_package_it_next(&package_it))) {
        if (plugin_package_group_add_package_r(page->m_packages_r, package) != 0) rv = -1;
    }

    return rv;
}

int plugin_ui_page_pacakge_add(plugin_ui_page_t page, const char * package_name) {
    plugin_ui_module_t module = page->m_env->m_module;
    plugin_package_package_t package;

    package = plugin_package_package_find(module->m_package_module, package_name);
    if (package == NULL) {
        CPE_ERROR(module->m_em, "page %s: add package %s: package not exist!", plugin_ui_page_name(page), package_name);
        return -1;
    }

    if (page->m_packages && plugin_package_package_is_in_group(package, page->m_packages)) return 0;
    
    if (page->m_packages == NULL) {
        char group_name[64];
        snprintf(group_name, sizeof(group_name), "page.%s", plugin_ui_page_name(page));
        page->m_packages = plugin_package_group_create(module->m_package_module, group_name);
        if (page->m_packages == NULL) {
            CPE_ERROR(module->m_em, "page %s: add package %s: create package group fail!", plugin_ui_page_name(page), package_name);
            return -1;
        }
    }

    if (page->m_packages_r == NULL) {
        char group_name[64];
        snprintf(group_name, sizeof(group_name), "page.%s.all", plugin_ui_page_name(page));
        page->m_packages_r = plugin_package_group_create(module->m_package_module, group_name);
        if (page->m_packages_r == NULL) {
            CPE_ERROR(module->m_em, "page %s: add package %s: create package group fail!", plugin_ui_page_name(page), package_name);
            return -1;
        }
    }

    if (plugin_package_group_add_package(page->m_packages, package) != 0) {
        CPE_ERROR(module->m_em, "page %s: add package %s: add to group fail!", plugin_ui_page_name(page), package_name);
        return -1;
    }
    
    if (plugin_package_group_add_package_r(page->m_packages_r, package) != 0) {
        CPE_ERROR(module->m_em, "page %s: add package %s: add to group with base fail!", plugin_ui_page_name(page), package_name);
        plugin_ui_page_pacakge_build_r(page);
        return -1;
    }

    return 0;
}

void plugin_ui_page_pacakge_remove_all(plugin_ui_page_t page) {
    if (page->m_packages_r) {
        plugin_package_group_clear(page->m_packages_r);
    }

    if (page->m_packages) {
        plugin_package_group_clear(page->m_packages);
    }
}

int plugin_ui_page_pacakge_load_all_async(plugin_ui_page_t page, plugin_package_load_task_t task) {
    return page->m_packages_r
        ? plugin_package_group_load_async(page->m_packages_r, task)
        : 0;
}

int plugin_ui_page_pacakge_load_all_sync(plugin_ui_page_t page) {
    return page->m_packages_r
        ? plugin_package_group_load_sync(page->m_packages_r)
        : 0;
}

/*event */
void plugin_ui_page_send_event(plugin_ui_page_t page, LPDRMETA meta, void * data, size_t data_size) {
    plugin_ui_env_t env = page->m_env;
    dr_data_t overwrite = NULL;
    struct dr_data overwrite_buf;
    
    if (page->m_page_data_meta) {
        overwrite_buf.m_meta = page->m_page_data_meta;
        overwrite_buf.m_size = page->m_page_data_size;
        overwrite_buf.m_data = page->m_page_data;
        overwrite = &overwrite_buf;
    }

    env->m_backend->send_event(env->m_backend->ctx, env, meta, data, data_size, overwrite);
}

void plugin_ui_page_build_and_send_event(plugin_ui_page_t page, const char * def, dr_data_source_t data_source) {
    plugin_ui_env_t env = page->m_env;
    struct dr_data_source data_source_buf;
    dr_data_t overwrite = NULL;
    struct dr_data overwrite_buf;

    if (page->m_page_data_meta) {
        data_source_buf.m_data.m_meta = page->m_page_data_meta;
        data_source_buf.m_data.m_data = page->m_page_data;
        data_source_buf.m_data.m_size = page->m_page_data_size;
        data_source_buf.m_next = data_source;
        data_source = &data_source_buf;

        overwrite_buf.m_meta = page->m_page_data_meta;
        overwrite_buf.m_size = page->m_page_data_size;
        overwrite_buf.m_data = page->m_page_data;
        overwrite = &overwrite_buf;
    }

    env->m_backend->build_and_send_event(env->m_backend->ctx, env, def, data_source, overwrite);
}    

uint16_t plugin_ui_page_control_count(plugin_ui_page_t page) {
    return page->m_control_count;
}

uint8_t plugin_ui_page_visible(plugin_ui_page_t page) {
    if (page->m_visible_in_popup) {
        return plugin_ui_control_visible(&page->m_control);
    }
    else {
        return TAILQ_EMPTY(&page->m_visible_in_states) ? 0 : 1;
    }
}

uint8_t plugin_ui_page_is_in_render(plugin_ui_page_t page) {
    if (page->m_visible_in_popup) {
        return plugin_ui_control_visible(&page->m_control);
    }
    else {
        plugin_ui_phase_node_t cur_phase;
        plugin_ui_state_node_page_t node_page;
        plugin_ui_state_node_t check_state;

        node_page = TAILQ_LAST(&page->m_visible_in_states, plugin_ui_state_node_page_list);
        if (node_page == NULL) return 0;

        cur_phase = plugin_ui_phase_node_current(page->m_env);
        if (cur_phase == NULL) return 0;

        TAILQ_FOREACH_REVERSE(check_state, &cur_phase->m_state_stack, plugin_ui_state_node_list, m_next) {
            if (check_state == node_page->m_state_node) return 1;
            if (check_state->m_curent.m_suspend_old) return 0;
        }

        return 0;
    }
}

void plugin_ui_page_update_visible(plugin_ui_page_t page, uint8_t visible) {
    plugin_ui_page_eh_t eh, eh_next;
    uint8_t page_tag_local = 0;

    assert(!page->m_control.m_is_free);

    if (plugin_ui_control_visible(&page->m_control) == visible) return;

    /* printf("xxxxx: %s ==> %s(%s)\n", */
    /*        plugin_ui_page_name(page), */
    /*        visible ? "visible" : "hide", */
    /*        plugin_ui_page_is_loaded(page) ? "loaded" : "not-loaded"); */
    
    if (!page->m_control.m_is_processing) {
        page_tag_local = 1;
        page->m_control.m_is_processing = 1;
    }

    if (visible) {
        if (!plugin_ui_page_is_loaded(page) && (page->m_src || page->m_load_from)) {
            if (plugin_ui_page_load(page) != 0) {
                CPE_ERROR(
                    page->m_env->m_module->m_em, "page %s: load on visiable fail!",
                    plugin_ui_page_name(page));
                goto COMPLETE;
            }
            else {
                if (page->m_env->m_debug) {
                    CPE_INFO(
                        page->m_env->m_module->m_em, "page %s: load on visiable success!",
                        plugin_ui_page_name(page));
                }
            }
            if (page->m_control.m_is_free) goto COMPLETE;
        }
    
        if (page->m_visible_in_popup) {
            if (page->m_env->m_backend->popup_enter(page->m_env->m_backend->ctx, page->m_env, page->m_visible_in_popup) != 0) {
                goto COMPLETE;
            }
            if (page->m_control.m_is_free) goto COMPLETE;
        }
    }
    
    cpe_ba_set(&page->m_control.m_flag, plugin_ui_control_flag_visible, visible ? cpe_ba_true : cpe_ba_false);

    if (visible) {
        if (page->m_hiding) {
            TAILQ_REMOVE(&page->m_env->m_hiding_pages, page, m_next_for_hiding);
            page->m_hiding = 0;
        }
        else {
            if (page->m_force_change) {
                page->m_changed = 1;
            }
        }
    }
    else {
        if (!page->m_hiding) {
            TAILQ_INSERT_TAIL(&page->m_env->m_hiding_pages, page, m_next_for_hiding);
            page->m_hiding = 1;
        }
    }

    for(eh = TAILQ_FIRST(&page->m_ehs); eh; eh = eh_next) {
        uint8_t eh_tag_local = 0;

        if (eh->m_is_free) {
            eh_next = TAILQ_NEXT(eh, m_next);
            continue;
        }

        if (!eh->m_is_processing) {
            eh_tag_local = 1;
            eh->m_is_processing = 1;
        }

        plugin_ui_page_eh_sync_active(eh);

        eh_next = TAILQ_NEXT(eh, m_next);

        if (eh_tag_local) {
            eh->m_is_processing = 0;
            if (eh->m_is_free) {
                plugin_ui_page_eh_free(eh);
            }
        }

        if (page->m_control.m_is_free) goto COMPLETE;
    }

    page->m_env->m_visible_pages_need_update = 1;
    
COMPLETE:
    if (page_tag_local) {
        page->m_control.m_is_processing = 0;
        if (page->m_control.m_is_free) plugin_ui_page_free(page);
    }
}

uint8_t plugin_ui_page_is_show_in_state(plugin_ui_page_t page, plugin_ui_state_node_t state_node) {
    return plugin_ui_state_node_page_find(state_node, page) == NULL ? 0 : 1;
}

int plugin_ui_page_show_in_state(
    plugin_ui_page_t page, plugin_ui_state_node_t state_node, dr_data_source_t data_source, const char * before_page,
    plugin_ui_page_state_on_hide_fun_t on_hide_fun, void * on_hide_ctx)
{
    plugin_ui_phase_node_t cur_phase;
    plugin_ui_state_node_page_t node_page;

    if (page->m_visible_in_popup) {
        CPE_ERROR(
            page->m_env->m_module->m_em, "plugin_ui_page_show_in_state: page %s is popup page!",
            plugin_ui_page_name(page));
        return -1;
    }

    if (plugin_ui_state_node_page_find(state_node, page)) {
        CPE_ERROR(
            page->m_env->m_module->m_em, "plugin_ui_page_show_in_state: page %s is already in visible in state %d",
            plugin_ui_page_name(page), state_node->m_level);
        return -1;
    }

    cur_phase = plugin_ui_phase_node_current(page->m_env);
    assert(cur_phase);

    node_page = plugin_ui_state_node_page_create(state_node, page, before_page, on_hide_fun, on_hide_ctx);
    if (node_page == NULL) {
        CPE_ERROR(page->m_env->m_module->m_em, "plugin_ui_page_show_in_state: create node page fail!");
        return -1;
    }

    if (plugin_ui_page_sync_state_data(page, data_source) != 0) {
        CPE_ERROR(page->m_env->m_module->m_em, "plugin_ui_page_show_in_state: sync page data fail!");
        plugin_ui_state_node_page_free(node_page);
        return -1;
    }

    plugin_ui_page_update_visible(page, 1);

    return 0;
}

void plugin_ui_page_clear_hide_monitor(plugin_ui_page_t page, void * on_hide_ctx) {
    plugin_ui_state_node_page_t node_page;

    TAILQ_FOREACH(node_page, &page->m_visible_in_states, m_next_for_page) {
        if (node_page->m_on_hide_ctx == on_hide_ctx) {
            node_page->m_on_hide_fun = NULL;
            node_page->m_on_hide_ctx = NULL;
        }
    }
}

void plugin_ui_page_hide_in_state(plugin_ui_page_t page, plugin_ui_state_node_t state_node) {
    plugin_ui_state_node_page_t node_page;

    if (page->m_visible_in_popup) {
        CPE_ERROR(
            page->m_env->m_module->m_em, "plugin_ui_page_hide_in_state: page %s is popup page!",
            plugin_ui_page_name(page));
        return;
    }

    if ((node_page = plugin_ui_state_node_page_find(state_node, page))) {
        plugin_ui_state_node_page_free(node_page);

        if (TAILQ_EMPTY(&page->m_visible_in_states)) {
            plugin_ui_page_update_visible(page, 0);
        }
    }
}

plugin_ui_state_node_t plugin_ui_page_top_state_node(plugin_ui_page_t page) {
    plugin_ui_state_node_page_t node_page = TAILQ_LAST(&page->m_visible_in_states, plugin_ui_state_node_page_list);
    return node_page ? node_page->m_state_node : NULL;
}

int plugin_ui_page_show_in_current_state(
    plugin_ui_page_t page, dr_data_source_t data_source, const char * before_page,
    plugin_ui_page_state_on_hide_fun_t on_hide_fun, void * on_hide_ctx)
{
    plugin_ui_phase_node_t cur_phase;
    plugin_ui_state_node_t cur_state;

    cur_phase = plugin_ui_phase_node_current(page->m_env);
    assert(cur_phase);

    cur_state = plugin_ui_state_node_current(cur_phase);
    assert(cur_state);

    return plugin_ui_page_show_in_state(page, cur_state, data_source, before_page, on_hide_fun, on_hide_ctx);
}

void plugin_ui_page_do_hide(plugin_ui_page_t page) {
    plugin_ui_popup_t popup;
    uint8_t page_tag_local = 0;

    if (!page->m_control.m_is_processing) {
        page->m_control.m_is_processing = 1;
        page_tag_local = 1;
    }
        
    assert(page->m_hiding);
    assert(!plugin_ui_page_visible(page));

    TAILQ_REMOVE(&page->m_env->m_hiding_pages, page, m_next_for_hiding);
    page->m_hiding = 0;

    TAILQ_FOREACH(popup, &page->m_created_popups, m_next_for_create_from_page) {
        plugin_ui_popup_set_visible(popup, 0);
        if (page->m_control.m_is_free) goto COMPLETE;
    }

    if (page->m_page_meta && page->m_page_meta->m_on_hide) {
        page->m_page_meta->m_on_hide(page);
        if (page->m_control.m_is_free) goto COMPLETE;
    }
        
    if (page->m_visible_in_popup) {
        page->m_env->m_backend->popup_leave(page->m_env->m_backend->ctx, page->m_env, page->m_visible_in_popup);
        if (page->m_control.m_is_free) goto COMPLETE;
    }

    if (page->m_load_policy == plugin_ui_page_load_policy_visiable) {
        plugin_ui_page_unload(page);
    }
    
COMPLETE:
    if (page_tag_local) {
        page->m_control.m_is_processing = 0;
        if (page->m_control.m_is_free) plugin_ui_page_free(page);
    }
}
    
void plugin_ui_page_hide(plugin_ui_page_t page) {
    if (page->m_visible_in_popup) {
        plugin_ui_popup_set_visible(page->m_visible_in_popup, 0);
    }
    else {
        plugin_ui_phase_node_t cur_phase;
        plugin_ui_state_node_t cur_state;

        cur_phase = plugin_ui_phase_node_current(page->m_env);
        assert(cur_phase);

        cur_state = plugin_ui_state_node_current(cur_phase);
        assert(cur_state);

        plugin_ui_page_hide_in_state(page, cur_state);
    }
}

uint8_t plugin_ui_page_modal(plugin_ui_page_t page) {
    return page->m_modal;
}

void plugin_ui_page_set_modal(plugin_ui_page_t page, uint8_t is_modal) {
    page->m_modal = is_modal;
}

int plugin_ui_page_sync_state_data(plugin_ui_page_t page, dr_data_source_t data_source) {
    plugin_ui_env_t env = page->m_env;
    plugin_ui_state_node_page_t node_page;
    void * old_page_data = NULL;
    void * old_page_data_buf = NULL;
    char old_page_data_inline[128];
    
    if (page->m_page_data == NULL) return 0;

    /*只有页面数据没有变化时，才需要检查页面数据变化 */
    if (!page->m_changed && !page->m_force_change) {
        if (page->m_page_data_size <= CPE_ARRAY_SIZE(old_page_data_inline)) {
            old_page_data = old_page_data_inline;
        }
        else {
            old_page_data_buf = mem_alloc(env->m_module->m_alloc, page->m_page_data_size);
            if (old_page_data_buf == NULL) {
                CPE_ERROR(env->m_module->m_em, "page %s sync data: alloc old page data buf fail!", plugin_ui_page_name(page));
            }
            old_page_data = old_page_data_buf;
        }
    }

    /*如果需要检测页面数据变化，则先记录老数据 */
    if (old_page_data) {
        memcpy(old_page_data, page->m_page_data, page->m_page_data_size);
    }
    
    TAILQ_FOREACH(node_page, &page->m_visible_in_states, m_next_for_page) {
        dr_data_t data = plugin_ui_state_node_data(node_page->m_state_node);

        if (data == NULL) continue;

        if (dr_meta_copy_same_entry_part(
                page->m_page_data, page->m_page_data_size, page->m_page_data_meta,
                data->m_data, data->m_size, data->m_meta,
                NULL, 0, env->m_module->m_em)
            < 0)
        {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_page_sync_state_data: page %s copy data fail!",
                plugin_ui_page_name(page));
            if (old_page_data_buf) mem_free(env->m_module->m_alloc, old_page_data_buf);
            return -1;
        }
    }

    while(data_source) {
        if (dr_meta_copy_same_entry_part(
                page->m_page_data, page->m_page_data_size, page->m_page_data_meta,
                data_source->m_data.m_data, data_source->m_data.m_size, data_source->m_data.m_meta,
                NULL, 0, env->m_module->m_em)
            < 0)
        {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_page_sync_state_data: sync copy page data from fsm fail!");
            if (old_page_data_buf) mem_free(env->m_module->m_alloc, old_page_data_buf);
            return -1;
        }

        data_source = data_source->m_next;
    }

    if (old_page_data) {
        if (memcmp(old_page_data, page->m_page_data, page->m_page_data_size) != 0) {
            /* CPE_ERROR( */
            /*     env->m_module->m_em, "plugin_ui_page_sync_state_data: page %s: old data: %s\n", */
            /*     plugin_ui_page_name(page), */
            /*     dr_json_dump_inline(gd_app_tmp_buffer(env->m_module->m_app),  old_page_data,  page->m_page_data_size, page->m_page_data_meta)); */
            /* CPE_ERROR( */
            /*     env->m_module->m_em, "plugin_ui_page_sync_state_data: page %s: new data: %s\n", */
            /*     plugin_ui_page_name(page), */
            /*     dr_json_dump_inline(gd_app_tmp_buffer(env->m_module->m_app),  page->m_page_data,  page->m_page_data_size, page->m_page_data_meta)); */
            page->m_changed = 1;
        }
    }
    else if (page->m_force_change) {
        page->m_changed = 1;
    }

    if (old_page_data_buf) mem_free(env->m_module->m_alloc, old_page_data_buf);

    return 0;
}

void plugin_ui_page_update_area(const plugin_ui_page_t page) {
    plugin_ui_env_t env = page->m_env;
    page->m_control.m_render_sz_abs = env->m_runtime_sz;
    page->m_control.m_render_sz_ns = env->m_runtime_sz;
    page->m_control.m_cliped_rt_abs.rb = env->m_runtime_sz;
    page->m_control.m_client_real_sz = env->m_runtime_sz;
}

uint32_t plugin_ui_page_hash(const plugin_ui_page_t page) {
    return cpe_hash_str(page->m_control.m_name, strlen(page->m_control.m_name));
}

int plugin_ui_page_eq(const plugin_ui_page_t l, const plugin_ui_page_t r) {
    return strcmp(l->m_control.m_name, r->m_control.m_name) == 0;
}

ui_data_src_t plugin_ui_page_src(plugin_ui_page_t page) {
    return page->m_src;
}

void plugin_ui_page_set_src(plugin_ui_page_t page, ui_data_src_t src) {
    page->m_src = src;
}

int plugin_ui_page_set_src_by_path(plugin_ui_page_t page, const char * i_load_from) {
    plugin_ui_module_t module = page->m_env->m_module;
    char * load_from;
    const char * endp;

    endp = strrchr(i_load_from, '/');
    if (endp == NULL) endp = i_load_from;
    endp = strchr(endp, '.');
    if (endp == NULL) endp = i_load_from + strlen(i_load_from);
    
    load_from = cpe_str_mem_dup_range(module->m_alloc, i_load_from, endp);
    if (load_from == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_ui_page_set_src_by_path: %s: alloc fail!",
            plugin_ui_page_name(page));
        return -1;
    }

    if (page->m_load_from) {
        mem_free(module->m_alloc, page->m_load_from);
    }
    page->m_load_from = load_from;

    return 0;
}

uint8_t plugin_ui_page_is_loaded(plugin_ui_page_t page) {
    return page->m_control_count > 1 ? 1 : 0;
}

void plugin_ui_page_unload(plugin_ui_page_t page) {
    plugin_ui_page_plugin_t plugin;
    uint8_t page_tag_local = 0;

    if (!page->m_control.m_is_processing) {
        page->m_control.m_is_processing = 1;
        page_tag_local = 1;
    }

    if (page->m_hiding) {
        plugin_ui_page_do_hide(page);
        if (page->m_control.m_is_free) goto COMPLETE;
    }
    
    TAILQ_FOREACH_REVERSE(plugin, &page->m_plugins, plugin_ui_page_plugin_list, m_next_for_page) {
        if (plugin->m_is_loaded) {
            if (plugin->m_on_unload) plugin->m_on_unload(plugin->m_ctx, page, plugin);
            plugin->m_is_loaded = 0;
        }
    }

    while(!TAILQ_EMPTY(&page->m_ehs)) {
        assert(TAILQ_FIRST(&page->m_ehs)->m_is_processing == 0);
        plugin_ui_page_eh_free(TAILQ_FIRST(&page->m_ehs));
    }
    
    while(!TAILQ_EMPTY(&page->m_control.m_childs)) {
        plugin_ui_control_free(TAILQ_FIRST(&page->m_control.m_childs));
    }

    while(!TAILQ_EMPTY(&page->m_building_controls)) {
        plugin_ui_control_free(TAILQ_FIRST(&page->m_building_controls));
    }

    page->m_src = NULL;
    assert(page->m_control_count == 1);

COMPLETE:
    if (page_tag_local) {
        page->m_control.m_is_processing = 0;
        if (page->m_control.m_is_free) plugin_ui_page_free(page);
    }
}

int plugin_ui_page_load(plugin_ui_page_t page) {
    plugin_ui_module_t module = page->m_env->m_module;
    ui_data_layout_t data_layout;
    ui_data_control_t data_control;
    plugin_ui_page_plugin_t plugin;

    if (plugin_ui_page_is_loaded(page)) {
        CPE_ERROR(module->m_em, "plugin_ui_page_load: page %s: is already loaded!", plugin_ui_page_name(page));
        return -1;
    }

    if (page->m_src == NULL) {
        if (page->m_load_from == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_page_load: page %s: no src!", plugin_ui_page_name(page));
            return -1;
        }

        page->m_src = ui_data_src_find_by_path(module->m_data_mgr, page->m_load_from, ui_data_src_type_layout);
        if (page->m_src == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_ui_page_load: %s: layout %s not exist!",
                plugin_ui_page_name(page), page->m_load_from);
            return -1;
        }
    }
    
    if (!ui_data_src_is_loaded(page->m_src)) {
        CPE_ERROR(
            module->m_em, "plugin_ui_page_load: page %s: layout %s is not loaded!",
            plugin_ui_page_name(page), ui_data_src_path_dump(&module->m_dump_buffer, page->m_src));
        return -1;
    }

    data_layout = ui_data_src_product(page->m_src);
    assert(data_layout);

    data_control = ui_data_layout_root(data_layout);
    if (data_control == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_ui_page_load: page %s: layout %s no root control!",
            plugin_ui_page_name(page), ui_data_src_path_dump(&module->m_dump_buffer, page->m_src));
        return -1;
    }

    page->m_modal = ui_data_control_data(data_control)->data.window.mudal;

    if (plugin_ui_control_load_childs(&page->m_control,  data_control) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_ui_page_load: page %s: layout %s load fail!",
            plugin_ui_page_name(page), ui_data_src_path_dump(&module->m_dump_buffer, page->m_src));

        plugin_ui_control_remove_childs(&page->m_control);
        assert(!plugin_ui_page_is_loaded(page));
        return -1;
    }

    plugin_ui_control_update_cache(&page->m_control, 0);
    
    if (page->m_page_meta && page->m_page_meta->m_on_load) page->m_page_meta->m_on_load(page);

    TAILQ_FOREACH(plugin, &page->m_plugins, m_next_for_page) {
        if (plugin->m_is_loaded) continue;
        
        if (plugin->m_on_load && plugin->m_on_load(plugin->m_ctx, page, plugin) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_page_load: page %s: do setup fail!", plugin_ui_page_name(page));
            continue;

            /*回退，暂时只打印错误 加载流程继续 */
            /* while(!TAILQ_EMPTY(&page->m_control.m_childs)) { */
            /*     plugin_ui_control_free(TAILQ_FIRST(&page->m_control.m_childs)); */
            /* } */
            /* assert(!plugin_ui_page_is_loaded(page)); */
            
            /* for(plugin = TAILQ_PREV(plugin, plugin_ui_page_plugin_list, m_next_for_page); */
            /*     plugin != NULL; */
            /*     plugin = TAILQ_PREV(plugin, plugin_ui_page_plugin_list, m_next_for_page)) */
            /* { */
            /*     assert(plugin->m_is_loaded); */
            /*     if (plugin->m_on_unload) plugin->m_on_unload(plugin->m_ctx, page, plugin); */
            /*     plugin->m_is_loaded = 0; */
            /* } */
            
            /* return -1; */
        }

        plugin->m_is_loaded = 1;
    }

    page->m_changed = 1;
    return 0;
}

int plugin_ui_page_load_by_path(plugin_ui_page_t page, const char * load_from) {
    if (plugin_ui_page_set_src_by_path(page, load_from) != 0) return -1;
    return plugin_ui_page_load(page);
}

int plugin_ui_control_meta_page_regist(plugin_ui_module_t module) {
    if (plugin_ui_control_meta_create(
            module, ui_control_type_window, 0, NULL, NULL, NULL, NULL)
        == NULL)
    {
        return -1;
    }

    return 0;
}

void plugin_ui_control_meta_page_unregist(plugin_ui_module_t module) {
    plugin_ui_module_unregister_control(module, ui_control_type_window);
}
