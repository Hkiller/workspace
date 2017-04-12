#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_tri/ui_sprite_tri_action.h"
#include "ui/sprite_tri/ui_sprite_tri_action_meta.h"
#include "ui_sprite_tri_action_send_event_i.h"

ui_sprite_tri_action_send_event_t
ui_sprite_tri_action_send_event_create(ui_sprite_tri_rule_t rule) {
    ui_sprite_tri_action_t action;
    
    action = ui_sprite_tri_action_create(rule, UI_SPRITE_TRI_ACTION_SEND_EVENT);
    if (action == NULL) return NULL;

    return (ui_sprite_tri_action_send_event_t)ui_sprite_tri_action_data(action);
}
    
void ui_sprite_tri_action_send_event_free(ui_sprite_tri_action_send_event_t action_send_event) {
    ui_sprite_tri_action_t action;

    action = ui_sprite_tri_action_from_data(action_send_event);

    ui_sprite_tri_action_free(action);
}

static int ui_sprite_tri_action_send_event_init(void * ctx, ui_sprite_tri_action_t action) {
    ui_sprite_tri_action_send_event_t data = (ui_sprite_tri_action_send_event_t)ui_sprite_tri_action_data(action);
    ui_sprite_tri_module_t module = (ui_sprite_tri_module_t)ctx;
    
    data->m_module = module;
    
    return 0;
}

static void ui_sprite_tri_action_send_event_fini(void * ctx, ui_sprite_tri_action_t action) {
}

static int ui_sprite_tri_action_send_event_copy(void * ctx, ui_sprite_tri_action_t action, ui_sprite_tri_action_t source) {
    if (ui_sprite_tri_action_send_event_init(ctx, action) != 0) return -1;
    return 0;
}

static void ui_sprite_tri_action_send_event_exec(void * ctx, ui_sprite_tri_action_t action) {
    /* ui_sprite_tri_module_t module = (ui_sprite_tri_module_t)ctx; */
    /* ui_sprite_entity_t entity = ui_sprite_tri_action_entity(action); */

    /* if (ui_sprite_entity_debug(entity)) { */
    /*     CPE_INFO(module->m_em, "entity %d(%s): remove self", ui_sprite_entity_id(entity), ui_sprite_entity_name(entity)); */
    /* } */
    
    /* ui_sprite_entity_set_destory(entity); */
}

int ui_sprite_tri_action_send_event_regist(ui_sprite_tri_module_t module) {
    ui_sprite_tri_action_meta_t meta;

    meta = ui_sprite_tri_action_meta_create(
        module, UI_SPRITE_TRI_ACTION_SEND_EVENT, sizeof(struct ui_sprite_tri_action_send_event),
        module,
        ui_sprite_tri_action_send_event_init,
        ui_sprite_tri_action_send_event_fini,
        ui_sprite_tri_action_send_event_copy,
        ui_sprite_tri_action_send_event_exec);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: remove-self register: meta create fail",
            ui_sprite_tri_module_name(module));
        return -1;
    }
    
    return 0;
}

void ui_sprite_tri_action_send_event_unregist(ui_sprite_tri_module_t module) {
    ui_sprite_tri_action_meta_t meta;

    meta = ui_sprite_tri_action_meta_find(module, UI_SPRITE_TRI_ACTION_SEND_EVENT);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: remove-self unregister: meta not exist",
            ui_sprite_tri_module_name(module));
    }
    else {
        ui_sprite_tri_action_meta_free(meta);
    }
}

const char * UI_SPRITE_TRI_ACTION_SEND_EVENT = "send-event";

