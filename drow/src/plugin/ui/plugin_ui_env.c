#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_math.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_manage.h"
#include "render/utils/ui_string_table.h"
#include "render/model/ui_data_utils.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_camera.h"
#include "render/cache/ui_cache_group.h"
#include "render/model/ui_data_src_group.h"
#include "render/model/ui_data_language.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_queue.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_region.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_utils_i.h"
#include "plugin_ui_env_action_i.h"
#include "plugin_ui_package_queue_managed_i.h"
#include "plugin_ui_aspect_i.h"
#include "plugin_ui_aspect_ref_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_page_eh_i.h"
#include "plugin_ui_page_slot_i.h"
#include "plugin_ui_page_meta_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_category_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_control_action_i.h"
#include "plugin_ui_control_action_slots_i.h"
#include "plugin_ui_control_timer_i.h"
#include "plugin_ui_control_binding_i.h"
#include "plugin_ui_control_binding_use_slot_i.h"
#include "plugin_ui_mouse_i.h"
#include "plugin_ui_touch_track_i.h"
#include "plugin_ui_phase_i.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_state_node_i.h"
#include "plugin_ui_state_node_page_i.h"
#include "plugin_ui_popup_i.h"
#include "plugin_ui_popup_action_i.h"
#include "plugin_ui_popup_def_i.h"
#include "plugin_ui_animation_i.h"
#include "plugin_ui_animation_control_i.h"
#include "plugin_ui_move_algorithm_i.h"

static void plugin_ui_env_update_screen_adj(plugin_ui_env_t env);

