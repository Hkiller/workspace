#ifndef UI_SPRITE_DATA_BUILD_I_H
#define UI_SPRITE_DATA_BUILD_I_H
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui_sprite_entity_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_data_find_ctx {
    ui_sprite_world_t m_world;
    ui_sprite_entity_t m_entity;
    dr_data_source_t m_data_source;
};
xtoken_t ui_sprite_data_find_value(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em);
    
int ui_sprite_data_build(
    dr_data_entry_t to, char * arg_value,
    ui_sprite_world_t world, ui_sprite_entity_t entity, dr_data_source_t data_source);

#ifdef __cplusplus
}
#endif

#endif

