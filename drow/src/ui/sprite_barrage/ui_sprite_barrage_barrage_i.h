#ifndef UI_SPRITE_BARRAGE_EMITTER_I_H
#define UI_SPRITE_BARRAGE_EMITTER_I_H
#include "render/utils/ui_transform.h"
#include "plugin/barrage/plugin_barrage_barrage.h"
#include "plugin/barrage/plugin_barrage_emitter.h"
#include "ui/sprite_barrage/ui_sprite_barrage_barrage.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_barrage_barrage {
    ui_sprite_barrage_obj_t m_obj;
    TAILQ_ENTRY(ui_sprite_barrage_barrage) m_next_for_obj;
    const char * m_part_name;
    const char * m_group;
    uint8_t m_is_enable;
    uint8_t m_is_pause;
    uint32_t m_loop_count;
    char m_res[64];
    plugin_barrage_barrage_t m_barrage;
    plugin_barrage_data_emitter_flip_type_t m_flip_type;
    ui_transform m_transform;
    uint8_t m_accept_flip;
    uint8_t m_accept_scale;
    uint8_t m_accept_angle;
    float m_angle_adj_rad;
    ui_sprite_2d_part_t m_part;
};

int ui_sprite_barrage_barrage_load(ui_sprite_barrage_barrage_t emitter);
void ui_sprite_barrage_barrage_unload(ui_sprite_barrage_barrage_t emitter);

void ui_sprite_barrage_barrage_update_pos(ui_sprite_barrage_barrage_t emitter);
    
void ui_sprite_barrage_barrage_sync_state(ui_sprite_barrage_barrage_t emitter);
    
#ifdef __cplusplus
}
#endif

#endif
