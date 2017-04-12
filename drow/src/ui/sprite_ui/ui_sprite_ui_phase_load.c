#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_language.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_group.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/ui/plugin_ui_package_queue_managed.h"
#include "plugin/ui/plugin_ui_package_queue_using.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_ui_phase_i.h"
#include "ui_sprite_ui_state_i.h"
#include "ui_sprite_ui_navigation_i.h"

static int ui_sprite_ui_env_phase_load_using_package_queues(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, cfg_t cfg);
static int ui_sprite_ui_env_phase_load_states(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase);
static int ui_sprite_ui_env_phase_load_navigations_one(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, cfg_t cfg);

int ui_sprite_ui_env_phase_load_navigations(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, cfg_t cfg) {
    struct cfg_it child_cfg_it;
    cfg_t child_cfg;
    int rv = 0;
    
    cfg_it_init(&child_cfg_it, cfg);
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        if (strcmp(cfg_name(child_cfg), "runing-fsm") != 0 && !cpe_str_start_with(cfg_name(child_cfg), "runing-fsm-")) continue;
        
        if (ui_sprite_ui_env_phase_load_navigations_one(env, b_phase, child_cfg) != 0) rv = -1;
    }

    return rv;
}

int ui_sprite_ui_env_phase_load_without_navigations(
    ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, ui_sprite_cfg_loader_t loader, cfg_t cfg)
{
    ui_sprite_world_t world = ui_sprite_entity_world(env->m_entity);
    ui_sprite_ui_phase_t phase = plugin_ui_phase_data(b_phase);
    char fsm_proto_name[128];
    struct cfg_it child_cfg_it;
    cfg_t child_cfg;
    int rv = 0;

    plugin_ui_phase_set_fps(b_phase, cfg_get_float(cfg, "fps", plugin_ui_phase_fps(b_phase)));

    if (ui_sprite_ui_env_phase_load_using_package_queues(env, b_phase, cfg_find_cfg(cfg, "using-package-queues")) != 0) rv = -1;
    
    snprintf(fsm_proto_name, sizeof(fsm_proto_name), "ui.phase.%s.runing", plugin_ui_phase_name(b_phase));
    
    phase->m_fsm = ui_sprite_fsm_proto_create(world, fsm_proto_name);
    if (phase->m_fsm == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: creeate fsm protocol fail!",
            plugin_ui_phase_name(b_phase));
        return -1;
    }

    cfg_it_init(&child_cfg_it, cfg);
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        const char * cfg_group_name = cfg_name(child_cfg);
        plugin_package_package_t package = NULL;
        struct cfg_it fsm_state_it;
        cfg_t fsm_state;
        
        if (strcmp(cfg_group_name, "runing-fsm") != 0 && !cpe_str_start_with(cfg_group_name, "runing-fsm-")) continue;

        if (cpe_str_start_with(cfg_group_name, "runing-fsm-")) {
            char state_package_name[64];
            snprintf(state_package_name, sizeof(state_package_name), "phase/%s-%s", plugin_ui_phase_name(b_phase), cfg_group_name + strlen("runing-fsm-"));
            package = plugin_package_package_find(plugin_ui_env_package_mgr(env->m_env), state_package_name);
        }

        cfg_it_init(&fsm_state_it, child_cfg);
        while((fsm_state = cfg_it_next(&fsm_state_it))) {
            plugin_ui_state_t b_state = plugin_ui_state_create(b_phase, cfg_name(fsm_state));
            ui_sprite_ui_state_t state;
            
            if (b_state == NULL) {
                CPE_ERROR(
                    env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: load state %s fail",
                    plugin_ui_phase_name(b_phase), cfg_name(fsm_state));
                rv = -1;
                continue;
            }

            state = plugin_ui_state_data(b_state);

            if (package) {
                plugin_package_group_t package_group = plugin_ui_state_packages_check_create(b_state);
                if (package_group == NULL) {
                    CPE_ERROR(
                        env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: load state %s: create package group fail",
                        plugin_ui_phase_name(b_phase), cfg_name(fsm_state));
                    rv = -1;
                }
                else {
                    if (plugin_package_group_add_package(package_group, package) != 0) {
                        CPE_ERROR(
                            env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: load state %s: add package %s fail",
                            plugin_ui_phase_name(b_phase), cfg_name(fsm_state), plugin_package_package_name(package));
                        rv = -1;
                    }
                }
            }
        }
        
        if (ui_sprite_cfg_load_fsm(loader, phase->m_fsm, child_cfg) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: load fsm from %s fail!",
                plugin_ui_phase_name(b_phase), cfg_name(child_cfg));
            rv = -1;
        }
    }
        
    if (ui_sprite_ui_env_phase_load_states(env, b_phase) != 0) {
        rv = -1;
    }
    
    return rv;
}

