#include <assert.h>
#include "plugin_ui_mouse_i.h"
#include "plugin_ui_control_i.h"

static void plugin_ui_mouse_do_active(plugin_ui_env_t env, plugin_ui_mouse_t mouse);
static void plugin_ui_mouse_do_deactive(plugin_ui_env_t env, plugin_ui_mouse_t mouse);

plugin_ui_mouse_t plugin_ui_mouse_create(plugin_ui_env_t env) {
    plugin_ui_mouse_t mouse;
    
    assert(env->m_mouse == NULL);

    mouse = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_mouse));
    if (mouse == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_mouse_create: alloc fail!");
        return NULL;
    }

    mouse->m_env = env;
    mouse->m_cur_pt = UI_VECTOR_2_ZERO;
    mouse->m_is_active = 0;
    mouse->m_l_down = 0;
    mouse->m_r_down = 0;
    env->m_mouse = mouse;
    return mouse;
}

void plugin_ui_mouse_free(plugin_ui_mouse_t mouse) {
    plugin_ui_env_t env = mouse->m_env;

    assert(env->m_mouse == mouse);

    if (mouse->m_is_active) {
        plugin_ui_mouse_do_deactive(env, mouse);
    }

    mem_free(env->m_module->m_alloc, mouse);
    env->m_mouse = NULL;
}

void plugin_ui_env_mouse_active(plugin_ui_env_t env) {
    if (env->m_mouse == NULL) {
        env->m_mouse = plugin_ui_mouse_create(env);
    }

    if (!env->m_mouse->m_is_active) {
        env->m_mouse->m_is_active = 1;
        plugin_ui_mouse_do_active(env, env->m_mouse);
    }    
}

void plugin_ui_env_mouse_deactive(plugin_ui_env_t env) {
    if (env->m_mouse) {
        env->m_mouse->m_is_active = 0;
        plugin_ui_mouse_do_deactive(env, env->m_mouse);
    }
}

uint8_t plugin_ui_env_mouse_is_active(plugin_ui_env_t env) {
    return env->m_mouse ? env->m_mouse->m_is_active : 0;
}

plugin_ui_mouse_t plugin_ui_env_mouse(plugin_ui_env_t env) {
    return env->m_mouse && env->m_mouse->m_is_active ? env->m_mouse : NULL;
}

plugin_ui_mouse_t plugin_ui_env_mouse_check_create(plugin_ui_env_t env) {
    plugin_ui_env_mouse_active(env);
    assert(env->m_mouse);
    return env->m_mouse;
}

ui_vector_2_t plugin_ui_env_mouse_pos(plugin_ui_mouse_t mouse) {
    return mouse->m_is_active ? &mouse->m_cur_pt : NULL;
}

void plugin_ui_env_mouse_set_pos(plugin_ui_mouse_t mouse, ui_vector_2_t pos) {
    plugin_ui_env_t env = mouse->m_env;
    plugin_ui_control_t float_control = NULL;

    mouse->m_cur_pt = *pos;

    float_control = plugin_ui_env_find_float_control(env, &env->m_mouse->m_cur_pt);

    if (float_control != env->m_float_control) {
        plugin_ui_env_set_float_control(env, float_control);
    }
    else {
        plugin_ui_env_dispatch_event(env, float_control, plugin_ui_event_float_move);
    }
}

uint8_t plugin_ui_mouse_l_down(plugin_ui_mouse_t mouse) {
    return mouse->m_l_down;
}

void plugin_ui_mouse_set_l_down(plugin_ui_mouse_t mouse, uint8_t is_down) {
    mouse->m_l_down = is_down ? 1 : 0;
}

uint8_t plugin_ui_mouse_r_down(plugin_ui_mouse_t mouse) {
    return mouse->m_r_down;
}

void plugin_ui_mouse_set_r_down(plugin_ui_mouse_t mouse, uint8_t is_down) {
    mouse->m_r_down = is_down ? 1 : 0;
}

static void plugin_ui_mouse_do_active(plugin_ui_env_t env, plugin_ui_mouse_t mouse) {
}

static void plugin_ui_mouse_do_deactive(plugin_ui_env_t env, plugin_ui_mouse_t mouse) {
    if (env->m_float_control) {
        plugin_ui_env_set_float_control(env, NULL);
        assert(env->m_float_control == NULL);
    }
}
