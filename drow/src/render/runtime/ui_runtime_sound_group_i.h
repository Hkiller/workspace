#ifndef UI_RUNTIME_SOUND_GROUP_I_H
#define UI_RUNTIME_SOUND_GROUP_I_H
#include "render/runtime/ui_runtime_sound_group.h"
#include "ui_runtime_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_sound_group {
    ui_runtime_module_t m_module;
    TAILQ_ENTRY(ui_runtime_sound_group) m_next;
    char m_name[32];
    ui_runtime_sound_type_t m_sound_type;
    ui_runtime_sound_group_schedule_type_t m_schedule_type;
    uint8_t m_enable;
    float m_volume;
    uint16_t m_chanel_max_id;
    uint16_t m_chanel_count;
    ui_runtime_sound_chanel_list_t m_chanels;
};

#ifdef __cplusplus
}
#endif

#endif 