static int ui_sprite_ui_env_phase_load_states(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase) {
    int rv = 0;
    ui_sprite_ui_phase_t phase = plugin_ui_phase_data(b_phase);
    struct ui_sprite_fsm_state_it state_it;
    ui_sprite_fsm_state_t fsm_state;
    const char * init_state;
    const char * init_call_state;

    ui_sprite_fsm_ins_states(&state_it, phase->m_fsm);

    while((fsm_state = ui_sprite_fsm_state_it_next(&state_it))) {
        plugin_ui_state_t b_state = plugin_ui_state_find(b_phase, ui_sprite_fsm_state_name(fsm_state));
        ui_sprite_ui_state_t state;

        if (b_state == NULL) {
            rv = -1;
            continue;
        }
        
        state = plugin_ui_state_data(b_state);
        state->m_fsm_state = fsm_state;

        if (plugin_ui_env_debug(env->m_env)) {
            CPE_INFO(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: load state %s",
                plugin_ui_phase_name(b_phase), plugin_ui_state_name(b_state));
        }
    }

    init_state = ui_sprite_fsm_state_name(ui_sprite_fsm_default_state(phase->m_fsm));
    init_call_state =
        ui_sprite_fsm_default_call_state(phase->m_fsm)
        ? ui_sprite_fsm_state_name(ui_sprite_fsm_default_call_state(phase->m_fsm))
        : NULL;

    if (plugin_ui_phase_set_init_state(b_phase, init_state, init_call_state) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: set init state %s (loading=%s) fail",
            plugin_ui_phase_name(b_phase), init_state, init_call_state ? init_call_state : "");
        rv = -1;
    }

    return rv;
}

static plugin_ui_navigation_t ui_sprite_ui_env_phase_load_navigation_state(
    ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, plugin_ui_state_t b_state, cfg_t navigation_cfg,
    plugin_ui_navigation_state_op_t state_op, plugin_ui_renter_policy_t renter_policy, const char * to)
{
    plugin_ui_navigation_t b_navigation = NULL;
    plugin_ui_state_t to_state;
    const char * str_value;
    
    if (to) {
        to_state = plugin_ui_state_find(b_phase, to);
        if (to_state == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: state: phae %s: to state %s not exist",
                plugin_ui_phase_name(b_phase), to);
            return NULL;
        }
    }
    else {
        to_state = NULL;
    }

    b_navigation = plugin_ui_navigation_state_create(b_state, to_state, state_op, renter_policy);
    if (b_navigation == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: state: phae %s: create navigation fail",
            plugin_ui_phase_name(b_phase));
        return NULL;
    }

    if ((str_value = cfg_get_string(navigation_cfg, "enter", NULL))) {
        plugin_ui_state_t loading_state = plugin_ui_state_find(b_phase, str_value);
        if (loading_state == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: state: phae %s: loading state %s not exist",
                plugin_ui_phase_name(b_phase), str_value);
            return NULL;
        }

        plugin_ui_navigation_state_set_loading(b_navigation, loading_state);
    }
            
    if ((str_value = cfg_get_string(navigation_cfg, "leave", NULL))) {
        plugin_ui_state_t back_state = plugin_ui_state_find(b_phase, str_value);
        if (back_state == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: state: phae %s: back state %s not exist",
                plugin_ui_phase_name(b_phase), str_value);
            return NULL;
        }
        plugin_ui_navigation_state_set_back(b_navigation, back_state);
    }

    plugin_ui_navigation_state_set_suspend(b_navigation, cfg_get_uint8(navigation_cfg, "suspend", 0));
    plugin_ui_navigation_state_set_base_policy(b_navigation, cfg_get_uint8(navigation_cfg, "base-policy", 0));

    return b_navigation;
}

