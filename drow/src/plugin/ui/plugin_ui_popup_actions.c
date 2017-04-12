#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin_ui_popup_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_control_action_i.h"

static void plugin_ui_popup_action_close(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    plugin_ui_popup_t popup = ctx;
    plugin_ui_popup_set_visible(popup, 0);
}

int plugin_ui_popup_set_close_on_click(plugin_ui_popup_t popup, plugin_ui_control_t control) {
    plugin_ui_env_t env = popup->m_env;
    plugin_ui_control_action_t action;

    action = plugin_ui_control_action_create(
        control, plugin_ui_event_mouse_click, plugin_ui_event_scope_self, plugin_ui_popup_action_close, popup);
    if (action == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_popup_create_action: control %s create action fail!",
            plugin_ui_control_path_dump(&popup->m_env->m_module->m_dump_buffer, control));
        return -1;
    }

    return 0;
}

struct plugin_ui_popup_action_do_action_ctx {
    plugin_ui_popup_t m_popup;
    char m_action_name[32];
};

static void plugin_ui_popup_action_action(void * i_ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    struct plugin_ui_popup_action_do_action_ctx * ctx = i_ctx;

    if (plugin_ui_popup_trigger_action(ctx->m_popup, ctx->m_action_name) == 0) {
        CPE_ERROR(
            ctx->m_popup->m_env->m_module->m_em,
            "plugin_ui_popup_create_action: no action process %s", ctx->m_action_name);
    }
}

int plugin_ui_popup_set_action_on_click(plugin_ui_popup_t popup, plugin_ui_control_t control, const char * action_name) {
    plugin_ui_env_t env = popup->m_env;
    plugin_ui_control_action_t action;
    struct plugin_ui_popup_action_do_action_ctx * ctx;

    action = plugin_ui_control_action_create(
        control, plugin_ui_event_mouse_click, plugin_ui_event_scope_self, plugin_ui_popup_action_action, NULL);
    if (action == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_popup_create_action: control %s create action fail!",
            plugin_ui_control_path_dump(&popup->m_env->m_module->m_dump_buffer, control));
        return -1;
    }

    assert(sizeof(struct plugin_ui_popup_action_do_action_ctx) <= PLUGIN_UI_CONTROL_ACTION_DATA_CAPACITY);
    
    ctx = plugin_ui_control_action_data(action);
    ctx->m_popup = popup;
    cpe_str_dup(ctx->m_action_name, sizeof(ctx->m_action_name), action_name);

    return 0;
}

plugin_ui_control_t plugin_ui_popup_find_action_control(plugin_ui_popup_t popup, const char * action_name) {
    plugin_ui_control_action_t action;

    TAILQ_FOREACH(action, &popup->m_page->m_control_actions, m_next_for_page) {
        if (action->m_fun == plugin_ui_popup_action_action) {
            struct plugin_ui_popup_action_do_action_ctx * ctx = plugin_ui_control_action_data(action);
            if (strcmp(ctx->m_action_name, action_name) == 0) return action->m_control;
        }
    }

    return NULL;
}
