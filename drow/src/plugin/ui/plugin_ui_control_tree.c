#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_layout.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_control_calc_i.h"
#include "plugin_ui_control_action_i.h"
#include "plugin_ui_control_action_slots_i.h"
#include "plugin_ui_touch_track_i.h"
#include "plugin_ui_utils_i.h"

plugin_ui_control_t plugin_ui_control_parent(plugin_ui_control_t control) {
    return control->m_parent;
}

uint16_t plugin_ui_control_child_count(plugin_ui_control_t control) {
    return control->m_child_count;
}

static plugin_ui_control_t plugin_ui_control_child_next(struct plugin_ui_control_it * it) {
    plugin_ui_control_t * data = (plugin_ui_control_t *)(it->m_data);
    plugin_ui_control_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_parent);

    return r;
}

void plugin_ui_control_childs(plugin_ui_control_t control, plugin_ui_control_it_t child_it) {
    *(plugin_ui_control_t *)(child_it->m_data) = TAILQ_FIRST(&control->m_childs);
    child_it->next = plugin_ui_control_child_next;
}

static plugin_ui_control_t plugin_ui_control_child_next_reverse(struct plugin_ui_control_it * it) {
    plugin_ui_control_t * data = (plugin_ui_control_t *)(it->m_data);
    plugin_ui_control_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_PREV(r, plugin_ui_control_list, m_next_for_parent);

    return r;
}

void plugin_ui_control_childs_reverse(plugin_ui_control_t control, plugin_ui_control_it_t child_it) {
    *(plugin_ui_control_t *)(child_it->m_data) = TAILQ_LAST(&control->m_childs, plugin_ui_control_list);
    child_it->next = plugin_ui_control_child_next_reverse;
}

void plugin_ui_control_add_child_tail(plugin_ui_control_t parent, plugin_ui_control_t child) {
    plugin_ui_page_t page = parent->m_page;

    cpe_ba_set(&parent->m_cache_flag, plugin_ui_control_cache_flag_layout, cpe_ba_true);
    
    assert(child->m_page == page);
    
    if (child->m_parent) {
        child->m_parent->m_child_count--;
        TAILQ_REMOVE(&child->m_parent->m_childs, child, m_next_for_parent);
    }
    else {
        TAILQ_REMOVE(&child->m_page->m_building_controls, child, m_next_for_parent);
    }

    if (child->m_template) {
        plugin_ui_control_adj_by_new_parent(child, parent);
    }

    TAILQ_INSERT_TAIL(&parent->m_childs, child, m_next_for_parent);
    child->m_parent = parent;
    parent->m_child_count++;
}

void plugin_ui_control_add_child_head(plugin_ui_control_t parent, plugin_ui_control_t child) {
    plugin_ui_page_t page = parent->m_page;

    cpe_ba_set(&parent->m_cache_flag, plugin_ui_control_cache_flag_layout, cpe_ba_true);
    
    assert(child->m_page == page);
    
    if (child->m_parent) {
        child->m_parent->m_child_count--;
        TAILQ_REMOVE(&child->m_parent->m_childs, child, m_next_for_parent);
    }
    else {
        TAILQ_REMOVE(&child->m_page->m_building_controls, child, m_next_for_parent);
    }

    if (child->m_template) {
        plugin_ui_control_adj_by_new_parent(child, parent);
    }

    TAILQ_INSERT_HEAD(&parent->m_childs, child, m_next_for_parent);
    child->m_parent = parent;
    parent->m_child_count++;
}

int plugin_ui_control_adj_before(plugin_ui_control_t control, plugin_ui_control_t before_control) {
    assert(before_control);
    assert(control);
    assert(control != before_control);
    
    if (control->m_parent != before_control->m_parent) {
        CPE_ERROR(
            control->m_page->m_env->m_module->m_em,
            "plugin_ui_control_adj_before: control parent mismatch");
        return -1;
    }

    TAILQ_REMOVE(&control->m_parent->m_childs, control, m_next_for_parent);
    TAILQ_INSERT_AFTER(&control->m_parent->m_childs, before_control, control, m_next_for_parent);

    return 0;
}

