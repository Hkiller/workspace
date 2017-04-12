#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_popup_i.h"
#include "plugin_ui_popup_action_i.h"
#include "plugin_ui_page_i.h"

plugin_ui_popup_t plugin_ui_popup_create(plugin_ui_env_t env, const char * name, plugin_ui_page_meta_t page_meta, LPDRMETA data_meta) {
    plugin_ui_popup_t popup;
    plugin_ui_page_t page;

    page = plugin_ui_page_create(env, NULL, page_meta);
    if (page == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_popup_create: create page fail!");
        return NULL;
    }

    popup = TAILQ_FIRST(&env->m_free_popups);
    if (popup) {
        TAILQ_REMOVE(&env->m_free_popups, popup, m_next_for_env);
    }
    else {
        popup = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_popup));
        if (popup == NULL) {
            plugin_ui_page_free(page);
            CPE_ERROR(env->m_module->m_em, "plugin_ui_popup_create: alloc fail!");
            return NULL;
        }
    }

    popup->m_id = env->m_popup_max_id + 1;
    popup->m_env = env;
    popup->m_page = page;
    popup->m_def = NULL;
    popup->m_lifecircle = 0.0f;
    popup->m_create_from_page = NULL;
    TAILQ_INIT(&popup->m_actions);
    cpe_str_dup(popup->m_name, sizeof(popup->m_name), name);
    popup->m_layer = 0;

    assert(page->m_visible_in_popup == NULL);
    page->m_visible_in_popup = popup;
    popup->m_allock_data_buf = NULL;
    
    if (data_meta) {
        uint16_t data_size = dr_meta_size(data_meta);
        void * page_data;
        
        if (data_size > CPE_ARRAY_SIZE(popup->m_data_buf)) {
            popup->m_allock_data_buf = mem_alloc(env->m_module->m_alloc, data_size);
            if (popup->m_allock_data_buf == NULL) {
                plugin_ui_page_free(page);
                CPE_ERROR(env->m_module->m_em, "plugin_ui_popup_create: alloc page data(size=%d) fail!", data_size);
                return NULL;
            }
            page_data = popup->m_allock_data_buf;
        }
        else {
            page_data = popup->m_data_buf;
        }
        
        bzero(page_data, data_size);
        plugin_ui_page_set_data(page, data_meta, page_data, data_size);
    }

    env->m_popup_max_id++;
    
    TAILQ_INSERT_TAIL(&env->m_popups, popup, m_next_for_env);

    plugin_ui_popup_adj_layer(popup);
    
    return popup;
}

void plugin_ui_popup_free(plugin_ui_popup_t popup) {
    plugin_ui_env_t env = popup->m_env;

    TAILQ_REMOVE(&env->m_popups, popup, m_next_for_env);

    if (plugin_ui_page_visible(popup->m_page)) {
        env->m_visible_pages_need_update = 1;
        env->m_backend->popup_leave(env->m_backend->ctx, env, popup);
    }

    if (popup->m_allock_data_buf) {
        mem_free(env->m_module->m_alloc, popup->m_allock_data_buf);
        popup->m_allock_data_buf = NULL;
    }

    if (popup->m_create_from_page) {
        TAILQ_REMOVE(&popup->m_create_from_page->m_created_popups, popup, m_next_for_create_from_page);
        popup->m_create_from_page = NULL;
    }
    
    assert(popup->m_page->m_visible_in_popup == popup);

    if (popup->m_page) {
        assert(popup->m_page->m_visible_in_popup == popup);
        plugin_ui_page_free(popup->m_page);
        assert(popup->m_page == NULL);
    }

    while(!TAILQ_EMPTY(&popup->m_actions)) {
        plugin_ui_popup_action_free(TAILQ_FIRST(&popup->m_actions));
    }

    TAILQ_INSERT_TAIL(&env->m_free_popups, popup, m_next_for_env);
}

void plugin_ui_popup_real_free(plugin_ui_popup_t popup) {
    plugin_ui_env_t env = popup->m_env;

    TAILQ_REMOVE(&env->m_free_popups, popup, m_next_for_env);
    mem_free(env->m_module->m_alloc, popup);
}

plugin_ui_popup_t plugin_ui_popup_first(plugin_ui_env_t env) {
    return TAILQ_FIRST(&env->m_popups);
}

plugin_ui_popup_t plugin_ui_popup_prev(plugin_ui_popup_t popup) {
    return TAILQ_PREV(popup, plugin_ui_popup_list, m_next_for_env);
}

plugin_ui_popup_t plugin_ui_popup_next(plugin_ui_popup_t popup) {
    return TAILQ_NEXT(popup, m_next_for_env);
}