static plugin_ui_navigation_t ui_sprite_ui_env_phase_load_navigation_phase(
    ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, plugin_ui_state_t b_state, cfg_t navigation_cfg,
    plugin_ui_navigation_phase_op_t phase_op, plugin_ui_renter_policy_t renter_policy, const char * to)
{
    plugin_ui_navigation_t b_navigation = NULL;
    plugin_ui_phase_t to_phase;
    const char * str_value;

    if (to) {
        to_phase = plugin_ui_phase_find(env->m_env, to);
        if (to_phase == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phase: phae %s: to phase %s not exist",
                plugin_ui_phase_name(b_phase), to);
            return NULL;
        }
    }
    else {
        to_phase = NULL;
    }

    b_navigation = plugin_ui_navigation_phase_create(b_state, to_phase, phase_op, renter_policy);
    if (b_navigation == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phase: phae %s: create navigation fail",
            plugin_ui_phase_name(b_phase));
        return NULL;
    }

    if ((str_value = cfg_get_string(navigation_cfg, "phase-enter", NULL))) {
        uint8_t auto_complete = 1;
        plugin_ui_phase_t loading_phase;

        if (str_value[0] == '-') {
            auto_complete = 0;
            str_value++;
        }
        
        loading_phase = plugin_ui_phase_find(env->m_env, str_value);
        if (loading_phase == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phase: phae %s: loading phase %s not exist",
                plugin_ui_phase_name(b_phase), str_value);
            return NULL;
        }

        plugin_ui_navigation_phase_set_loading(b_navigation, loading_phase, auto_complete);
    }
            
    if ((str_value = cfg_get_string(navigation_cfg, "phase-leave", NULL))) {
        uint8_t auto_complete = 1;
        plugin_ui_phase_t back_phase;

        if (str_value[0] == '-') {
            auto_complete = 0;
            str_value++;
        }
        
        back_phase = plugin_ui_phase_find(env->m_env, str_value);
        if (back_phase == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phase: phae %s: back phase %s not exist",
                plugin_ui_phase_name(b_phase), str_value);
            return NULL;
        }
        plugin_ui_navigation_phase_set_back(b_navigation, back_phase, auto_complete);
    }

    return b_navigation;
}

static int ui_sprite_ui_env_phase_load_navigations_one(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, cfg_t cfg) {
    int rv = 0;
    struct plugin_ui_state_it state_it;
    plugin_ui_state_t b_state;
    
    plugin_ui_phase_states(b_phase, &state_it);
    while((b_state = plugin_ui_state_it_next(&state_it))) {
        struct cfg_it navigation_cfg_it;
        cfg_t navigation_cfg;

        cfg_it_init(&navigation_cfg_it, cfg_find_cfg(cfg_find_cfg(cfg, plugin_ui_state_name(b_state)), "Navigations"));
        while((navigation_cfg = cfg_it_next(&navigation_cfg_it))) {
            const char * str_value;
            plugin_ui_navigation_t b_navigation;
            ui_sprite_ui_navigation_t navigation;
            const char * to = NULL;
            plugin_ui_navigation_category_t category;
            plugin_ui_navigation_state_op_t state_op;
            plugin_ui_navigation_phase_op_t phase_op;
            plugin_ui_renter_policy_t renter_policy = plugin_ui_renter_back;
            const char * trigger = cfg_get_string(navigation_cfg, "trigger", NULL);
            const char * event = cfg_get_string(navigation_cfg, "event", NULL);
            const char * condition = cfg_get_string(navigation_cfg, "condition", NULL);
            uint8_t auto_execute = cfg_get_uint8(navigation_cfg, "auto-execute", 0);

            if ((str_value = cfg_get_string(navigation_cfg, "switch-to", NULL))) {
                category = plugin_ui_navigation_category_state;
                to = str_value;
                state_op = plugin_ui_navigation_state_op_switch;
            }
            else if ((str_value = cfg_get_string(navigation_cfg, "call-to", NULL))) {
                category = plugin_ui_navigation_category_state;
                to = str_value;
                state_op = plugin_ui_navigation_state_op_call;
            }
            else if  (cfg_find_cfg(navigation_cfg, "back")) {
                category = plugin_ui_navigation_category_state;
                to = NULL;
                state_op = plugin_ui_navigation_state_op_back;
            }
            else if ((str_value = cfg_get_string(navigation_cfg, "template-to", NULL))) {
                category = plugin_ui_navigation_category_state;
                to = str_value;
                state_op = plugin_ui_navigation_state_op_template;
            }
            else if ((str_value = cfg_get_string(navigation_cfg, "phase-switch-to", NULL))) {
                category = plugin_ui_navigation_category_phase;
                to = str_value;
                phase_op = plugin_ui_navigation_phase_op_switch;
            }
            else if ((str_value = cfg_get_string(navigation_cfg, "phase-call-to", NULL))) {
                category = plugin_ui_navigation_category_phase;
                to = str_value;
                phase_op = plugin_ui_navigation_phase_op_call;
            }
            else if ((str_value = cfg_get_string(navigation_cfg, "phase-back", NULL))) {
                category = plugin_ui_navigation_category_phase;
                to = NULL;
                phase_op = plugin_ui_navigation_phase_op_back;
            }
            else if ((str_value = cfg_get_string(navigation_cfg, "phase-reset", NULL))) {
                category = plugin_ui_navigation_category_phase;
                to = NULL;
                phase_op = plugin_ui_navigation_phase_op_reset;
            }
            else {
                CPE_ERROR(
                    env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: unknown navigation type",
                    plugin_ui_phase_name(b_phase));
                rv = -1;
                continue;
            }

            if ((str_value = cfg_get_string(navigation_cfg, "renter-policy", NULL))) {
                if (strcmp(str_value, "skip") == 0) {
                    renter_policy = plugin_ui_renter_skip;
                }
                else if (strcmp(str_value, "go") == 0) {
                    renter_policy = plugin_ui_renter_go;
                }
				else if (strcmp(str_value, "back") == 0) {
					renter_policy = plugin_ui_renter_back;
				}
                else {
                    CPE_ERROR(
                        env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: navigation renter-policy %s unknown",
                        plugin_ui_phase_name(b_phase), str_value);
                    rv = -1;
                    continue;
                }
            }

            /*检查配置错误 */
            if (category == plugin_ui_navigation_category_state && state_op == plugin_ui_navigation_state_op_template) {
                if (trigger) {
                    CPE_ERROR(
                        env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: template to %s: config with trigger",
                        plugin_ui_phase_name(b_phase), to);
                    rv = -1;
                    continue;
                }

                if (event) {
                    CPE_ERROR(
                        env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: template to %s: config with event",
                        plugin_ui_phase_name(b_phase), to);
                    rv = -1;
                    continue;
                }
            }
            else {
                if (!auto_execute && trigger == NULL && event == NULL) {
                    if (to) {
                        CPE_ERROR(
                            env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: to %s: no trigger or event configured",
                            plugin_ui_phase_name(b_phase), to);
                    }
                    else {
                        CPE_ERROR(
                            env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: back: no trigger or event configured",
                            plugin_ui_phase_name(b_phase));
                    }

                    rv = -1;
                    continue;
                }
            }

            if (category == plugin_ui_navigation_category_state) {
                b_navigation = ui_sprite_ui_env_phase_load_navigation_state(env, b_phase, b_state, navigation_cfg, state_op, renter_policy, to);
            }
            else {
                b_navigation = ui_sprite_ui_env_phase_load_navigation_phase(env, b_phase, b_state, navigation_cfg, phase_op, renter_policy, to);
            }

            if (b_navigation == NULL) {
                rv = -1;
                continue;
            }

            if (auto_execute) {
                plugin_ui_state_set_auto_execute(b_state, b_navigation);
            }
            
            navigation = plugin_ui_navigation_data(b_navigation);

            if (trigger) {
                if (plugin_ui_navigation_set_trigger_control(b_navigation, trigger) != 0) {
                    CPE_ERROR(
                        env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: set event fail",
                        plugin_ui_phase_name(b_phase));
                    rv = -1;
                    continue;
                }
            }

            if (event) {
                if (ui_sprite_ui_navigation_set_event(navigation, event) != 0) {
                    CPE_ERROR(
                        env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: set event fail",
                        plugin_ui_phase_name(b_phase));
                    rv = -1;
                    continue;
                }
            }

            if (condition) {
                if (plugin_ui_navigation_set_condition(b_navigation, condition) != 0) {
                    CPE_ERROR(
                        env->m_module->m_em, "ui_sprite_ui_env_phase_load_navigations: phae %s: set condition fail",
                        plugin_ui_phase_name(b_phase));
                    rv = -1;
                    continue;
                }
            }

            plugin_ui_navigation_set_weight(b_navigation, cfg_get_float(navigation_cfg, "weight", plugin_ui_navigation_weight(b_navigation)));
        }
    }
    
    return rv;
}