int plugin_ui_control_adj_after(plugin_ui_control_t control, plugin_ui_control_t after_control) {
    assert(after_control);
    assert(control);
    assert(control != after_control);
    
    if (control->m_parent != after_control->m_parent) {
        CPE_ERROR(
            control->m_page->m_env->m_module->m_em,
            "plugin_ui_control_adj_after: control parent mismatch");
        return -1;
    }

    TAILQ_REMOVE(&control->m_parent->m_childs, control, m_next_for_parent);
    TAILQ_INSERT_BEFORE(after_control, control, m_next_for_parent);

    return 0;
}

void plugin_ui_control_adj_top(plugin_ui_control_t control){
	TAILQ_REMOVE(&control->m_parent->m_childs, control, m_next_for_parent);
	TAILQ_INSERT_HEAD(&control->m_parent->m_childs, control, m_next_for_parent);
}

void plugin_ui_control_adj_tail(plugin_ui_control_t control){
	TAILQ_REMOVE(&control->m_parent->m_childs, control, m_next_for_parent);
	TAILQ_INSERT_TAIL(&control->m_parent->m_childs, control, m_next_for_parent);
}

void plugin_ui_control_remove_child(plugin_ui_control_t parent, plugin_ui_control_t child) {
    assert(child->m_parent == parent);

    cpe_ba_set(&parent->m_cache_flag, plugin_ui_control_cache_flag_layout, cpe_ba_true);

    plugin_ui_control_evt_on_control_remove(parent->m_page->m_env, child);

    parent->m_child_count--;
    TAILQ_REMOVE(&parent->m_childs, child, m_next_for_parent);
    
    child->m_parent = NULL;
    TAILQ_INSERT_TAIL(&child->m_page->m_building_controls, child, m_next_for_parent);
}

void plugin_ui_control_remove_childs(plugin_ui_control_t parent) {
    while(!TAILQ_EMPTY(&parent->m_childs)) {
        plugin_ui_control_remove_child(parent, TAILQ_FIRST(&parent->m_childs));
    }
}

void plugin_ui_control_destory_childs(plugin_ui_control_t parent) {
    while(!TAILQ_EMPTY(&parent->m_childs)) {
        plugin_ui_control_free(TAILQ_FIRST(&parent->m_childs));
    }
}

plugin_ui_control_t plugin_ui_control_find_parent_by_name(plugin_ui_control_t control, const char * name) {
    if (control->m_parent == NULL || control->m_parent->m_parent == NULL) return NULL;

    if (strcmp(plugin_ui_control_name(control->m_parent), name) == 0) return control->m_parent;

    return NULL;
}

plugin_ui_control_t plugin_ui_control_find_parent_by_name_r(plugin_ui_control_t control, const char * name) {
    plugin_ui_control_t p;

    for(p = control->m_parent; p && p->m_parent; p = p->m_parent) {
        if (strcmp(plugin_ui_control_name(p), name) == 0) return p;
    }

    return NULL;
}

plugin_ui_control_t plugin_ui_control_find_parent_by_name_no_category(plugin_ui_control_t control, const char * name) {
    if (control->m_parent == NULL || control->m_parent->m_parent == NULL) return NULL;

    if (plugin_ui_control_is_name_eq_no_category(control->m_parent, name)) return control->m_parent;

    return NULL;
}

plugin_ui_control_t plugin_ui_control_find_parent_by_name_no_category_r(plugin_ui_control_t control, const char * name) {
    plugin_ui_control_t p;

    for(p = control->m_parent; p && p->m_parent; p = p->m_parent) {
        if (plugin_ui_control_is_name_eq_no_category(p, name)) return p;
    }

    return NULL;
}

plugin_ui_control_t plugin_ui_control_find_child_by_name(plugin_ui_control_t control, const char * name) {
    plugin_ui_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
		if (strcmp(plugin_ui_control_name(child), name) == 0) return child;
    }

	return NULL;
}

plugin_ui_control_t plugin_ui_control_find_child_by_name_r(plugin_ui_control_t control, const char * name) {
    plugin_ui_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_t r;
        
		if (strcmp(plugin_ui_control_name(child), name) == 0) return child;

        if ((r = plugin_ui_control_find_child_by_name_r(child, name))) return r;
    }

	return NULL;
}