plugin_ui_env_t plugin_ui_env_create(plugin_ui_module_t module) {
    plugin_ui_env_t env;
    
    env = mem_alloc(module->m_alloc, sizeof(struct plugin_ui_env));
    if (env == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_env_create: create fail!");
        return NULL;
    }

    env->m_module = module;
    env->m_debug = 0;
    env->m_focus_control = NULL;
    env->m_float_control = NULL;
    env->m_backend = NULL;
    env->m_init_phase = NULL;
    env->m_init_call_phase = NULL;

    env->m_tick = 0;
    env->m_lock_packages = NULL;
    env->m_next_phase_loading_packages = NULL;
    env->m_next_phase_runing_packages = NULL;
    env->m_next_state_loading_packages = NULL;
    env->m_next_state_runing_packages = NULL;

    env->m_double_click_control = NULL;
    env->m_double_click_duration = 0.0f;
    env->m_double_click_span = 0.4f;

    TAILQ_INIT(&env->m_env_actions);
    TAILQ_INIT(&env->m_aspects);

    env->m_max_animation_id = 0;
    env->m_animation_wait_runging_time = 0.0f;
    TAILQ_INIT(&env->m_init_animations);
    TAILQ_INIT(&env->m_wait_animations);
    TAILQ_INIT(&env->m_working_animations);
    TAILQ_INIT(&env->m_done_animations);
    TAILQ_INIT(&env->m_keep_animations);
    
    TAILQ_INIT(&env->m_free_animations);
    TAILQ_INIT(&env->m_free_animation_controls);

    env->m_max_move_algorithm_id = 0;
    TAILQ_INIT(&env->m_free_move_algorithms);
    
    env->m_visible_pages_need_update = 1;
    TAILQ_INIT(&env->m_visible_pages);    
    TAILQ_INIT(&env->m_hiding_pages);
    TAILQ_INIT(&env->m_pages);
    TAILQ_INIT(&env->m_page_plugins);
    env->m_template_page = NULL;

    TAILQ_INIT(&env->m_package_queue_manageds);
    TAILQ_INIT(&env->m_control_categories);

    TAILQ_INIT(&env->m_popup_defs);
    TAILQ_INIT(&env->m_popups);    

    TAILQ_INIT(&env->m_free_env_actions);
    TAILQ_INIT(&env->m_free_aspects);
    TAILQ_INIT(&env->m_free_aspect_refs);
    TAILQ_INIT(&env->m_free_controls);    
    env->m_free_control_action_slots = NULL;
    TAILQ_INIT(&env->m_free_control_actions);
    TAILQ_INIT(&env->m_free_frames);    
    TAILQ_INIT(&env->m_free_timers);                                         
    TAILQ_INIT(&env->m_free_bindings);                                         
    TAILQ_INIT(&env->m_free_binding_use_slots);                                         

    TAILQ_INIT(&env->m_phase_stack);    
    TAILQ_INIT(&env->m_phases);
    TAILQ_INIT(&env->m_free_phase_nodes);    
    TAILQ_INIT(&env->m_free_state_nodes);
    TAILQ_INIT(&env->m_free_page_ehs);
    TAILQ_INIT(&env->m_free_page_slots);
    TAILQ_INIT(&env->m_free_node_pages);
    TAILQ_INIT(&env->m_free_popups);
    TAILQ_INIT(&env->m_free_popup_actions);
    
    env->m_accept_input = 1;
    env->m_accept_multi_touch = 0;
    env->m_long_push_span = 1.5f;
    env->m_popup_max_id = 0;

    env->m_origin_sz.x = 100.0f;
    env->m_origin_sz.y = 100.0f;
    env->m_runtime_sz.x = 100.0f;
    env->m_runtime_sz.y = 100.0f;
    env->m_screen_resize_policy = plugin_ui_screen_resize_free;
    env->m_screen_adj.x = 1.0f;
    env->m_screen_adj.y = 1.0f;

    env->m_visible_page_count = 0;
    env->m_page_count = 0;
    env->m_control_count = 0;
    env->m_control_free_count = 0;

    env->m_mouse = NULL;

    env->m_package_need_gc = 0;
    
    TAILQ_INIT(&env->m_touch_tracks);
    TAILQ_INIT(&env->m_free_touch_tracks);

    env->m_cfg = cfg_create(env->m_module->m_alloc);
    if (env->m_cfg == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_env_create: create cfg fail!");
        mem_free(module->m_alloc, env);
        return NULL;
    }
    
    env->m_strings = ui_string_table_create(env->m_module->m_alloc, env->m_module->m_em);
    if (env->m_strings == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_env_create: create string table fail!");
        cfg_free(env->m_cfg);
        mem_free(module->m_alloc, env);
        return NULL;
    }

    env->m_camera = ui_runtime_render_camera_create(ui_runtime_module_render(module->m_runtime), "ui");
    if (env->m_camera == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_env_create: create camera fail!");
        ui_string_table_free(env->m_strings);
        cfg_free(env->m_cfg);
        mem_free(module->m_alloc, env);
        return NULL;
    }

    env->m_lock_aspect = plugin_ui_aspect_create(env, "*lock");
    if (env->m_lock_aspect == NULL) {
        ui_runtime_render_camera_free(env->m_camera);
        ui_string_table_free(env->m_strings);
        cfg_free(env->m_cfg);
        mem_free(module->m_alloc, env);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &env->m_pages_by_name,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_ui_page_hash,
            (cpe_hash_eq_t) plugin_ui_page_eq,
            CPE_HASH_OBJ2ENTRY(plugin_ui_page, m_hh_for_env),
            -1) != 0)
    {
        plugin_ui_aspect_free(env->m_lock_aspect);
        ui_runtime_render_camera_free(env->m_camera);
        ui_string_table_free(env->m_strings);
        cfg_free(env->m_cfg);
        mem_free(module->m_alloc, env);
        return NULL;
    }

    if (cpe_hash_table_init(&env->m_animations,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_ui_animation_hash,
            (cpe_hash_eq_t) plugin_ui_animation_eq,
            CPE_HASH_OBJ2ENTRY(plugin_ui_animation, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&env->m_pages_by_name);
        plugin_ui_aspect_free(env->m_lock_aspect);
        ui_runtime_render_camera_free(env->m_camera);
        ui_string_table_free(env->m_strings);
        cfg_free(env->m_cfg);
        mem_free(module->m_alloc, env);
        return NULL;
    }

    if (cpe_hash_table_init(&env->m_move_algorithms,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_ui_move_algorithm_hash,
            (cpe_hash_eq_t) plugin_ui_move_algorithm_eq,
            CPE_HASH_OBJ2ENTRY(plugin_ui_move_algorithm, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&env->m_animations);
        cpe_hash_table_fini(&env->m_pages_by_name);
        plugin_ui_aspect_free(env->m_lock_aspect);
        ui_runtime_render_camera_free(env->m_camera);
        ui_string_table_free(env->m_strings);
        cfg_free(env->m_cfg);
        mem_free(module->m_alloc, env);
        return NULL;
    }
    
    plugin_ui_env_cfg_init(env);

    TAILQ_INSERT_TAIL(&module->m_envs, env, m_next_for_module);

    return env;
}

void plugin_ui_env_clear_runtime(plugin_ui_env_t env) {
    plugin_ui_animation_free_all(env);

    if (env->m_lock_packages) {
        plugin_package_group_free(env->m_lock_packages);
        env->m_lock_packages = NULL;
    }
    
    if (env->m_next_phase_loading_packages) {
        plugin_package_group_free(env->m_next_phase_loading_packages);
        env->m_next_phase_loading_packages = NULL;
    }

    if (env->m_next_phase_runing_packages) {
        plugin_package_group_free(env->m_next_phase_runing_packages);
        env->m_next_phase_runing_packages = NULL;
    }
    
    if (env->m_next_state_loading_packages) {
        plugin_package_group_free(env->m_next_state_loading_packages);
        env->m_next_state_loading_packages = NULL;
    }

    if (env->m_next_state_runing_packages) {
        plugin_package_group_free(env->m_next_state_runing_packages);
        env->m_next_state_runing_packages = NULL;
    }
    
    while(!TAILQ_EMPTY(&env->m_env_actions)) {
        plugin_ui_env_action_free(TAILQ_FIRST(&env->m_env_actions));
    }

    while(!TAILQ_EMPTY(&env->m_phase_stack)) {
        plugin_ui_phase_node_free(TAILQ_LAST(&env->m_phase_stack, plugin_ui_phase_node_list));
    }
    
    while(!TAILQ_EMPTY(&env->m_touch_tracks)) {
        plugin_ui_touch_track_free(TAILQ_FIRST(&env->m_touch_tracks));
    }
    
    if (env->m_template_page) {
        plugin_ui_page_free(env->m_template_page);
        env->m_template_page = NULL;
    }

    while(!TAILQ_EMPTY(&env->m_popups)) {
        plugin_ui_popup_free(TAILQ_FIRST(&env->m_popups));
    }
    assert(TAILQ_EMPTY(&env->m_popups));

    while(!TAILQ_EMPTY(&env->m_pages)) {
        plugin_ui_page_free(TAILQ_FIRST(&env->m_pages));
    }

    assert(cpe_hash_table_count(&env->m_pages_by_name) == 0);
    assert(TAILQ_EMPTY(&env->m_visible_pages));
    assert(TAILQ_EMPTY(&env->m_hiding_pages));
    assert(TAILQ_EMPTY(&env->m_page_plugins));
    assert(TAILQ_FIRST(&env->m_aspects) == env->m_lock_aspect);
    assert(TAILQ_NEXT(env->m_lock_aspect, m_next) == NULL);
    assert(TAILQ_EMPTY(&env->m_env_actions));
    assert(env->m_visible_page_count == 0);
    assert(env->m_page_count == 0);
    assert(env->m_control_count == 0);
    assert(cpe_hash_table_count(&env->m_animations) == 0);
    assert(cpe_hash_table_count(&env->m_move_algorithms) == 0);

    assert(env->m_focus_control == NULL);
    assert(env->m_float_control == NULL);
}

void plugin_ui_env_free(plugin_ui_env_t env) {
    plugin_ui_module_t module = env->m_module;

    TAILQ_REMOVE(&module->m_envs, env, m_next_for_module);

    plugin_ui_env_clear_runtime(env);

    if (env->m_mouse) {
        plugin_ui_mouse_free(env->m_mouse);
        env->m_mouse = NULL;
    }

    cpe_hash_table_fini(&env->m_pages_by_name);
    cpe_hash_table_fini(&env->m_animations);
    cpe_hash_table_fini(&env->m_move_algorithms);

    /*popups*/
    while(!TAILQ_EMPTY(&env->m_popup_defs)) {
        plugin_ui_popup_def_free(TAILQ_FIRST(&env->m_popup_defs));
    }
    
    /*phase*/
    while(!TAILQ_EMPTY(&env->m_phases)) {
        plugin_ui_phase_free(TAILQ_FIRST(&env->m_phases));
    }
    assert(env->m_init_phase == NULL);
    assert(env->m_init_call_phase == NULL);

    /*package managed queue*/
    while(!TAILQ_EMPTY(&env->m_package_queue_manageds)) {
        plugin_ui_package_queue_managed_free(TAILQ_FIRST(&env->m_package_queue_manageds));
    }
    
    /*control category*/
    while(!TAILQ_EMPTY(&env->m_control_categories)) {
        plugin_ui_control_category_free(TAILQ_FIRST(&env->m_control_categories));
    }

    /*asspect*/
    assert(env->m_lock_aspect);
    plugin_ui_aspect_free(env->m_lock_aspect);
    env->m_lock_aspect = NULL;
    while(!TAILQ_EMPTY(&env->m_aspects)) {
        plugin_ui_aspect_free(TAILQ_FIRST(&env->m_aspects));
    }
        
    /*strings*/
    ui_string_table_free(env->m_strings);
    env->m_strings = NULL;

    /*camera*/
    ui_runtime_render_camera_free(env->m_camera);
    env->m_camera = NULL;
    
    /*buff*/
    while(!TAILQ_EMPTY(&env->m_free_env_actions)) {
        plugin_ui_env_action_real_free(TAILQ_FIRST(&env->m_free_env_actions));
    }

    while(!TAILQ_EMPTY(&env->m_free_animations)) {
        plugin_ui_animation_real_free(TAILQ_FIRST(&env->m_free_animations));
    }
    
    while(!TAILQ_EMPTY(&env->m_free_animation_controls)) {
        plugin_ui_animation_control_real_free(TAILQ_FIRST(&env->m_free_animation_controls));
    }

    while(!TAILQ_EMPTY(&env->m_free_move_algorithms)) {
        plugin_ui_move_algorithm_real_free(TAILQ_FIRST(&env->m_free_move_algorithms));
    }

    while(!TAILQ_EMPTY(&env->m_free_aspects)) {
        plugin_ui_aspect_real_free(TAILQ_FIRST(&env->m_free_aspects));
    }

    while(!TAILQ_EMPTY(&env->m_free_aspect_refs)) {
        plugin_ui_aspect_ref_real_free(TAILQ_FIRST(&env->m_free_aspect_refs));
    }
    
    while(!TAILQ_EMPTY(&env->m_free_controls)) {
        plugin_ui_control_real_free(TAILQ_FIRST(&env->m_free_controls));
    }
    assert(env->m_control_free_count == 0);
    
    while(!TAILQ_EMPTY(&env->m_free_control_actions)) {
        plugin_ui_control_action_real_free(TAILQ_FIRST(&env->m_free_control_actions));
    }

    while(!TAILQ_EMPTY(&env->m_free_frames)) {
        plugin_ui_control_frame_real_free(TAILQ_FIRST(&env->m_free_frames));
    }
    
    while(env->m_free_control_action_slots) {
        plugin_ui_control_action_slots_real_free(env, env->m_free_control_action_slots);
    }

    while(!TAILQ_EMPTY(&env->m_free_timers)) {
        plugin_ui_control_timer_real_free(TAILQ_FIRST(&env->m_free_timers));
    }

    while(!TAILQ_EMPTY(&env->m_free_bindings)) {
        plugin_ui_control_binding_real_free(TAILQ_FIRST(&env->m_free_bindings));
    }

    while(!TAILQ_EMPTY(&env->m_free_binding_use_slots)) {
        plugin_ui_control_binding_use_slot_real_free(TAILQ_FIRST(&env->m_free_binding_use_slots));
    }
    
    while(!TAILQ_EMPTY(&env->m_free_phase_nodes)) {
        plugin_ui_phase_node_real_free(TAILQ_FIRST(&env->m_free_phase_nodes));
    }

    while(!TAILQ_EMPTY(&env->m_free_state_nodes)) {
        plugin_ui_state_node_real_free(TAILQ_FIRST(&env->m_free_state_nodes));
    }

    while(!TAILQ_EMPTY(&env->m_free_page_ehs)) {
        plugin_ui_page_eh_real_free(TAILQ_FIRST(&env->m_free_page_ehs));
    }

    while(!TAILQ_EMPTY(&env->m_free_page_slots)) {
        plugin_ui_page_slot_real_free(TAILQ_FIRST(&env->m_free_page_slots));
    }

    while(!TAILQ_EMPTY(&env->m_free_node_pages)) {
        plugin_ui_state_node_page_real_free(TAILQ_FIRST(&env->m_free_node_pages));
    }

    while(!TAILQ_EMPTY(&env->m_free_popups)) {
        plugin_ui_popup_real_free(TAILQ_FIRST(&env->m_free_popups));
    }

    while(!TAILQ_EMPTY(&env->m_free_popup_actions)) {
        plugin_ui_popup_action_real_free(TAILQ_FIRST(&env->m_free_popup_actions));
    }

    while(!TAILQ_EMPTY(&env->m_free_touch_tracks)) {
        plugin_ui_touch_track_real_free(TAILQ_FIRST(&env->m_free_touch_tracks));
    }

    cfg_free(env->m_cfg);
    
    mem_free(module->m_alloc, env);
}

plugin_ui_env_t plugin_ui_env_first(plugin_ui_module_t module) {
    return TAILQ_FIRST(&module->m_envs);
}

plugin_ui_module_t plugin_ui_env_module(plugin_ui_env_t env) {
    return env->m_module;
}

gd_app_context_t plugin_ui_env_app(plugin_ui_env_t env) {
    return env->m_module->m_app;
}

cfg_t plugin_ui_env_cfg(plugin_ui_env_t env) {
    return env->m_cfg;
}

ui_data_mgr_t plugin_ui_env_data_mgr(plugin_ui_env_t env) {
    return env->m_module->m_data_mgr;
}

ui_runtime_module_t plugin_ui_env_runtime(plugin_ui_env_t env) {
    return env->m_module->m_runtime;
}

void plugin_ui_env_set_backend(plugin_ui_env_t env, plugin_ui_env_backend_t backend) {
    env->m_backend = backend;
}

uint8_t plugin_ui_env_debug(plugin_ui_env_t env) {
    return env->m_debug;
}

void plugin_ui_env_set_debug(plugin_ui_env_t env, uint8_t debug) {
    env->m_debug = debug;
}

ui_cache_manager_t plugin_ui_env_cache_mgr(plugin_ui_env_t env) {
    return ui_runtime_module_cache_mgr(env->m_module->m_runtime);
}

plugin_package_module_t plugin_ui_env_package_mgr(plugin_ui_env_t env) {
    return env->m_module->m_package_module;
}

ui_string_table_t plugin_ui_env_string_table(plugin_ui_env_t env) {
    return env->m_strings;
}

uint8_t plugin_ui_env_accept_multi_touch(plugin_ui_env_t env) {
    return env->m_accept_multi_touch;
}

void plugin_ui_env_set_accept_multi_touch(plugin_ui_env_t env, uint8_t accept_multi_touch) {
    env->m_accept_multi_touch = accept_multi_touch;
}

void plugin_ui_env_send_event(plugin_ui_env_t env, LPDRMETA meta, void * data, uint32_t data_size) {
    env->m_backend->send_event(env->m_backend->ctx, env, meta, data, data_size, NULL);
}

void plugin_ui_env_build_and_send_event(plugin_ui_env_t env, const char * def, dr_data_source_t data_source) {
    env->m_backend->build_and_send_event(env->m_backend->ctx, env, def, data_source, NULL);
}

float plugin_ui_env_double_click_span(plugin_ui_env_t env) {
    return env->m_double_click_span;
}

void plugin_ui_env_double_click_set_span(plugin_ui_env_t env, float span) {
    env->m_double_click_span = span;
}

ui_vector_2_t plugin_ui_env_runtime_sz(plugin_ui_env_t env) {
    return &env->m_runtime_sz;
}

void plugin_ui_env_set_runtime_sz(plugin_ui_env_t env, ui_vector_2_t sz) {
    plugin_ui_page_t page;
    plugin_ui_popup_t popup;
    
    env->m_runtime_sz = *sz;
    plugin_ui_env_update_screen_adj(env);
    
    TAILQ_FOREACH(page, &env->m_pages, m_next_for_env) {
        plugin_ui_page_update_area(page);
    }

    TAILQ_FOREACH(popup, &env->m_popups, m_next_for_env) {
        plugin_ui_page_update_area(popup->m_page);
    }

    if (env->m_template_page) {
        plugin_ui_page_update_area(env->m_template_page);
    }
}

ui_vector_2_t plugin_ui_env_screen_adj(plugin_ui_env_t env) {
    return &env->m_screen_adj;
}

ui_vector_2_t plugin_ui_env_origin_sz(plugin_ui_env_t env) {
    return &env->m_origin_sz;
}

void plugin_ui_env_set_origin_sz(plugin_ui_env_t env, ui_vector_2_t sz) {
    plugin_ui_page_t page;

    assert(sz->x > 0.0f);
    assert(sz->y > 0.0f);
    assert(ui_vector_2_cmp(&env->m_origin_sz, &env->m_runtime_sz) == 0);
    
    env->m_origin_sz = *sz;
    env->m_runtime_sz = *sz;
    env->m_screen_adj.x = 1.0f;
    env->m_screen_adj.y = 1.0f;
    
    TAILQ_FOREACH(page, &env->m_pages, m_next_for_env) {
        plugin_ui_page_update_area(page);
    }
}

plugin_ui_screen_resize_policy_t plugin_ui_env_screen_resize_policy(plugin_ui_env_t env) {
    return env->m_screen_resize_policy;
}

void plugin_ui_env_set_screen_resize_policy(plugin_ui_env_t env, plugin_ui_screen_resize_policy_t policy) {
    env->m_screen_resize_policy = policy;
    plugin_ui_env_update_screen_adj(env);
}

plugin_ui_control_t plugin_ui_env_focus_control(plugin_ui_env_t env) {
    return env->m_focus_control;
}

void plugin_ui_env_set_focus_control(plugin_ui_env_t env, plugin_ui_control_t ctrl) {
    plugin_ui_page_t process_page = NULL;
    plugin_ui_control_t process_control = NULL;
    uint8_t page_tag_local = 0;
    uint8_t control_tag_local = 0;
    plugin_ui_control_t last_ctrl;
    
    if (env->m_focus_control == ctrl) return;

    last_ctrl = env->m_focus_control;
    env->m_focus_control = ctrl;

    if (last_ctrl && !last_ctrl->m_is_free && !last_ctrl->m_page->m_control.m_is_free) {
        process_control = last_ctrl;
        process_page = last_ctrl->m_page;

        if (!process_control->m_is_processing) {
            control_tag_local = 1;
            process_control->m_is_processing = 1;
            assert(!process_control->m_is_free);
        }

        if (!process_page->m_control.m_is_processing) {
            page_tag_local = 1;
            process_page->m_control.m_is_processing = 1;
            assert(!process_page->m_control.m_is_free);
        }

        plugin_ui_control_dispatch_event(
            process_control, process_control, plugin_ui_event_lost_focus, plugin_ui_event_dispatch_to_self_and_parent);
        if (process_control->m_is_free || process_page->m_control.m_is_free) goto COMPLETE;

        if (control_tag_local) {
            process_control->m_is_processing = 0;
            control_tag_local = 0;
            assert(!process_control->m_is_free);
        }
        
        if (page_tag_local) {
            process_page->m_control.m_is_processing = 0;
            page_tag_local = 0;
            assert(!process_page->m_control.m_is_free);
        }
    }

    if (ctrl) {
        assert(!ctrl->m_is_free);
        process_control = ctrl;
        process_page = ctrl->m_page;

        assert(control_tag_local == 0);
        assert(page_tag_local == 0);
        
        if (!process_control->m_is_processing) {
            control_tag_local = 1;
            process_control->m_is_processing = 1;
            assert(!process_control->m_is_free);
        }

        if (!process_page->m_control.m_is_processing) {
            page_tag_local = 1;
            process_page->m_control.m_is_processing = 1;
            assert(!process_page->m_control.m_is_free);
        }

        plugin_ui_control_dispatch_event(
            ctrl, ctrl, plugin_ui_event_gain_focus, plugin_ui_event_dispatch_to_self_and_parent);
        if (process_control->m_is_free || process_page->m_control.m_is_free) goto COMPLETE;

        if (control_tag_local) {
            process_control->m_is_processing = 0;
            control_tag_local = 0;
            assert(!process_control->m_is_free);
        }
        
        if (page_tag_local) {
            process_page->m_control.m_is_processing = 0;
            page_tag_local = 0;
            assert(!process_page->m_control.m_is_free);
        }
    }

COMPLETE:
    if (control_tag_local) {
        process_control->m_is_processing = 0;
        if (process_control->m_is_free) {
            plugin_ui_control_free(process_control);
        }
    }

    if (page_tag_local) {
        process_page->m_control.m_is_processing = 0;
        if (process_page->m_control.m_is_free) {
            plugin_ui_page_free(process_page);
        }
    }
}

plugin_ui_control_t plugin_ui_env_float_control(plugin_ui_env_t env) {
    return env->m_float_control;
}

void plugin_ui_env_set_float_control(plugin_ui_env_t env, plugin_ui_control_t ctrl) {
    plugin_ui_page_t process_page = NULL;
    plugin_ui_control_t process_control = NULL;
    uint8_t page_tag_local = 0;
    uint8_t control_tag_local = 0;
    plugin_ui_control_t last_ctrl;
    
    if (env->m_float_control == ctrl) return;

    last_ctrl = env->m_float_control;
    env->m_float_control = ctrl;

    if (last_ctrl && !last_ctrl->m_is_free && !last_ctrl->m_page->m_control.m_is_free) {
        process_control = last_ctrl;
        process_page = last_ctrl->m_page;

        if (!process_control->m_is_processing) {
            control_tag_local = 1;
            process_control->m_is_processing = 1;
            assert(!process_control->m_is_free);
        }

        if (!process_page->m_control.m_is_processing) {
            page_tag_local = 1;
            process_page->m_control.m_is_processing = 1;
            assert(!process_page->m_control.m_is_free);
        }

        plugin_ui_control_set_float(process_control, 0);
        plugin_ui_control_dispatch_event(
            process_control, process_control, plugin_ui_event_float_done, plugin_ui_event_dispatch_to_self_and_parent);
        if (process_control->m_is_free || process_page->m_control.m_is_free) goto COMPLETE;

        if (control_tag_local) {
            process_control->m_is_processing = 0;
            control_tag_local = 0;
            assert(!process_control->m_is_free);
        }
        
        if (page_tag_local) {
            process_page->m_control.m_is_processing = 0;
            page_tag_local = 0;
            assert(!process_page->m_control.m_is_free);
        }
    }

    if (ctrl) {
        assert(!ctrl->m_is_free);
        process_control = ctrl;
        process_page = ctrl->m_page;

        assert(control_tag_local == 0);
        assert(page_tag_local == 0);
        
        if (!process_control->m_is_processing) {
            control_tag_local = 1;
            process_control->m_is_processing = 1;
            assert(!process_control->m_is_free);
        }

        if (!process_page->m_control.m_is_processing) {
            page_tag_local = 1;
            process_page->m_control.m_is_processing = 1;
            assert(!process_page->m_control.m_is_free);
        }

        plugin_ui_control_set_float(process_control, 1);
        plugin_ui_control_dispatch_event(
            ctrl, ctrl, plugin_ui_event_float_begin, plugin_ui_event_dispatch_to_self_and_parent);
        if (process_control->m_is_free || process_page->m_control.m_is_free) goto COMPLETE;

        if (control_tag_local) {
            process_control->m_is_processing = 0;
            control_tag_local = 0;
            assert(!process_control->m_is_free);
        }
        
        if (page_tag_local) {
            process_page->m_control.m_is_processing = 0;
            page_tag_local = 0;
            assert(!process_page->m_control.m_is_free);
        }
    }

COMPLETE:
    if (control_tag_local) {
        process_control->m_is_processing = 0;
        if (process_control->m_is_free) {
            plugin_ui_control_free(process_control);
        }
    }

    if (page_tag_local) {
        process_page->m_control.m_is_processing = 0;
        if (process_page->m_control.m_is_free) {
            plugin_ui_page_free(process_page);
        }
    }
}

void plugin_ui_env_set_double_click_control(plugin_ui_env_t env, plugin_ui_control_t ctrl) {
    env->m_double_click_control = ctrl;
    env->m_double_click_duration = 0.0f;
}

void plugin_ui_control_evt_on_control_remove(plugin_ui_env_t env, plugin_ui_control_t control) {
    plugin_ui_touch_track_t track;
    
    assert(control->m_parent);

    if (env->m_float_control == control) {
        plugin_ui_env_set_float_control(env, NULL);
    }

    if (env->m_double_click_control == control) {
        plugin_ui_env_set_double_click_control(env, NULL);
    }
    
    if (env->m_focus_control == control) {
        plugin_ui_env_set_focus_control(env, NULL);
    }

    TAILQ_FOREACH(track, &env->m_touch_tracks, m_next_for_env) {
        if (track->m_catch_control == control) {
            if (!control->m_is_free) {
                plugin_ui_control_dispatch_event(
                    track->m_catch_control, track->m_catch_control,
                    plugin_ui_event_mouse_up, plugin_ui_event_dispatch_to_self_and_parent);
            }
            track->m_catch_control = NULL;
            track->m_cache_duration = 0.0f;
        }

        if (track->m_process_control == control) {
            plugin_ui_control_dispatch_event(
                track->m_catch_control, track->m_catch_control,
                plugin_ui_event_mouse_click, plugin_ui_event_dispatch_to_self_and_parent);
            track->m_process_control = NULL;
        }
    }
}

void plugin_ui_env_update(plugin_ui_env_t env, float delta) {
    plugin_ui_page_t page;
    plugin_ui_touch_track_t track;

    env->m_tick++;
    
PLUGIN_UI_ENV_UDPATE_AGAIN:
    plugin_ui_env_update_double_click(env, delta);
    plugin_ui_env_update_popups(env, delta);
    plugin_ui_env_update_phase(env);
    plugin_ui_env_update_hiding_pages(env);

    /*更新所有控件 */
    if (env->m_visible_pages_need_update) {
        plugin_ui_env_update_visible_pages(env);
        env->m_visible_pages_need_update = 0;
    }

    plugin_ui_env_update_animations(env, delta);
    
    TAILQ_FOREACH(page, &env->m_visible_pages, m_next_for_visible_queue) {
        if(page->m_hiding) continue;

        assert(plugin_ui_page_visible(page));
        
        if (page->m_changed) {
            page->m_changed = 0;
            plugin_ui_control_update_cache(&page->m_control, 0);
            if (page->m_page_meta) {
                page->m_page_meta->m_on_changed(page);
                if (!plugin_ui_page_visible(page)) continue;
            }
        }
        
        /*更新数值 */
        while(!TAILQ_EMPTY(&page->m_need_process_bindings)) {
            plugin_ui_control_binding_t binding = TAILQ_FIRST(&page->m_need_process_bindings);
            plugin_ui_control_binding_set_need_process(binding, 0);
            plugin_ui_control_binding_apply(binding);
        }

        plugin_ui_control_visit_tree(
            &page->m_control, plugin_ui_control_visit_tree_bfs, &delta, plugin_ui_control_do_update, 1);

        if (page->m_page_meta && page->m_page_meta->m_on_update) page->m_page_meta->m_on_update(page, delta);
    }

    /*检测长按 */
    TAILQ_FOREACH(track, &env->m_touch_tracks, m_next_for_env) {
        track->m_cache_duration += delta;
        
        if(fabs(track->m_down_pt.x - track->m_cur_pt.x) > 10.0f
            || fabs(track->m_down_pt.y - track->m_cur_pt.y) > 10.0f)
            track->m_cache_duration = 0;

        if (track->m_catch_control && !track->m_long_push_sended) {
            if (track->m_cache_duration >= env->m_long_push_span) {

                plugin_ui_control_dispatch_event(
                    track->m_catch_control, track->m_catch_control,
                    plugin_ui_event_mouse_long_push, plugin_ui_event_dispatch_to_self_and_parent);

                track->m_long_push_sended = 1;
            }
        }
    }

    TAILQ_FOREACH(page, &env->m_visible_pages, m_next_for_visible_queue) {
        if(page->m_hiding) {
            delta = 0.0f;
            goto PLUGIN_UI_ENV_UDPATE_AGAIN;
        }
    }
}

uint32_t plugin_ui_env_page_count(plugin_ui_env_t env) {
    return env->m_page_count;
}

uint32_t plugin_ui_env_page_visiable_count(plugin_ui_env_t env) {
    return env->m_visible_page_count;
}

uint32_t plugin_ui_env_control_count(plugin_ui_env_t env) {
    return env->m_control_count;
}

uint32_t plugin_ui_env_control_free_count(plugin_ui_env_t env) {
    return env->m_control_free_count;
}

void plugin_ui_env_update_visible_pages(plugin_ui_env_t env) {
    plugin_ui_phase_node_t cur_phase;
    plugin_ui_state_node_t cur_state;
    plugin_ui_state_node_t state_node;
    plugin_ui_state_node_page_t visible_page;
    plugin_ui_popup_t popup;

    cur_phase = plugin_ui_phase_node_current(env);
    if (cur_phase == NULL) return;

    for(cur_state = TAILQ_LAST(&cur_phase->m_state_stack, plugin_ui_state_node_list);
        cur_state;
        cur_state = TAILQ_PREV(cur_state, plugin_ui_state_node_list, m_next))
    {
        if (cur_state->m_op != plugin_ui_state_node_op_init) break;
    }

    state_node = cur_state;
    while(state_node && !state_node->m_curent.m_suspend_old) {
        plugin_ui_state_node_t prev_node = TAILQ_PREV(state_node, plugin_ui_state_node_list, m_next);
        if (prev_node == NULL) break;
        state_node = prev_node;
    }
    
    while(!TAILQ_EMPTY(&env->m_visible_pages)) {
        plugin_ui_page_t page = TAILQ_FIRST(&env->m_visible_pages);
        assert(page->m_is_in_visible_queue);
        assert(env->m_visible_page_count > 0);
        env->m_visible_page_count--;
        TAILQ_REMOVE(&env->m_visible_pages, page, m_next_for_visible_queue);
        page->m_is_in_visible_queue = 0;
    }

    assert(env->m_visible_page_count == 0);
    for(; state_node; state_node = TAILQ_NEXT(state_node, m_next)) {
        TAILQ_FOREACH(visible_page, &state_node->m_pages, m_next_for_state_node) {
            plugin_ui_page_t page = visible_page->m_page;

            if (TAILQ_LAST(&page->m_visible_in_states, plugin_ui_state_node_page_list) != visible_page) continue;

            assert(!page->m_is_in_visible_queue);

            if (visible_page->m_before_page[0]) {
                if (strcasecmp(visible_page->m_before_page, "all") == 0) {
                    TAILQ_INSERT_HEAD(&env->m_visible_pages, page, m_next_for_visible_queue);
                }
                else {
                    plugin_ui_page_t before_page;
                    TAILQ_FOREACH(before_page, &env->m_visible_pages, m_next_for_visible_queue) {
                        if (strcmp(plugin_ui_page_name(before_page), visible_page->m_before_page) == 0) break;
                    }

                    if (before_page) {
                        TAILQ_INSERT_BEFORE(before_page, page, m_next_for_visible_queue);
                    }
                    else {
                        TAILQ_INSERT_TAIL(&env->m_visible_pages, page, m_next_for_visible_queue);
                    }
                }
            }
            else {
                TAILQ_INSERT_TAIL(&env->m_visible_pages, page, m_next_for_visible_queue);
            }

            env->m_visible_page_count++;
            page->m_is_in_visible_queue = 1;
        }

        if (state_node == cur_state) break;
    }

    TAILQ_FOREACH(popup, &env->m_popups, m_next_for_env) {
        if (plugin_ui_popup_visible(popup)) {
            env->m_visible_page_count++;            
            TAILQ_INSERT_TAIL(&env->m_visible_pages, popup->m_page, m_next_for_visible_queue);
            popup->m_page->m_is_in_visible_queue = 1;
            /* printf("xxxxx: layer %d: %s\n", popup->m_layer, plugin_ui_page_name(popup->m_page)); */
        }
    }
}

void plugin_ui_env_update_hiding_pages(plugin_ui_env_t env) {
    plugin_ui_page_t page, next_page;
    
    for(page = TAILQ_FIRST(&env->m_hiding_pages); page; page = next_page) {
        next_page = TAILQ_NEXT(page, m_next_for_hiding);
        plugin_ui_page_do_hide(page);
    }
}

void plugin_ui_env_update_popups(plugin_ui_env_t env, float delta) {
    plugin_ui_popup_t popup, pre_processed;

    pre_processed = NULL;
    for(popup = TAILQ_FIRST(&env->m_popups);
        popup;
        popup = pre_processed ? TAILQ_NEXT(pre_processed, m_next_for_env) : TAILQ_FIRST(&env->m_popups))
    {
        if (!plugin_ui_popup_visible(popup)) {
            plugin_ui_popup_free(popup);
            continue;
        }

        if (popup->m_lifecircle > 0.0f) {
            if (popup->m_lifecircle <= delta) {
                plugin_ui_popup_free(popup);
                continue;
            }
            else {
                popup->m_lifecircle -= delta;
            }
        }

        pre_processed = popup;
    }
}

void plugin_ui_env_dispatch_event(plugin_ui_env_t env, plugin_ui_control_t from, plugin_ui_event_t evt) {
    plugin_ui_page_t page = from ? from->m_page : NULL;
    uint8_t page_tag_local = 0;
    uint8_t from_tag_local = 0;
    plugin_ui_env_action_t env_action, next_env_action;
    const char * from_name = from ? plugin_ui_control_name(from) : NULL;

    assert(page == NULL || !page->m_control.m_is_free);
    assert(from == NULL || !from->m_is_free);

    if (page && !page->m_control.m_is_processing) {
        page->m_control.m_is_processing = 1;
        page_tag_local = 1;
    }
    
    if (from && !from->m_is_processing) {
        from->m_is_processing = 1;
        from_tag_local = 1;
    }
    
    for(env_action = TAILQ_FIRST(&env->m_env_actions); env_action; env_action = next_env_action) {
        if (env_action->m_event != evt) {
            next_env_action = TAILQ_NEXT(env_action, m_next_for_env);
            continue;
        }
        
        if (env_action->m_name_prefix == NULL || (from_name && cpe_str_start_with(from_name, env_action->m_name_prefix))) {
            assert(!env_action->m_is_processing);
            assert(!env_action->m_is_free);

            env_action->m_is_processing = 1;
            env_action->m_fun(env_action->m_ctx ? env_action->m_ctx : env_action->m_data, from, evt);
            env_action->m_is_processing = 0;

            next_env_action = TAILQ_NEXT(env_action, m_next_for_env);
            
            if (env_action->m_is_free) plugin_ui_env_action_free(env_action);
            
            if ((page && page->m_control.m_is_free) || (from && from->m_is_free)) goto COMPLETE;
        }
    }

COMPLETE:
    if (from_tag_local) {
        from->m_is_processing = 0;
        if (from->m_is_free) plugin_ui_control_free(from);
    }

    if (page_tag_local) {
        page->m_control.m_is_processing = 0;
        if (page->m_control.m_is_free) plugin_ui_page_free(page);
    }
}

static void plugin_ui_env_update_screen_adj(plugin_ui_env_t env) {
    switch(env->m_screen_resize_policy) {
    case plugin_ui_screen_resize_by_width:
        env->m_screen_adj.x = env->m_screen_adj.y = env->m_runtime_sz.x / env->m_origin_sz.x;
        break;
    case plugin_ui_screen_resize_by_height:
        env->m_screen_adj.x = env->m_screen_adj.y = env->m_runtime_sz.y / env->m_origin_sz.y;
        break;
    case plugin_ui_screen_resize_free:
        env->m_screen_adj.x = env->m_runtime_sz.x / env->m_origin_sz.x;
        env->m_screen_adj.y = env->m_runtime_sz.y / env->m_origin_sz.y;
        break;
    }
}

int plugin_ui_env_set_language(plugin_ui_env_t env, const char * str_language) {
    const char * root = gd_app_root(env->m_module->m_app);
    ui_data_language_t language;
    char file_buf[256];

    language = ui_data_language_find(env->m_module->m_data_mgr, str_language);
    if (language == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_set_language: language %s not exist!", str_language);
        return -1;
    }

    if (root) {
        snprintf(file_buf, sizeof(file_buf), "%s/language/strings_%s.stb", root, str_language);
    }
    else {
        snprintf(file_buf, sizeof(file_buf), "language/strings_%s.stb", str_language);
    }
    
    if (ui_string_table_load_file(
        plugin_ui_env_string_table(env),
        gd_app_vfs_mgr(env->m_module->m_app),
        file_buf) != 0)
    {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_set_language: load strings from %s fail!", file_buf);
        return -1;
    }

    ui_data_language_active(language);

    //TODO:
    
    return 0;
}

const char * plugin_ui_env_language(plugin_ui_env_t env) {
	return ui_data_language_name(ui_data_active_language(env->m_module->m_data_mgr));
}

int plugin_ui_env_load_package_to_queue_async(
    plugin_ui_env_t env, const char * package_name, const char * queue_name, plugin_package_load_task_t task)
{
    plugin_ui_package_queue_managed_t queue;
    plugin_package_package_t package;

    assert(package_name);
    package = plugin_package_package_find(env->m_module->m_package_module, package_name);
    if (package == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_to_queue_async: package %s not exist!", package_name);
        return -1;
    }

    assert(queue_name);
    queue = plugin_ui_package_queue_managed_find(env, queue_name);
    if (queue == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_to_queue_async: queue %s: queue not exist!", queue_name);
        return -1;
    }

    if (plugin_package_queue_add_package(queue->m_package_queue, package) != 0) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_to_queue_async: add package %s to queue %s fail!", package_name, queue_name);
        return -1;
    }

    if (plugin_package_package_load_async_r(package, task) != 0) {
        plugin_package_queue_remove_package(queue->m_package_queue, package);
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_to_queue_async: load package %s async fail!", package_name);
        return -1;
    }

    return 0;
}