void plugin_ui_popup_adj_layer(plugin_ui_popup_t popup) {
    plugin_ui_env_t env = popup->m_env;
    plugin_ui_popup_t insert_pos;
    plugin_ui_popup_t pp;

    if ((insert_pos = TAILQ_PREV(popup, plugin_ui_popup_list, m_next_for_env)) && insert_pos->m_layer > popup->m_layer) {
        TAILQ_REMOVE(&env->m_popups, popup, m_next_for_env);

        for(pp = TAILQ_PREV(insert_pos, plugin_ui_popup_list, m_next_for_env);
            pp && pp->m_layer > popup->m_layer;
            insert_pos = pp, pp = TAILQ_PREV(insert_pos, plugin_ui_popup_list, m_next_for_env)
            )
        {
        }

        if (insert_pos->m_layer > popup->m_layer) {
            /* printf( */
            /*     "xxxxxxxx: prev: popup %s(%d) before %s(%d)\n", */
            /*     plugin_ui_popup_name(popup), popup->m_layer, */
            /*     plugin_ui_popup_name(insert_pos), insert_pos->m_layer); */

            TAILQ_INSERT_BEFORE(insert_pos, popup, m_next_for_env);
        }
        else {
            /* printf( */
            /*     "xxxxxxxx: prev: popup %s(%d) after %s(%d)\n", */
            /*     plugin_ui_popup_name(popup), popup->m_layer, */
            /*     plugin_ui_popup_name(insert_pos), insert_pos->m_layer); */

            
            TAILQ_INSERT_AFTER(&env->m_popups, insert_pos, popup, m_next_for_env);
        }

        if (plugin_ui_popup_visible(popup)) {
            env->m_visible_pages_need_update = 1;
        }
    }
    else if ((insert_pos = TAILQ_NEXT(popup, m_next_for_env)) && insert_pos->m_layer <= popup->m_layer) {
        TAILQ_REMOVE(&env->m_popups, popup, m_next_for_env);

        for(pp = TAILQ_NEXT(insert_pos, m_next_for_env);
            pp && pp->m_layer <= popup->m_layer;
            insert_pos = pp, pp = TAILQ_NEXT(insert_pos, m_next_for_env)
            )
        {
        }

        /* printf( */
        /*     "xxxxxxxx: next: popup %s(%d) after %s(%d)\n", */
        /*     plugin_ui_popup_name(popup), popup->m_layer, */
        /*     plugin_ui_popup_name(insert_pos), insert_pos->m_layer); */
        
        TAILQ_INSERT_AFTER(&env->m_popups, insert_pos, popup, m_next_for_env);
        if (plugin_ui_popup_visible(popup)) {
            env->m_visible_pages_need_update = 1;
        }
    }
}

plugin_ui_popup_t plugin_ui_popup_find_first_by_name(plugin_ui_env_t env, const char * name) {
    plugin_ui_popup_t popup;

    TAILQ_FOREACH(popup, &env->m_popups, m_next_for_env) {
        if (!plugin_ui_popup_visible(popup)) continue;
        if (strcmp(popup->m_name, name) == 0) return popup;
    }

    return NULL;
}

plugin_ui_popup_t plugin_ui_popup_find_by_id(plugin_ui_env_t env, uint32_t popup_id) {
    plugin_ui_popup_t popup;

    TAILQ_FOREACH(popup, &env->m_popups, m_next_for_env) {
        if (popup->m_id == popup_id) return popup;
    }

    return NULL;
}

static plugin_ui_popup_t plugin_ui_env_popup_next(struct plugin_ui_popup_it * it) {
    plugin_ui_popup_t * data = (plugin_ui_popup_t *)(it->m_data);
    plugin_ui_popup_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_env);
    return r;
}

void plugin_ui_env_popups(plugin_ui_env_t env, plugin_ui_popup_it_t it) {
    *(plugin_ui_popup_t *)(it->m_data) = TAILQ_FIRST(&env->m_popups);
    it->next = plugin_ui_env_popup_next;
}

int16_t plugin_ui_popup_layer(plugin_ui_popup_t popup) {
    return popup->m_layer;
}

void plugin_ui_popup_set_layer(plugin_ui_popup_t popup, int16_t layer) {
    if (popup->m_layer != layer) {
        popup->m_layer = layer;
        plugin_ui_popup_adj_layer(popup);
    }
}

uint32_t plugin_ui_popup_id(plugin_ui_popup_t popup) {
    return popup->m_id;
}

plugin_ui_env_t plugin_ui_popup_env(plugin_ui_popup_t popup) {
    return popup->m_env;
}

plugin_ui_page_t plugin_ui_popup_page(plugin_ui_popup_t popup) {
    return popup->m_page;
}

const char * plugin_ui_popup_name(plugin_ui_popup_t popup) {
    return popup->m_name;
}