plugin_ui_control_t plugin_ui_control_find_child_by_name_no_category(plugin_ui_control_t control, const char * name) {
    plugin_ui_control_t child;

    if (name[0] == 0) return control;
    if (strcmp(name, "^") == 0) return control->m_parent;
    
    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
		if (plugin_ui_control_is_name_eq_no_category(child, name)) return child;
    }

	return NULL;
}

plugin_ui_control_t plugin_ui_control_find_child_by_name_no_category_r(plugin_ui_control_t control, const char * name) {
    plugin_ui_control_t child;

    if (name[0] == 0) return control;
    if (strcmp(name, "^") == 0) return control->m_parent;
    
    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_t r;
        
		if (plugin_ui_control_is_name_eq_no_category(child, name)) return child;

        if ((r = plugin_ui_control_find_child_by_name_no_category_r(child, name))) return r;
    }

	return NULL;
}

plugin_ui_control_t
plugin_ui_control_find_child_by_attr(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value) {
    plugin_ui_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        if (plugin_ui_control_is_attr_eq(child, attr_name, attr_value)) return child;
    }

	return NULL;
}

plugin_ui_control_t
plugin_ui_control_find_child_by_attr_r(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value) {
    plugin_ui_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_t r;

        if (plugin_ui_control_is_attr_eq(child, attr_name, attr_value)) return child;

        if ((r = plugin_ui_control_find_child_by_attr_r(child, attr_name, attr_value))) return r;
    }

	return NULL;
}

static uint8_t
plugin_ui_control_is_condition_match(plugin_ui_control_t control, const char * name, const char * condition) {
    if (name) {
        if (!plugin_ui_control_is_name_eq_no_category(control, name)) return 0;
    }

    if (condition) {
        if (!plugin_ui_control_calc_bool_with_dft(condition, control, NULL, 0)) return 0;
    }

    return 1;
}

plugin_ui_control_t
plugin_ui_control_find_child_by_condition(plugin_ui_control_t control, const char * name, const char * condition) {
    plugin_ui_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        if (plugin_ui_control_is_condition_match(child, name, condition)) return child;
    }

	return NULL;
}

plugin_ui_control_t
plugin_ui_control_find_child_by_condition_r(plugin_ui_control_t control, const char * name, const char * condition) {
    plugin_ui_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_t r;

        if (plugin_ui_control_is_condition_match(child, name, condition)) return child;
        
        if ((r = plugin_ui_control_find_child_by_condition_r(child, name, condition))) return r;
    }

	return NULL;
}

plugin_ui_control_t
plugin_ui_control_find_child_by_attr_str(plugin_ui_control_t control, const char * attr_name, const char * attr_value) {
    struct dr_value v;

    v.m_type = CPE_DR_TYPE_STRING;
    v.m_meta = NULL;
    v.m_data = (void*)attr_value;
    v.m_size = strlen(attr_value) + 1;

    return plugin_ui_control_find_child_by_attr(control, attr_name, &v);
}

plugin_ui_control_t
plugin_ui_control_find_child_by_attr_str_r(plugin_ui_control_t control, const char * attr_name, const char * attr_value) {
    struct dr_value v;

    v.m_type = CPE_DR_TYPE_STRING;
    v.m_meta = NULL;
    v.m_data = (void*)attr_value;
    v.m_size = strlen(attr_value) + 1;

    return plugin_ui_control_find_child_by_attr_r(control, attr_name, &v);
}

static plugin_ui_control_t
plugin_ui_control_find_by_condition_range(plugin_ui_env_t env, plugin_ui_control_t control, const char * name, const char * name_end) {
    char * buf;
    char * condition_begin;
    char * condition_end;
    
    mem_buffer_clear_data(&env->m_module->m_dump_buffer);
    buf = mem_buffer_strdup_range(&env->m_module->m_dump_buffer, name, name_end);
    if (buf == NULL) {
        CPE_ERROR(
            env->m_module->m_em,
            "plugin_ui_control_find_by_path: alloc name overflow, name=%s", name);
        return NULL;
    }

    if ((condition_begin = strchr(buf, '['))) {
        condition_end = strchr(condition_begin + 1, ']');
        if (buf == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_find_by_path: condition format error, str=%s", buf);
            return NULL;
        }

        *condition_begin = 0;
        *condition_end = 0;

        //printf("xxxxxxx: name=%s, condition=%s\n", buf, condition_begin + 1);
        return plugin_ui_control_find_child_by_condition_r(control, buf, condition_begin + 1);
    }
    else {
        return plugin_ui_control_find_child_by_name_no_category_r(control, buf);
    }
}

