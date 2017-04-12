#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "plugin_ui_animation_i.h"
#include "plugin_ui_animation_meta_i.h"
#include "plugin_ui_animation_control_i.h"
#include "plugin_ui_aspect_ref_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"

plugin_ui_animation_t
plugin_ui_animation_create(plugin_ui_env_t env, plugin_ui_animation_meta_t meta) {
    plugin_ui_module_t module = env->m_module;
    plugin_ui_animation_t animation;

    animation = TAILQ_FIRST(&env->m_free_animations);
    if (animation) {
        TAILQ_REMOVE(&env->m_free_animations, animation, m_next_for_env);
    }
    else {
        animation = mem_alloc(module->m_alloc, sizeof(struct plugin_ui_animation) + module->m_animation_max_capacity);
        if (animation == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_animation: create: alloc fail!");
            return NULL;
        }
    }

    animation->m_env = env;
    animation->m_meta = meta;
    animation->m_id = env->m_max_animation_id + 1;
    animation->m_name = NULL;
    animation->m_state = plugin_ui_animation_state_init;
    animation->m_auto_free = 1;
    animation->m_delay = 0.0f;
    animation->m_loop_count = 1;
    animation->m_loop_delay = 0.0f;
    animation->m_is_processing = 0;
    animation->m_on_complete.m_ctx = NULL;
    animation->m_on_complete.m_fun = NULL;
    animation->m_on_complete.m_arg = NULL;
    animation->m_on_complete.m_arg_free = NULL;
    animation->m_aspect = NULL;
    TAILQ_INIT(&animation->m_controls);
    TAILQ_INIT(&animation->m_aspects);
    bzero(animation + 1, module->m_animation_max_capacity);
    
    if (meta->m_init_fun && meta->m_init_fun(animation, meta->m_ctx) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_animation: type %s init fail!", meta->m_name);
        TAILQ_INSERT_TAIL(&env->m_free_animations, animation, m_next_for_env);
        return NULL;
    }
    
    cpe_hash_entry_init(&animation->m_hh);
    if (cpe_hash_table_insert(&env->m_animations, animation) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_animation: id %d duplicate!", animation->m_id);
        TAILQ_INSERT_TAIL(&env->m_free_animations, animation, m_next_for_env);
        return NULL;
    }
    TAILQ_INSERT_TAIL(&env->m_init_animations, animation, m_next_for_env);
    TAILQ_INSERT_TAIL(&meta->m_animations, animation, m_next_for_meta);

    env->m_max_animation_id++;
    return animation;
}

plugin_ui_animation_t
plugin_ui_animation_create_by_type_name(plugin_ui_env_t env, const char * type_name) {
    plugin_ui_animation_meta_t meta;

    meta = plugin_ui_animation_meta_find(env->m_module, type_name);
    if (meta == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_animation: create: meta %s not exist!", type_name);
        return NULL;
    }

    return plugin_ui_animation_create(env, meta);
}