int plugin_ui_env_load_package_async(
    plugin_ui_env_t env, const char * package_name, plugin_ui_package_scope_t scope, plugin_package_load_task_t task)
{
    plugin_package_group_t scope_group;
    plugin_package_package_t package = NULL;
    plugin_ui_phase_node_t cur_phase_node;
    plugin_ui_state_node_t cur_state_node;
    
    assert(package_name);
    package = plugin_package_package_find(env->m_module->m_package_module, package_name);
    if (package == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_async: package %s not exist!", package_name);
        return -1;
    }

    cur_phase_node = plugin_ui_phase_node_current(env);
    if (cur_phase_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_async: no current phase!");
        return -1;
    }
    
    switch(scope) {
    case plugin_ui_package_cur_phase:
        scope_group = cur_phase_node->m_runtime_packages;
        break;
    case plugin_ui_package_next_phase:
        if (env->m_next_phase_runing_packages == NULL) {
            env->m_next_phase_runing_packages = plugin_package_group_create(env->m_module->m_package_module, "next-phase-runing");
            if (env->m_next_phase_runing_packages == NULL) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_async: create nexts phase package group fail!");
                return -1;
            }
        }
        scope_group = env->m_next_phase_runing_packages;
        break;
    case plugin_ui_package_next_phase_loading:
        if (env->m_next_phase_loading_packages == NULL) {
            env->m_next_phase_loading_packages = plugin_package_group_create(env->m_module->m_package_module, "next-phase-loading");
            if (env->m_next_phase_loading_packages == NULL) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_async: create nexts phase package group fail!");
                return -1;
            }
        }
        scope_group = env->m_next_phase_loading_packages;
        break;
    case plugin_ui_package_cur_state:
        cur_state_node = plugin_ui_state_node_current(cur_phase_node);
        if (cur_state_node == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_async: no current state!");
            return -1;
        }
        scope_group = plugin_ui_state_node_runtime_packages_check_create(cur_state_node);
        break;
    case plugin_ui_package_next_state:
        if (env->m_next_state_runing_packages == NULL) {
            env->m_next_state_runing_packages = plugin_package_group_create(env->m_module->m_package_module, "next-state");
            if (env->m_next_state_runing_packages == NULL) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_env_load_package_async: create nexts state package group fail!");
                return -1;
            }
        }
        scope_group = env->m_next_state_runing_packages;
        break;
    }

    if (plugin_package_group_add_package_r(scope_group, package) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_env_load_package_async: add package %s to group fail!",
            package_name);
        return -1;
    }

    if (scope == plugin_ui_package_cur_phase || scope == plugin_ui_package_cur_state) {
        if (plugin_package_package_load_async_r(package, task) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_load_package_async: load package %s async fail!",
                package_name);
            return -1;
        }
    }
    
    return 0;
}