plugin_ui_control_t
plugin_ui_control_find_child_by_path(plugin_ui_control_t control, const char * path) {
    plugin_ui_env_t env = control->m_page->m_env;
    const char * sep;
    char * condition_begin = strchr(path, '[');
    char * condition_end = condition_begin ? strchr(condition_begin + 1, ']') : NULL;

    if (condition_begin) {
        if (condition_end == NULL || (*(condition_end + 1) != '.' && *(condition_end + 1) != 0)) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_find_by_path: path condition format error, path=%s", path);
            return NULL;
        }
    }

    while((sep = strchr(path, '.'))) {
        if (condition_begin && sep > condition_begin) {
            if (sep < condition_end) sep = condition_end + 1;
        }
        
        control = plugin_ui_control_find_by_condition_range(env, control, path, sep);
        if (control == NULL) return NULL;

        path = sep + 1;

        if (condition_begin && condition_begin < path) {
            condition_begin = strchr(sep + 1, '[');
            condition_end = condition_begin ? strchr(condition_begin + 1, ']') : NULL;
            if (condition_begin) {
                if (condition_end == NULL || (*(condition_end + 1) != '.' && *(condition_end + 1) != 0)) {
                    CPE_ERROR(env->m_module->m_em, "plugin_ui_control_find_by_path: path condition format error, path=%s", path);
                    return NULL;
                }
            }
        }
    }

    if (condition_begin == NULL) {
        control = plugin_ui_control_find_child_by_name_no_category_r(control, path);
    }
    else {
        control = plugin_ui_control_find_by_condition_range(env, control, path, path + strlen(path));
    }
    
    if (control == NULL) return NULL;

    return control;
}

plugin_ui_control_t
plugin_ui_control_find_by_path(plugin_ui_env_t env, const char * path) {
    plugin_ui_page_t page;
    const char * p;
    
    p = strchr(path, '.');
    if (p == NULL) {
        page = plugin_ui_page_find(env, path);
        if (page == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_find_by_path: page %s not exist", path);
            return NULL;
        }
        return &page->m_control;
    }
    else {
        char page_name[64];
        size_t len;

        len = p - path;
        if (len + 1 > CPE_ARRAY_SIZE(page_name)) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_find_by_path: page name len overflow, path=%s", path);
            return NULL;
        }
    
        memcpy(page_name, path, len);
        page_name[len] = 0;

        page = plugin_ui_page_find(env, page_name);
        if (page == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_find_by_path: page %s not exist", page_name);
            return NULL;
        }

        return plugin_ui_control_find_child_by_path(&page->m_control, p + 1);
    }
}

void plugin_ui_control_visit_tree(
    plugin_ui_control_t input,
    enum plugin_ui_control_visit_tree_policy policy,
    void * ctx, int (*fun)(plugin_ui_control_t c, void * ctx), uint8_t include_self)
{
    plugin_ui_control_t stack[256];
    int16_t stack_pos = -1;

    if (policy == plugin_ui_control_visit_tree_bfs) {
        plugin_ui_control_t child;
        if (include_self) {
            stack[++stack_pos] = input;
        }
        else {
            TAILQ_FOREACH(child, &input->m_childs, m_next_for_parent) {
                assert(stack_pos + 1 < CPE_ARRAY_SIZE(stack));
                stack[++stack_pos] = child;
            }
        }

        while(stack_pos >= 0) {
            int process_child;
            plugin_ui_control_t cur;

            assert(stack_pos < CPE_ARRAY_SIZE(stack));
        
            cur = stack[stack_pos--];

            process_child = fun(cur, ctx);
            if (process_child) {
                TAILQ_FOREACH(child, &cur->m_childs, m_next_for_parent) {
                    assert(stack_pos + 1 < CPE_ARRAY_SIZE(stack));
                    stack[++stack_pos] = child;
                }
            }
        }
    }
    else {
        assert(policy == plugin_ui_control_visit_tree_dfs);

        if (include_self) {
            stack[++stack_pos] = input;
        }

        while((input = TAILQ_FIRST(&input->m_childs))) {
            assert(stack_pos + 1 < CPE_ARRAY_SIZE(stack));
            stack[++stack_pos] = input;
        }

        while(stack_pos >= 0) {
            int process_child;
            plugin_ui_control_t cur;

            assert(stack_pos < CPE_ARRAY_SIZE(stack));
        
            cur = stack[stack_pos--];

            process_child = fun(cur, ctx);

            if ((stack_pos > 0 || !include_self) /*不是根节点 */
                && process_child)
            {
                plugin_ui_control_t next;
                for(next = TAILQ_NEXT(cur, m_next_for_parent); next; next = TAILQ_FIRST(&next->m_childs)) {
                    assert(stack_pos + 1 < CPE_ARRAY_SIZE(stack));
                    stack[++stack_pos] = next;
                }
            }
        }
    }
}