void plugin_ui_animation_free(plugin_ui_animation_t animation) {
    plugin_ui_env_t env = animation->m_env;

    assert(!animation->m_is_processing);
    
    if (animation->m_state == plugin_ui_animation_state_working) {
        plugin_ui_animation_stop(animation);
    }

    plugin_ui_animation_check_notify_complete(animation);
    if (animation->m_meta->m_fini_fun) {
        animation->m_meta->m_fini_fun(animation, animation->m_meta->m_ctx);
    }
    
    while(!TAILQ_EMPTY(&animation->m_controls)) {
        plugin_ui_animation_control_free(TAILQ_FIRST(&animation->m_controls));
    }

    if (animation->m_aspect) {
        plugin_ui_aspect_free(animation->m_aspect);
        animation->m_aspect = NULL;
    }
    
    while(!TAILQ_EMPTY(&animation->m_aspects)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&animation->m_aspects);
        plugin_ui_aspect_ref_free(ref, &ref->m_aspect->m_animations, &animation->m_aspects);
    }

    if (animation->m_name) {
        mem_free(animation->m_env->m_module->m_alloc, animation->m_name);
        animation->m_name = NULL;
    }

    switch(animation->m_state) {
    case plugin_ui_animation_state_init:
        TAILQ_REMOVE(&env->m_init_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_waiting:
        TAILQ_REMOVE(&env->m_wait_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_working:
        TAILQ_REMOVE(&env->m_working_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_done:
        TAILQ_REMOVE(&env->m_done_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_keep:
        TAILQ_REMOVE(&env->m_keep_animations, animation, m_next_for_env);
        break;
    default:
        break;
    }        

    TAILQ_REMOVE(&animation->m_meta->m_animations, animation, m_next_for_meta);
    cpe_hash_table_remove_by_ins(&env->m_animations, animation);

    TAILQ_INSERT_TAIL(&env->m_free_animations, animation, m_next_for_env);
}
    
void plugin_ui_animation_real_free(plugin_ui_animation_t animation) {
    TAILQ_REMOVE(&animation->m_env->m_free_animations, animation, m_next_for_env);
    mem_free(animation->m_env->m_module->m_alloc, animation);
}

void plugin_ui_animation_free_all(const plugin_ui_env_t env) {
    struct cpe_hash_it animation_it;
    plugin_ui_animation_t animation;

    cpe_hash_it_init(&animation_it, &env->m_animations);

    animation = cpe_hash_it_next(&animation_it);
    while (animation) {
        plugin_ui_animation_t next = cpe_hash_it_next(&animation_it);
        plugin_ui_animation_free(animation);
        animation = next;
    }

    assert(TAILQ_EMPTY(&env->m_init_animations));
    assert(TAILQ_EMPTY(&env->m_wait_animations));
    assert(TAILQ_EMPTY(&env->m_working_animations));
    assert(TAILQ_EMPTY(&env->m_done_animations));
}

uint8_t plugin_ui_animation_auto_free(plugin_ui_animation_t animation) {
    return animation->m_auto_free;
}

void plugin_ui_animation_set_auto_free(plugin_ui_animation_t animation, uint8_t auto_free) {
    animation->m_auto_free = auto_free;
}

float plugin_ui_animation_delay(plugin_ui_animation_t animation) {
    return animation->m_delay;
}

int plugin_ui_animation_set_delay(plugin_ui_animation_t animation, float delay) {
    if (animation->m_state != plugin_ui_animation_state_init) {
        CPE_ERROR(animation->m_env->m_module->m_em, "plugin_ui_animation_set_delay: animation is not init!");
        return -1;
    }

    animation->m_delay = delay;
    
    return 0;
}

int plugin_ui_animation_set_delay_frame(plugin_ui_animation_t animation, uint32_t frame) {
    return plugin_ui_animation_set_delay(animation, frame / animation->m_env->m_module->m_cfg_fps);
}

uint32_t plugin_ui_animation_loop_count(plugin_ui_animation_t animation) {
    return animation->m_loop_count;
}

float plugin_ui_animation_loop_delay(plugin_ui_animation_t animation) {
    return animation->m_loop_delay;
}

int plugin_ui_animation_set_loop(plugin_ui_animation_t animation, uint32_t loop_count, float loop_delay) {
    if (animation->m_state != plugin_ui_animation_state_init) {
        CPE_ERROR(animation->m_env->m_module->m_em, "plugin_ui_animation_set_loop: animation is not init!");
        return -1;
    }

    animation->m_loop_count = loop_count;
    animation->m_loop_delay = loop_delay;
    
    return 0;
}

plugin_ui_env_t plugin_ui_animation_env(plugin_ui_animation_t animation) {
    return animation->m_env;
}

const char * plugin_ui_animation_type_name(plugin_ui_animation_t animation) {
    return animation->m_meta->m_name;
}

uint32_t plugin_ui_animation_id(plugin_ui_animation_t animation) {
    return animation->m_id;
}

void * plugin_ui_animation_data(plugin_ui_animation_t animation) {
    return animation + 1;
}

plugin_ui_animation_t plugin_ui_animation_from_data(void * data) {
    return ((plugin_ui_animation_t)data) - 1;
}

const char * plugin_ui_animation_name(plugin_ui_animation_t animation) {
    return animation->m_name ? animation->m_name : "";
}

int plugin_ui_animation_set_name(plugin_ui_animation_t animation, const char * name) {
    plugin_ui_module_t module =  animation->m_env->m_module;
    char * new_name = NULL;

    if (name) {
        new_name = cpe_str_mem_dup(module->m_alloc, name);
        if (new_name == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_ui_animation_set_name: animation %d(%s) set name dup fail!",
                animation->m_id, animation->m_meta->m_name);
            return -1;
        }
    }

    if (animation->m_name) {
        mem_free(module->m_alloc, animation->m_name);
    }

    animation->m_name = new_name;
    return 0;
}

plugin_ui_animation_t
plugin_ui_animation_find(plugin_ui_env_t env, uint32_t animation_id) {
    struct plugin_ui_animation key;
    key.m_id = animation_id;
    return cpe_hash_table_find(&env->m_animations, &key);
}

uint8_t plugin_ui_animation_have_visiable_control(plugin_ui_animation_t animation) {
    plugin_ui_animation_control_t anim_control;

    TAILQ_FOREACH(anim_control, &animation->m_controls, m_next_for_animation) {
        plugin_ui_page_t page = anim_control->m_control->m_page;

        if ((page == page->m_env->m_template_page
             || plugin_ui_page_visible(page))
            && plugin_ui_control_visible(anim_control->m_control))
        {
            return 1;
        }
    }

    return 0;
}

plugin_ui_animation_state_t plugin_ui_animation_state(plugin_ui_animation_t animation) {
    return animation->m_state;
}

const char * plugin_ui_animation_state_str(plugin_ui_animation_t animation) {
    return plugin_ui_animation_state_to_str(animation->m_state);
}

void plugin_ui_animation_set_state(plugin_ui_animation_t animation, plugin_ui_animation_state_t state) {
    plugin_ui_env_t env = animation->m_env;

    if (animation->m_state == state) return;

    switch(animation->m_state) {
    case plugin_ui_animation_state_init:
        TAILQ_REMOVE(&env->m_init_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_waiting:
        TAILQ_REMOVE(&env->m_wait_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_working:
        TAILQ_REMOVE(&env->m_working_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_done:
        TAILQ_REMOVE(&env->m_done_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_keep:
        TAILQ_REMOVE(&env->m_keep_animations, animation, m_next_for_env);
        break;
    default:
        break;
    }        

    animation->m_state = state;
    
    switch(animation->m_state) {
    case plugin_ui_animation_state_init:
        TAILQ_INSERT_TAIL(&env->m_init_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_waiting: {
        plugin_ui_animation_t check_animation;
        TAILQ_FOREACH(check_animation, &env->m_wait_animations, m_next_for_env) {
            if (animation->m_delay < check_animation->m_delay) break;
        }

        if (check_animation) {
            TAILQ_INSERT_BEFORE(check_animation, animation, m_next_for_env);
        }
        else {
            TAILQ_INSERT_TAIL(&env->m_wait_animations, animation, m_next_for_env);
        }
        break;
    }
    case plugin_ui_animation_state_working:
        TAILQ_INSERT_TAIL(&env->m_working_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_done:
        TAILQ_INSERT_TAIL(&env->m_done_animations, animation, m_next_for_env);
        break;
    case plugin_ui_animation_state_keep:
        TAILQ_INSERT_TAIL(&env->m_keep_animations, animation, m_next_for_env);
        break;
    default:
        break;
    }
}

plugin_ui_control_t plugin_ui_animation_find_first_tie_control(plugin_ui_animation_t animation) {
    plugin_ui_animation_control_t animation_control;

    TAILQ_FOREACH(animation_control, &animation->m_controls, m_next_for_animation) {
        if (animation_control->m_is_tie) return animation_control->m_control;
    }

    return NULL;
}

static plugin_ui_control_t plugin_ui_animation_control_next(struct plugin_ui_control_it * it) {
    plugin_ui_animation_control_t * data = (plugin_ui_animation_control_t *)(it->m_data);
    plugin_ui_animation_control_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_animation);

    return r->m_control;
}

void plugin_ui_animation_controls(plugin_ui_animation_t animation, plugin_ui_control_it_t control_it) {
    *(plugin_ui_animation_control_t *)(control_it->m_data) = TAILQ_FIRST(&animation->m_controls);
    control_it->next = plugin_ui_animation_control_next;
}

int plugin_ui_animation_start(plugin_ui_animation_t animation) {
    plugin_ui_module_t module = animation->m_env->m_module;

    if (animation->m_state == plugin_ui_animation_state_working) {
        CPE_ERROR(
            module->m_em, "plugin_ui_animation_state: animation %d(%s) is already working %s",
            animation->m_id, animation->m_meta->m_name, plugin_ui_animation_state_to_str(animation->m_state));
        return -1;
    }

    if (animation->m_meta->m_enter_fun) {
        if (animation->m_meta->m_enter_fun(animation, animation->m_meta->m_ctx) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_ui_animation_state: animation %d(%s) enter fail!",
                animation->m_id, animation->m_meta->m_name);
            plugin_ui_animation_set_state(animation, plugin_ui_animation_state_done);
            return -1;
        }
    }

    if (animation->m_meta->m_update_fun) {
        plugin_ui_animation_set_state(animation, plugin_ui_animation_state_working);
    }
    else {
        plugin_ui_animation_set_state(animation, plugin_ui_animation_state_done);
    }

    return 0;
}

int plugin_ui_animation_stop(plugin_ui_animation_t animation) {
    plugin_ui_module_t module = animation->m_env->m_module;

    if (animation->m_state != plugin_ui_animation_state_working) {
        CPE_ERROR(
            module->m_em, "plugin_ui_animation_state: animation %d(%s) is in state %s",
            animation->m_id, animation->m_meta->m_name, plugin_ui_animation_state_to_str(animation->m_state));
        return -1;
    }

    if (animation->m_meta->m_exit_fun) {
        animation->m_meta->m_exit_fun(animation, animation->m_meta->m_ctx);
    }

    plugin_ui_animation_set_state(animation, plugin_ui_animation_state_done);
    
    return 0;
}

void plugin_ui_env_update_animations(plugin_ui_env_t env, float delta) {
    plugin_ui_animation_t animation, animation_next;

    /*处理需要更新的控件 */
    for(animation = TAILQ_FIRST(&env->m_working_animations); animation; animation = animation_next) {
        animation_next = TAILQ_NEXT(animation, m_next_for_env);

        assert(!animation->m_is_processing);
        
        /*没有可见的控件，则跳过更新 */
        if (!plugin_ui_animation_have_visiable_control(animation)) continue;

        /*更新动画 */
        if (animation->m_meta->m_update_fun) {
            animation->m_is_processing = 1;
            if (!animation->m_meta->m_update_fun(animation, animation->m_meta->m_ctx, delta)) {
                if (animation->m_meta->m_exit_fun) {
                    animation->m_meta->m_exit_fun(animation, animation->m_meta->m_ctx);
                }
                plugin_ui_animation_set_state(animation, plugin_ui_animation_state_done);
            }
            animation->m_is_processing = 0;
        }
        else {
            plugin_ui_animation_set_state(animation, plugin_ui_animation_state_done);
        }
    }

    /*处理等待控件 */
    if (!TAILQ_EMPTY(&env->m_wait_animations)) {
        env->m_animation_wait_runging_time += delta;

        for(animation = TAILQ_FIRST(&env->m_wait_animations); animation; animation = animation_next) {
            animation_next = TAILQ_NEXT(animation, m_next_for_env);

            if (env->m_animation_wait_runging_time < animation->m_delay) break;
            
            animation->m_delay = 0.0f;
            plugin_ui_animation_set_state(animation, plugin_ui_animation_state_init);
        }

        if (TAILQ_EMPTY(&env->m_wait_animations)) {
            env->m_animation_wait_runging_time = 0.0f;
        }
    }
    
    /*处理刚刚启动的控件 */
    for(animation = TAILQ_FIRST(&env->m_init_animations); animation; animation = animation_next) {
        animation_next = TAILQ_NEXT(animation, m_next_for_env);

        if (animation->m_delay > 0.0f) {
            animation->m_delay += env->m_animation_wait_runging_time;
            plugin_ui_animation_set_state(animation, plugin_ui_animation_state_waiting);
        }
        else { 
            assert(!animation->m_is_processing);

            /*没有控件，自动清理 */
            if (TAILQ_EMPTY(&animation->m_controls)) {
                plugin_ui_animation_set_state(animation, plugin_ui_animation_state_done);
                continue;
            }

            plugin_ui_animation_start(animation);
        }
    }

    /*处理所有done的动画，回调完成接口，启动后续动画 通知动画完成 */
    for(animation = TAILQ_FIRST(&env->m_done_animations); animation; animation = animation_next) {
        animation_next = TAILQ_NEXT(animation, m_next_for_env);

        if (animation->m_loop_count > 0) {
            animation->m_loop_count--;
            if (animation->m_loop_count == 0) {
                /*通知动画完成 */
                animation->m_is_processing = 1;
                plugin_ui_animation_check_notify_complete(animation);
                animation->m_is_processing = 0;

                if (animation->m_auto_free) {
                    plugin_ui_animation_free(animation);
                }
                else {
                    plugin_ui_animation_set_state(animation, plugin_ui_animation_state_keep);
                }
                continue;
            }
        }

        /*处理动画循环 */
        if (animation->m_loop_delay > 0.0f) {
            animation->m_delay = animation->m_loop_delay + env->m_animation_wait_runging_time;
            plugin_ui_animation_set_state(animation, plugin_ui_animation_state_waiting);
        }
        else { 
            assert(!animation->m_is_processing);
            
            animation->m_delay = animation->m_loop_delay;
            plugin_ui_animation_set_state(animation, plugin_ui_animation_state_init);
            plugin_ui_animation_start(animation);
        }
    }
}

const char * plugin_ui_animation_state_to_str(plugin_ui_animation_state_t state) {
    switch(state) {
    case plugin_ui_animation_state_init:
        return "init";
    case plugin_ui_animation_state_waiting:
        return "wait";
    case plugin_ui_animation_state_working:
        return "working";
    case plugin_ui_animation_state_done:
        return "done";
    case plugin_ui_animation_state_keep:
        return "keep";
    default:
        return "unknown";
    }
}

int plugin_ui_animation_setup(
    plugin_ui_animation_t animation, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame)
{
    const char * str_value;

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "delay-frame", ',', '='))) {
        animation->m_delay = (float)atoi(str_value) / animation->m_env->m_module->m_cfg_fps;
    }
    else if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "delay", ',', '='))) {
        animation->m_delay = atof(str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "loop-delay-frame", ',', '='))) {
        animation->m_loop_delay = (float)atoi(str_value) / animation->m_env->m_module->m_cfg_fps;
    }
    else if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "loop-delay", ',', '='))) {
        animation->m_loop_delay = atof(str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "loop-count", ',', '='))) {
        animation->m_loop_count = atoi(str_value);
    }
    
    if (animation->m_meta->m_setup_fun == NULL) {
        CPE_ERROR(
            animation->m_env->m_module->m_em, "animation %d(%s): not support setup",
            animation->m_id, animation->m_meta->m_name);
        return -1;
    }

    if (animation->m_meta->m_setup_fun(animation, animation->m_meta->m_ctx, arg_buf_will_change, control, frame) != 0) {
        CPE_ERROR(
            animation->m_env->m_module->m_em, "animation %d(%s): setup from %s fail",
            animation->m_id, animation->m_meta->m_name, arg_buf_will_change);
        return -1;
    }

    return 0;
}