int plugin_ui_env_calc_world_pos(ui_vector_2_t result, plugin_ui_env_t env, const char * str) {
    if (str[0] == '(') {
        const char * end;
        const char * limit;
        const char * sep;
        char buf[32];
        
        limit = strchr(str, ',');
        if (limit == NULL) limit = str + strlen(str);

        end = strchr(str, ')');
        sep = strchr(str, '-');
        if (end > limit || sep > limit) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_calc_world_pos: %s format(pos) error!", str);
            return -1;
        }

        result->x = atof(cpe_str_dup_range(buf, sizeof(buf), str + 1, sep));
        result->y = atof(cpe_str_dup_range(buf, sizeof(buf), sep + 1, end));

        return 0;
    }
    else {
        const char * sep = strchr(str, '@');
        char policy_buf[32];
        plugin_ui_control_t control;
        ui_vector_2_t control_pt;

        if (sep == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_calc_world_pos: %s format error!", str);
            return -1;
        }

        control = plugin_ui_control_find_by_path(env, sep + 1);
        if (control == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_calc_world_pos: control %s not exist!", sep + 1);
            return -1;
        }
        
        plugin_ui_control_check_update_from_root(control);
        control_pt = plugin_ui_control_real_pt_abs(control);

        cpe_str_dup_range(policy_buf, sizeof(policy_buf), str, sep);
        *result = plugin_ui_calc_adj_sz_by_pos_policy(plugin_ui_control_real_sz_abs(control), &control->m_pivot, ui_pos_policy_from_str(policy_buf));
        result->x += control_pt->x;
        result->y += control_pt->y;
    
        return 0;
    }
}

