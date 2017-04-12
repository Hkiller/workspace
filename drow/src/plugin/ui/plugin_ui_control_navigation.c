#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_calc.h"
#include "plugin/ui/plugin_ui_control_calc.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_navigation_i.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_state_node_i.h"

int plugin_ui_control_trigger_navigation(plugin_ui_control_t control, dr_data_t data) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_phase_node_t cur_phase;
    plugin_ui_state_node_t state_node;

    cur_phase = plugin_ui_phase_node_current(env);
    if (cur_phase == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_trigger_navigation: no current phase!");
        return -1;
    }

    TAILQ_FOREACH_REVERSE(state_node, &cur_phase->m_state_stack, plugin_ui_state_node_list, m_next) {
        plugin_ui_navigation_t navigation = NULL;
        
        switch(state_node->m_state) {
        case plugin_ui_state_node_state_prepare_loading:
        case plugin_ui_state_node_state_loading:
            if (state_node->m_curent.m_loading_state) {
                navigation = plugin_ui_navigation_find_by_control(state_node->m_curent.m_loading_state, control, data);
            }

            if (navigation == NULL) {
                assert(state_node->m_curent.m_process_state);
                navigation = plugin_ui_navigation_find_by_control(state_node->m_curent.m_process_state, control, data);
            }
            break;
        case plugin_ui_state_node_state_processing:
            assert(state_node->m_curent.m_process_state);
            navigation = plugin_ui_navigation_find_by_control(state_node->m_curent.m_process_state, control, data);
            break;
        case plugin_ui_state_node_state_back:
            break;
        default:
            assert(0);
            break;
        }

        if (navigation) {
            return plugin_ui_navigation_execute(navigation, state_node, data);
        }
    }

    CPE_ERROR(
        env->m_module->m_em, "plugin_ui_control_trigger_navigation: control %s no trigger!",
        plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, control));
    
    return -1;
}

int plugin_ui_env_navigation(plugin_ui_env_t env, const char * path, dr_data_t data) {
    plugin_ui_phase_node_t cur_phase;
    plugin_ui_state_node_t state_node;

    cur_phase = plugin_ui_phase_node_current(env);
    if (cur_phase == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_trigger_navigation: no current phase!");
        return -1;
    }

    TAILQ_FOREACH_REVERSE(state_node, &cur_phase->m_state_stack, plugin_ui_state_node_list, m_next) {
        plugin_ui_navigation_t navigation = NULL;
        
        switch(state_node->m_state) {
        case plugin_ui_state_node_state_prepare_loading:
        case plugin_ui_state_node_state_loading:
            if (state_node->m_curent.m_loading_state) {
                navigation = plugin_ui_navigation_find_by_path(state_node->m_curent.m_loading_state, path, data);
            }

            if (navigation == NULL) {
                assert(state_node->m_curent.m_process_state);
                navigation = plugin_ui_navigation_find_by_path(state_node->m_curent.m_process_state, path, data);
            }
            break;
        case plugin_ui_state_node_state_prepare_resume:
        case plugin_ui_state_node_state_suspend:
        case plugin_ui_state_node_state_processing:
            assert(state_node->m_curent.m_process_state);
            navigation = plugin_ui_navigation_find_by_path(state_node->m_curent.m_process_state, path, data);
            break;
        case plugin_ui_state_node_state_prepare_back:
        case plugin_ui_state_node_state_back:
            break;
        default:
            assert(0);
            break;
        }

        if (navigation) {
            return plugin_ui_navigation_execute(navigation, state_node, data);
        }
    }

    CPE_ERROR(
        env->m_module->m_em, "plugin_ui_env_trigger_navigation: path %s no trigger!",
        path);
    
    return -1;
}

plugin_ui_navigation_t
plugin_ui_navigation_find_by_control(plugin_ui_state_t state, plugin_ui_control_t control, dr_data_t data) {
    plugin_ui_page_t page = control->m_page;
    plugin_ui_env_t env = page->m_env;
    plugin_ui_navigation_t navigation;
    const char * page_name = plugin_ui_page_name(page);
    size_t page_name_len = strlen(page_name);
    const char * control_name = plugin_ui_control_name(control);

    TAILQ_FOREACH(navigation, &state->m_navigations_to, m_next_for_from) {
        const char * control_begin;
        const char * name_begin;

        if (navigation->m_trigger_control == NULL) continue;
        if (!cpe_str_start_with(navigation->m_trigger_control, page_name)) continue;
        if (navigation->m_trigger_control[page_name_len] != '.') continue;

        control_begin =  navigation->m_trigger_control + page_name_len + 1;

        name_begin = strrchr(navigation->m_trigger_control, '.');
        assert(name_begin);

        if (!plugin_ui_control_name_is_eq_no_category(env, control_name, name_begin + 1)) continue;
        
        if (name_begin + 1 != control_begin) {
            char parent_path_buf[128];
            size_t parent_len = name_begin - control_begin;
            plugin_ui_control_t check_control = control;
            char * sep;
                
            if (parent_len + 1 > CPE_ARRAY_SIZE(parent_path_buf)) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_trigger_navigation: trigger %s parent path overflow!",
                    navigation->m_trigger_control);
                assert(0);
                continue;
            }

            memcpy(parent_path_buf, control_begin, parent_len);
            parent_path_buf[parent_len] = 0;

            while(check_control && (sep = strrchr(parent_path_buf, '.'))) {
                *sep = 0;
                //printf("check %s\n", sep + 1);
                check_control = plugin_ui_control_find_parent_by_name_no_category_r(check_control, sep + 1);
            }
                
            if (check_control == NULL) continue;
        }

        if (navigation->m_condition) {
            uint8_t result;
            dr_data_source_t data_source = NULL;
            struct dr_data_source data_source_buf;

            if (data) {
                data_source_buf.m_next = NULL;
                data_source_buf.m_data = *data;
                data_source = &data_source_buf;
            }

            if (plugin_ui_control_try_calc_bool(&result, navigation->m_condition, control, data_source, env->m_module->m_em) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_trigger_navigation: calc condition %s fail!",
                    navigation->m_condition);
                return NULL;
            }

            if (!result) continue;
        }

        return navigation;
    }

    return NULL;    
}