plugin_ui_aspect_t plugin_ui_animation_aspect(plugin_ui_animation_t animation) {
    return animation->m_aspect;
}

plugin_ui_aspect_t plugin_ui_animation_aspect_check_create(plugin_ui_animation_t animation) {
    if (animation->m_aspect == NULL) {
        animation->m_aspect = plugin_ui_aspect_create(animation->m_env, NULL);
    }

    return animation->m_aspect;
}

void plugin_ui_animation_check_notify_complete(plugin_ui_animation_t animation) {
    if (animation->m_on_complete.m_fun) {
        animation->m_on_complete.m_fun(animation->m_on_complete.m_ctx, animation, animation->m_on_complete.m_arg);
        if (animation->m_on_complete.m_arg_free) {
            animation->m_on_complete.m_arg_free(animation->m_on_complete.m_arg);
        }

        animation->m_on_complete.m_ctx = NULL;
        animation->m_on_complete.m_fun = NULL;
        animation->m_on_complete.m_arg = NULL;
        animation->m_on_complete.m_arg_free = NULL;
    }
}

int plugin_ui_animation_set_on_complete(
    plugin_ui_animation_t animation, void * ctx, plugin_ui_animation_fun_t fun, void * arg, void (*arg_free)(void *))
{
    assert(animation->m_on_complete.m_fun == NULL);

    animation->m_on_complete.m_ctx = ctx;
    animation->m_on_complete.m_fun = fun;
    animation->m_on_complete.m_arg = arg;
    animation->m_on_complete.m_arg_free = arg_free;
    
    return 0;
}

uint32_t plugin_ui_animation_hash(const plugin_ui_animation_t meta) {
    return meta->m_id;
}

int plugin_ui_animation_eq(const plugin_ui_animation_t l, const plugin_ui_animation_t r) {
    return l->m_id == r->m_id ? 1 : 0;
}
