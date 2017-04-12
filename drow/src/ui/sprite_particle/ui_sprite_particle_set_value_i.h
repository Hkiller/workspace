#ifndef UI_SPRITE_PARTICLE_SET_VALUE_I_H
#define UI_SPRITE_PARTICLE_SET_VALUE_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/particle/plugin_particle_obj.h"
#include "ui/sprite_particle/ui_sprite_particle_set_value.h"
#include "ui_sprite_particle_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_particle_set_value {
    ui_sprite_particle_module_t m_module;
    char * m_particle;
    char * m_value;
};

int ui_sprite_particle_set_value_regist(ui_sprite_particle_module_t module);
void ui_sprite_particle_set_value_unregist(ui_sprite_particle_module_t module);

#ifdef __cplusplus
}
#endif

#endif
