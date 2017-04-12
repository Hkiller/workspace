#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "render/model/ui_data_src_group.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_language.h"
#include "render/cache/ui_cache_group.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_queue.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_phase_i.h"
#include "plugin_ui_phase_use_page_i.h"
#include "plugin_ui_phase_use_popup_def_i.h"
#include "plugin_ui_state_i.h"
#include "plugin_ui_package_queue_using_i.h"

plugin_ui_phase_t plugin_ui_phase_create(plugin_ui_env_t env, const char * phase_name) {
    plugin_ui_phase_t phase;
    char package_name[64];
    
    if (env->m_backend == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_create: env no backend!");
        return NULL;
    }
    
    phase = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_phase) + env->m_backend->phase_capacity);
    if (phase == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_create: alloc fail!");
        return NULL;
    }

    phase->m_env = env;
    cpe_str_dup(phase->m_name, sizeof(phase->m_name), phase_name);
    phase->m_fps = 60;
    phase->m_init_state = NULL;
    phase->m_init_call_state = NULL;

    /*基础资源 */
    snprintf(package_name, sizeof(package_name), "phase/%s", phase_name);
    phase->m_package = plugin_package_package_find(env->m_module->m_package_module, package_name);
    if (phase->m_package == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_create: phase package not exist!");
        mem_free(env->m_module->m_alloc, phase);
        return NULL;
    }

    /**/
    if (cpe_hash_table_init(
            &phase->m_states,
            env->m_module->m_alloc,
            (cpe_hash_fun_t) plugin_ui_state_hash,
            (cpe_hash_eq_t) plugin_ui_state_eq,
            CPE_HASH_OBJ2ENTRY(plugin_ui_state, m_hh_for_phase),
            -1) != 0)
    {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_create: init state hashtable fail!");
        mem_free(env->m_module->m_alloc, phase);
        return NULL;
    }
    
    if (env->m_backend->phase_init(env->m_backend->ctx, env, phase) != 0) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_create: backend init phase fail!");
        cpe_hash_table_fini(&phase->m_states);
        mem_free(env->m_module->m_alloc, phase);
        return NULL;
    }

    TAILQ_INIT(&phase->m_using_package_queues);
    TAILQ_INIT(&phase->m_using_pages);
    TAILQ_INIT(&phase->m_using_popup_defs);

    TAILQ_INSERT_TAIL(&env->m_phases, phase, m_next_for_env);

    return phase;
}

void plugin_ui_phase_free(plugin_ui_phase_t phase) {
    plugin_ui_env_t env = phase->m_env;

    env->m_backend->phase_fini(env->m_backend->ctx, env, phase);
    
    /*resource*/
    plugin_ui_state_free_all(phase);
    cpe_hash_table_fini(&phase->m_states);

    if (env->m_init_phase == phase) env->m_init_phase = NULL;
    if (env->m_init_call_phase == phase) env->m_init_call_phase = NULL;

    while(!TAILQ_EMPTY(&phase->m_using_package_queues)) {
        plugin_ui_package_queue_using_free(TAILQ_FIRST(&phase->m_using_package_queues));
    }
    
    while(!TAILQ_EMPTY(&phase->m_using_pages)) {
        plugin_ui_phase_use_page_free(TAILQ_FIRST(&phase->m_using_pages));
    }

    while(!TAILQ_EMPTY(&phase->m_using_popup_defs)) {
        plugin_ui_phase_use_popup_def_free(TAILQ_FIRST(&phase->m_using_popup_defs));
    }

    TAILQ_REMOVE(&env->m_phases, phase, m_next_for_env);

    mem_free(env->m_module->m_alloc, phase);
}

plugin_ui_phase_t plugin_ui_phase_find(plugin_ui_env_t env, const char * phase_name) {
    plugin_ui_phase_t phase;

    TAILQ_FOREACH(phase, &env->m_phases, m_next_for_env) {
        if (strcmp(phase->m_name, phase_name) == 0) return phase;
    }

    return NULL;
}

const char * plugin_ui_phase_name(plugin_ui_phase_t phase) {
    return phase->m_name;
}

uint8_t plugin_ui_phase_fps(plugin_ui_phase_t phase) {
    return phase->m_fps;
}

void plugin_ui_phase_set_fps(plugin_ui_phase_t phase, uint8_t fps) {
    assert(fps > 0);
    phase->m_fps = fps;
}

void * plugin_ui_phase_data(plugin_ui_phase_t phase) {
    return phase + 1;
}