void plugin_ui_env_update_double_click(plugin_ui_env_t env, float delta) {
    if (env->m_double_click_control) {
        if (plugin_ui_control_hide_by(env->m_double_click_control)) {
            plugin_ui_env_set_double_click_control(env, NULL);
        }
        else {
            env->m_double_click_duration += delta;
            if (env->m_double_click_duration > env->m_double_click_span) {
                plugin_ui_control_t control = env->m_double_click_control;
                plugin_ui_page_t page = control->m_page;
                uint8_t control_tag_local = 0;
                uint8_t page_tag_local = 0;
                
                if (!control->m_is_processing) {
                    control_tag_local = 1;
                    control->m_is_processing = 1;
                }
                
                if (!page->m_control.m_is_processing) {
                    page_tag_local = 1;
                    page->m_control.m_is_processing = 1;
                }
                
                plugin_ui_control_dispatch_event(
                    control, control,
                    plugin_ui_event_mouse_click, plugin_ui_event_dispatch_to_self_and_parent);
                env->m_double_click_control = NULL;
                env->m_double_click_duration = 0.0f;

                if (control_tag_local) {
                    control->m_is_processing = 0;
                    if (control->m_is_free) plugin_ui_control_free(control);
                }

                if (page_tag_local) {
                    page->m_control.m_is_processing = 0;
                    if (page->m_control.m_is_free) plugin_ui_page_free(page);
                }
                
            }
        }
    }
}