static int ui_sprite_ui_env_phase_load_using_package_queues(ui_sprite_ui_env_t env, plugin_ui_phase_t b_phase, cfg_t cfg) {
    struct cfg_it child_cfg_it;
    cfg_t child_cfg;
    
    cfg_it_init(&child_cfg_it, cfg);
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        const char * queue_name;
        const char * str_value;
        plugin_ui_package_queue_managed_t managed_queue;
        plugin_ui_package_queue_using_t using;        
        
        queue_name = cfg_get_string(child_cfg, "name", NULL);
        if (queue_name == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: using package queues: queue name not configured",
                plugin_ui_phase_name(b_phase));
            return -1;
        }

        managed_queue = plugin_ui_package_queue_managed_find(env->m_env, queue_name);
        if (managed_queue == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: using package queues: managed queue %s not exist",
                plugin_ui_phase_name(b_phase), queue_name);
            return -1;
        }

        using = plugin_ui_package_queue_using_create(managed_queue, b_phase);
        if (using == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_phase_load: %s: using package queues: managed queue %s create using",
                plugin_ui_phase_name(b_phase), queue_name);
            return -1;
        }

        if ((str_value = cfg_get_string(child_cfg, "limit", NULL))) {
            plugin_ui_package_queue_using_set_limit(using, (uint32_t)atoi(str_value));
        }
        else {
            plugin_ui_package_queue_using_set_limit(using, (uint32_t)-1);
        }
    }

    return 0;
}