plugin_ui_popup_def_t plugin_ui_popup_def(plugin_ui_popup_t popup) {
    return popup->m_def;
}

float plugin_ui_popup_lifecircle(plugin_ui_popup_t popup) {
    return popup->m_lifecircle;
}

void plugin_ui_popup_set_lifecircle(plugin_ui_popup_t popup, float lifecircle) {
    popup->m_lifecircle = lifecircle;
}
    
uint8_t plugin_ui_popup_visible(plugin_ui_popup_t popup) {
    return popup->m_page ? plugin_ui_page_visible(popup->m_page) : 0;
}

int plugin_ui_popup_set_data(plugin_ui_popup_t popup, dr_data_t data) {
    plugin_ui_env_t env = popup->m_env;
    void * page_data;

    if (popup->m_page == NULL) {
        CPE_ERROR(env->m_module->m_em, "popup: set data: page already destoried!");
        return -1;
    }
    
    page_data = plugin_ui_page_data(popup->m_page);
    if (page_data == NULL) {
        CPE_ERROR(env->m_module->m_em, "popup: page %s: set data: page no data!", plugin_ui_page_name(popup->m_page));
        return -1;
    }

    if (dr_meta_copy_same_entry_part(
            popup->m_page->m_page_data, popup->m_page->m_page_data_size, popup->m_page->m_page_data_meta,
            data->m_data, data->m_size, data->m_meta,
            NULL, 0, env->m_module->m_em)
        < 0)
    {
        CPE_ERROR(env->m_module->m_em, "popup: page %s: set data: copy data fail!", plugin_ui_page_name(popup->m_page));
        return -1;
    }

    popup->m_page->m_changed = 1;
    
    return 0;
}

int plugin_ui_popup_set_visible(plugin_ui_popup_t popup, uint8_t visible) {

    if (visible && visible != 1) visible = 1;
    
    if (plugin_ui_page_visible(popup->m_page) == visible) return 0;

    popup->m_env->m_visible_pages_need_update = 1;

    plugin_ui_page_update_visible(popup->m_page, visible);
    
    return 0;
}

plugin_ui_page_t plugin_ui_popup_create_from_page(plugin_ui_popup_t popup) {
    return popup->m_create_from_page;
}

void plugin_ui_popup_set_create_from_page(plugin_ui_popup_t popup, plugin_ui_page_t page) {
    if (popup->m_create_from_page) {
        TAILQ_REMOVE(&popup->m_create_from_page->m_created_popups, popup, m_next_for_create_from_page);
    }

    popup->m_create_from_page = page;

    if (popup->m_create_from_page) {
        TAILQ_INSERT_TAIL(&popup->m_create_from_page->m_created_popups, popup, m_next_for_create_from_page);
    }
}

void plugin_ui_popup_close_by_data_meta(plugin_ui_env_t env, LPDRMETA data_meta) {
    plugin_ui_popup_t popup;

    TAILQ_FOREACH(popup, &env->m_popups, m_next_for_env) {
        if (plugin_ui_page_data_meta(popup->m_page) == data_meta) {
            plugin_ui_popup_set_visible(popup, 0);
        }
    }
}
    
void plugin_ui_popup_close_from_page(plugin_ui_page_t page) {
    plugin_ui_popup_t popup;

    TAILQ_FOREACH(popup, &page->m_created_popups, m_next_for_create_from_page) {
        plugin_ui_popup_set_visible(popup, 0);
    }
}

void plugin_ui_popup_close_from_page_by_data_meta(plugin_ui_page_t page, LPDRMETA data_meta) {
    plugin_ui_popup_t popup;

    TAILQ_FOREACH(popup, &page->m_created_popups, m_next_for_create_from_page) {
        if (plugin_ui_page_data_meta(popup->m_page) == data_meta) {
            plugin_ui_popup_set_visible(popup, 0);
        }
    }
}

void plugin_ui_popup_close_from_page_by_name(plugin_ui_page_t page, const char * popup_name) {
    plugin_ui_popup_t popup;

    TAILQ_FOREACH(popup, &page->m_created_popups, m_next_for_create_from_page) {
        if (strcmp(popup->m_name, popup_name) == 0) {
            plugin_ui_popup_set_visible(popup, 0);
        }
    }
}

uint16_t plugin_ui_popup_trigger_action(plugin_ui_popup_t popup, const char * action_name) {
    plugin_ui_popup_action_t action;
    uint32_t processed_count = 0;

    TAILQ_FOREACH(action, &popup->m_actions, m_next) {
        if (strcmp(action->m_name, action_name) == 0) {
            action->m_fun(
                action->m_ctx ? action->m_ctx : action->m_data,
                popup, action_name);
            processed_count++;
        }
    }

    return processed_count;
}