void plugin_ui_control_dispatch_event(
    plugin_ui_control_t at, plugin_ui_control_t from, plugin_ui_event_t evt, plugin_ui_event_dispatch_scope_t scope)
{
    plugin_ui_page_t page = from->m_page;
    plugin_ui_control_t p;
    struct plugin_ui_control_meta_event_slot * slot;
    uint8_t slot_pos;
    const char * from_name = plugin_ui_control_name(from);
    uint8_t page_tag_local = 0;
    uint8_t at_tag_local = 0;
    uint8_t from_tag_local = 0;
    
    assert(at);
    assert(from);

    assert(!page->m_control.m_is_free);
    assert(!at->m_is_free);
    assert(!from->m_is_free);

    if (!page->m_control.m_is_processing) {
        page->m_control.m_is_processing = 1;
        page_tag_local = 1;
    }
    
    if (!at->m_is_processing) {
        at->m_is_processing = 1;
        at_tag_local = 1;
    }

    if (!from->m_is_processing) {
        from->m_is_processing = 1;
        from_tag_local = 1;
    }
    
    slot_pos = evt - plugin_ui_event_min;

    if (at == from) plugin_ui_control_play_event_sfx(at, evt);

    /*给全局处理器发送 */
    plugin_ui_env_dispatch_event(from->m_page->m_env, from, evt);
    if (from->m_is_free || at->m_is_free || page->m_control.m_is_free) goto COMPLETE;
    
    /*给自己发送 */
    if (scope & plugin_ui_event_dispatch_to_self) {
        slot = at->m_meta->m_event_slots + slot_pos;
        if (slot->m_fun && slot->m_scope & plugin_ui_event_scope_self) {
            slot->m_fun(at, from, evt);
            if (from->m_is_free || at->m_is_free || page->m_control.m_is_free) goto COMPLETE;
        }

        if (at->m_action_slots) {
            plugin_ui_control_action_list_t * action_list = &at->m_action_slots->m_actions[slot_pos];
            plugin_ui_control_action_t action;

            TAILQ_FOREACH(action, action_list, m_next) {
                if (action->m_scope & plugin_ui_event_scope_self) {
                    if (action->m_name_prefix == NULL || cpe_str_start_with(from_name, action->m_name_prefix)) {
                        action->m_fun(action->m_ctx ? action->m_ctx : action->m_data, from, evt);
                        if (from->m_is_free || at->m_is_free || page->m_control.m_is_free) goto COMPLETE;
                    }
                }
            }
        }
    }    

    /*向父控件发送 */
    if (scope & plugin_ui_event_dispatch_to_parent) {
        for(p = at->m_parent; p; p = p->m_parent) {
            uint8_t p_tag_local = 0;

            assert(!p->m_is_free);
            if (!p->m_is_processing) {
                p->m_is_processing = 1;
                p_tag_local = 1;
            }
            
            slot = p->m_meta->m_event_slots + slot_pos;
            if (slot->m_fun && slot->m_scope & plugin_ui_event_scope_childs) {
                slot->m_fun(p, from, evt);
                if (p->m_is_free) goto P_COMPLETE;
                if (from->m_is_free || at->m_is_free || page->m_control.m_is_free) goto P_COMPLETE;
            }

            if (p->m_action_slots) {
                plugin_ui_control_action_list_t * action_list = &p->m_action_slots->m_actions[slot_pos];
                plugin_ui_control_action_t action;

                TAILQ_FOREACH(action, action_list, m_next) {
                    if (action->m_scope & plugin_ui_event_scope_childs) {
                        if (action->m_name_prefix == NULL || cpe_str_start_with(from_name, action->m_name_prefix)) {
                            action->m_fun(action->m_ctx ? action->m_ctx : action->m_data, from, evt);
                            if (p->m_is_free) goto P_COMPLETE;
                            if (from->m_is_free || at->m_is_free || page->m_control.m_is_free) goto P_COMPLETE;
                        }
                    }
                }
            }

        P_COMPLETE:
            if (p_tag_local) {
                p->m_is_processing = 0;
                if (p->m_is_free) plugin_ui_control_free(p);
            }
            
            if (from->m_is_free || at->m_is_free || page->m_control.m_is_free) goto COMPLETE;
        }
    }

COMPLETE:
    if (from_tag_local) {
        from->m_is_processing = 0;
        if (from->m_is_free) plugin_ui_control_free(from);
    }

    if (at_tag_local) {
        at->m_is_processing = 0;
        if (at->m_is_free) plugin_ui_control_free(at);
    }

    if (page_tag_local) {
        page->m_control.m_is_processing = 0;
        if (page->m_control.m_is_free) plugin_ui_page_free(page);
    }
}