static char * plugin_ui_control_name_parent(char * control, const char * begin, char * * sep) {
    if (control <= begin) {
        if (*sep) (**sep) = '.';
        return NULL;
    }

    control--;
    assert(*control == '.');
    
    *sep = control;
    (**sep) = 0;
    
    while(control > begin) {
        if (*(control - 1) == '.') return control;
        control--;
    }

    return control;
}

plugin_ui_navigation_t
plugin_ui_navigation_find_by_path(plugin_ui_state_t state, const char * i_path, dr_data_t data) {
    plugin_ui_env_t env = state->m_phase->m_env;
    char * path;
    plugin_ui_navigation_t navigation;
    char * sep;
    const char * page_name;
    size_t page_name_len;
    char * control_name;

    mem_buffer_clear_data(gd_app_tmp_buffer(env->m_module->m_app));
    path = mem_buffer_strdup(gd_app_tmp_buffer(env->m_module->m_app), i_path);
    if (path == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_trigger_navigation: dup path %s fail!",
            i_path);
        return NULL;
    }

    sep = strchr(path, '.');
    if (sep == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_trigger_navigation: path %s format error(no page name sep)!", path);
        return NULL;
    }
    page_name = path; path = sep + 1; *sep = 0;
    page_name_len = strlen(page_name);

    sep = strrchr(path, '.');
    control_name = sep ? sep + 1 : path;
    
    TAILQ_FOREACH(navigation, &state->m_navigations_to, m_next_for_from) {
        const char * control_begin;
        const char * name_begin;

        if (navigation->m_trigger_control == NULL) continue;
        if (!cpe_str_start_with(navigation->m_trigger_control, page_name)) continue;
        if (navigation->m_trigger_control[page_name_len] != '.') continue;

        control_begin =  navigation->m_trigger_control + page_name_len + 1;

        name_begin = strrchr(navigation->m_trigger_control, '.');
        assert(name_begin);

        if (!plugin_ui_control_name_is_eq_no_category(env, control_name, name_begin + 1)) continue;
        
        if (name_begin + 1 != control_begin) {
            char parent_path_buf[128];
            size_t parent_len = name_begin - control_begin;
            char * check_control;
            char * check_sep;
            char * sep;
            
            if (parent_len + 1 > CPE_ARRAY_SIZE(parent_path_buf)) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_trigger_navigation: trigger %s parent path overflow!",
                    navigation->m_trigger_control);
                assert(0);
                continue;
            }

            memcpy(parent_path_buf, control_begin, parent_len);
            parent_path_buf[parent_len] = 0;

            check_sep = NULL;
            for(check_control = plugin_ui_control_name_parent(control_name, path, &check_sep);
                check_control && (sep = strrchr(parent_path_buf, '.'));
                check_control = plugin_ui_control_name_parent(check_control, path, &check_sep)
                )
            {
                *sep = 0;

                while(check_control) {
                    if (plugin_ui_control_name_is_eq_no_category(env, check_control, sep + 1)) break;
                    check_control = plugin_ui_control_name_parent(check_control, path, &check_sep);
                }
            }

            if (check_sep) *check_sep = '.';
            
            if (check_control == NULL) continue;
        }

        if (navigation->m_condition) {
            uint8_t result;
            dr_data_source_t data_source = NULL;
            struct dr_data_source data_source_buf;

            if (data) {
                data_source_buf.m_next = NULL;
                data_source_buf.m_data = *data;
                data_source = &data_source_buf;
            }

            if (dr_try_calc_bool(&result, env->m_module->m_computer, navigation->m_condition, data_source, env->m_module->m_em) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_trigger_navigation: calc condition %s fail!",
                    navigation->m_condition);
                return NULL;
            }

            if (!result) continue;
        }

        return navigation;
    }

    return NULL;    
}

