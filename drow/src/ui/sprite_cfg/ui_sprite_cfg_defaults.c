#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui_sprite_cfg_loader_i.h"

#define UI_SPRITE_CFG_INIT_COMPONENT(__name, __fun)                     \
    do {                                                                \
        extern int ui_sprite_cfg_load_component_ ## __fun(void * ctx, ui_sprite_component_t component, cfg_t cfg); \
            if (ui_sprite_cfg_loader_add_comp_loader(loader, __name, ui_sprite_cfg_load_component_ ## __fun, loader) != 0) { \
                CPE_ERROR(loader->m_em, "%s: add default component loader %s fail!", ui_sprite_cfg_loader_name(loader), __name); \
                return -1;                                              \
            }                                                           \
    } while(0)

#define UI_SPRITE_CFG_INIT_ACTION(__name, __fun)                        \
    do {                                                                \
        extern ui_sprite_fsm_action_t ui_sprite_cfg_load_action_ ## __fun(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg); \
            if (ui_sprite_cfg_loader_add_action_loader(loader, __name, ui_sprite_cfg_load_action_ ## __fun, loader) != 0) { \
                CPE_ERROR(loader->m_em, "%s: add default action loader %s fail!", ui_sprite_cfg_loader_name(loader), __name); \
                return -1;                                              \
            }                                                           \
    } while(0)

int ui_sprite_cfg_loader_init_default_loaders(ui_sprite_cfg_loader_t loader) {
    UI_SPRITE_CFG_INIT_COMPONENT(UI_SPRITE_FSM_COMPONENT_FSM_NAME, fsm);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_FSM_ACTION_FSM_NAME, fsm);

    return 0;
}