plugin_ui_control_t plugin_ui_control_find_child_by_pt(plugin_ui_control_t control, ui_vector_2_t pt) {
    plugin_ui_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
		if (plugin_ui_control_contain_test(child, pt)) return child;
    }

	return NULL;
}

plugin_ui_control_t plugin_ui_control_find_child_by_pt_r(plugin_ui_control_t control, ui_vector_2_t pt) {
    plugin_ui_control_t child;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_t r;
        
        if ((r = plugin_ui_control_find_child_by_pt_r(child, pt))) return r;

		if (plugin_ui_control_contain_test(child, pt)) return child;
    }

	return NULL;
}    

plugin_ui_control_t plugin_ui_control_find_click(plugin_ui_control_t control, ui_vector_2_t pt) {
    plugin_ui_control_t child;

    if (!plugin_ui_control_visible(control)) return NULL;
    if (!plugin_ui_control_enable(control)) return NULL;
    
    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_t r;
        if ((r = plugin_ui_control_find_click(child, pt))) return r;
    }

    if (!plugin_ui_control_accept_click(control)) return NULL;
    
    if (plugin_ui_control_contain_test(control, pt)) return control;
    
	return NULL;
}

plugin_ui_control_t plugin_ui_control_find_float(plugin_ui_control_t control, ui_vector_2_t pt) {
    plugin_ui_control_t child;

    if (!plugin_ui_control_visible(control)) return NULL;
    if (!plugin_ui_control_enable(control)) return NULL;
    
    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_t r;
        if ((r = plugin_ui_control_find_float(child, pt))) return r;
    }

    if (!plugin_ui_control_accept_float(control)) return NULL;
    
    if (plugin_ui_control_contain_test(control, pt)) return control;
    
	return NULL;
}

uint8_t plugin_ui_control_has_catch(plugin_ui_control_t control) {
    plugin_ui_touch_track_t track;
    plugin_ui_env_t env = control->m_page->m_env;

    TAILQ_FOREACH(track, &env->m_touch_tracks, m_next_for_env) {
        if (track->m_catch_control == control) return 1;
    }
    
    return 0;
}

uint8_t plugin_ui_control_is_child_of_r(plugin_ui_control_t self, plugin_ui_control_t check_parent) {
    plugin_ui_control_t p;

    for(p = self->m_parent; p; p = p->m_parent) {
        if (p == check_parent) return 1;
    }

    return 0;
}

uint8_t plugin_ui_control_has_catch_r(plugin_ui_control_t control) {
    plugin_ui_touch_track_t track;
    plugin_ui_env_t env = control->m_page->m_env;

    TAILQ_FOREACH(track, &env->m_touch_tracks, m_next_for_env) {
        if (track->m_catch_control == NULL) continue;

        if (track->m_catch_control == control
            || plugin_ui_control_is_child_of_r(track->m_catch_control, control))
        {
            return 1;
        }
    }
    
    return 0;
}    