plugin_ui_control_t plugin_ui_env_find_float_control(plugin_ui_env_t env, ui_vector_2_t pt) {
    plugin_ui_phase_node_t cur_phase;
    plugin_ui_page_t page;

    cur_phase = plugin_ui_phase_node_current(env);
    if (cur_phase == NULL) return NULL;
    
    TAILQ_FOREACH_REVERSE(page, &env->m_visible_pages, plugin_ui_page_list, m_next_for_visible_queue) {
        plugin_ui_control_t float_control = plugin_ui_control_find_float(&page->m_control, pt);
        if (float_control) return float_control;
    }

    return NULL;
}

int plugin_ui_env_init_lock_packages(plugin_ui_env_t env) {
    plugin_package_region_t region;
    
    if (env->m_lock_packages) return 0;

    env->m_lock_packages = plugin_package_group_create(env->m_module->m_package_module, "lock");
    if (env->m_lock_packages == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_init_lock_packages: create package group fail!");
        return -1;
    }
    plugin_package_group_set_using_state(env->m_lock_packages, plugin_package_package_using_state_ref_count);

    if ((region = plugin_package_region_find(env->m_module->m_package_module, "lock"))) {
        struct plugin_package_package_it package_it;
        plugin_package_package_t package;

        plugin_package_group_packages(&package_it, plugin_package_region_group(region));
        while((package = plugin_package_package_it_next(&package_it))) {
            plugin_package_group_add_package_r(env->m_lock_packages, package);
        }
    }

    return 0;
}

void plugin_ui_env_package_gc(plugin_ui_env_t env) {
    env->m_package_need_gc = 0;
    plugin_package_module_gc(env->m_module->m_package_module);
}