uint8_t plugin_ui_phase_is_use_page(plugin_ui_phase_t phase, plugin_ui_page_t page) {
    plugin_ui_phase_use_page_t use_page;

    TAILQ_FOREACH(use_page, &page->m_used_by_phases, m_next_for_page) {
        if (use_page->m_phase == phase) return 1;
    }

    return 0;
}

plugin_package_package_t plugin_ui_phase_package(plugin_ui_phase_t phase) {
    return phase->m_package;
}

int plugin_ui_phase_enter(plugin_ui_phase_t phase) {
    plugin_ui_env_t env = phase->m_env;
    plugin_ui_page_t page;
    plugin_ui_phase_use_page_t use_page;
    plugin_ui_phase_use_popup_def_t use_popup_def;    

    /*加载新的page */
    TAILQ_FOREACH(use_page, &phase->m_using_pages, m_next_for_phase) {
        page = use_page->m_page;
        if (page->m_load_policy != plugin_ui_page_load_policy_phase) continue;
            
        if (!plugin_ui_page_is_loaded(page) && page->m_src) {
            if (plugin_ui_page_load(page) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_phase %s: load page %s fail!",
                    plugin_ui_phase_name(phase), plugin_ui_page_name(page));
                return -1;
            }
            else {
                if (env->m_debug) {
                    CPE_INFO(
                        env->m_module->m_em, "plugin_ui_phase %s: load page %s success!",
                        plugin_ui_phase_name(phase), plugin_ui_page_name(page));
                }
            }
        }
    }
    
    /*加载popup_def*/
    TAILQ_FOREACH(use_popup_def, &phase->m_using_popup_defs, m_next_for_phase) {
        ui_data_src_t load_from =
            ui_data_src_find_by_path(
                env->m_module->m_data_mgr,
                use_popup_def->m_popup_def->m_page_load_from,
                ui_data_src_type_layout);
        if (load_from == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_phase %s: load popup def %s: src %s not exist!",
                plugin_ui_phase_name(phase),
                plugin_ui_popup_def_name(use_popup_def->m_popup_def),
                use_popup_def->m_popup_def->m_page_load_from);
            return -1;
        }
        
        if (!ui_data_src_is_loaded(load_from)) {
            if (ui_data_src_load(load_from, env->m_module->m_em) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_phase %s: load popup def %s fail!",
                    plugin_ui_phase_name(phase), plugin_ui_popup_def_name(use_popup_def->m_popup_def));
                return -1;
            }
            else {
                if (env->m_debug) {
                    CPE_INFO(
                        env->m_module->m_em, "plugin_ui_phase %s: load popup def %s success!",
                        plugin_ui_phase_name(phase), plugin_ui_popup_def_name(use_popup_def->m_popup_def));
                }
            }
        }
    }
    
    if (phase->m_env->m_backend->phase_enter(phase->m_env->m_backend->ctx, phase->m_env, phase) != 0) {
        return -1;
    }

    if (env->m_debug) {
        CPE_INFO(env->m_module->m_em, "plugin_ui_phase %s: enter success!", plugin_ui_phase_name(phase));
    }

    return 0;
}

void plugin_ui_phase_leave(plugin_ui_phase_t phase) {
    plugin_ui_env_t env = phase->m_env;

    env->m_backend->phase_leave(env->m_backend->ctx, env, phase);

    if (env->m_debug) {
        CPE_INFO(env->m_module->m_em, "plugin_ui_phase %s: leave success!", plugin_ui_phase_name(phase));
    }
}

int plugin_ui_phase_set_init_state(plugin_ui_phase_t phase, const char * init_state_name, const char * init_call_state_name) {
    plugin_ui_env_t env = phase->m_env;
    plugin_ui_state_t init_state;
    plugin_ui_state_t init_call_state;

    init_state = plugin_ui_state_find(phase, init_state_name);
    if (init_state == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_set_init_state: init state %s not exist!", init_state_name);
        return -1;
    }

    init_call_state = NULL;
    if (init_call_state_name) {
        init_call_state = plugin_ui_state_find(phase, init_call_state_name);
        if (init_call_state == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_set_init_state: init call state %s not exist!", init_call_state_name);
            return -1;
        }
    }

    phase->m_init_state = init_state;
    phase->m_init_call_state = init_call_state;

    return 0;
}

static plugin_ui_state_t plugin_ui_phase_state_next(struct plugin_ui_state_it * it) {
    cpe_hash_it_t * hs_it = (cpe_hash_it_t *)it->m_data;
    return cpe_hash_it_next(hs_it);
}

void plugin_ui_phase_states(plugin_ui_phase_t phase, plugin_ui_state_it_t it) {
    cpe_hash_it_init((cpe_hash_it_t *)(it->m_data), &phase->m_states);
    it->next = plugin_ui_phase_state_next;
}
